
#include "system.h"

TRACE_TAG(hal_uart);
#if !ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif

#ifndef CFG_HAL_UART_RX_BUFSIZE
#define CFG_HAL_UART_RX_BUFSIZE        1024
#endif

#ifndef CFG_HAL_UART_TX_BUFSIZE
#define CFG_HAL_UART_TX_BUFSIZE        1024
#endif

#ifndef CFG_HAL_UART_RXCHAR_TIMEOUT
#define CFG_HAL_UART_RXCHAR_TIMEOUT    250
#endif

#ifndef CFG_HAL_UART_TXCHAR_TIMEOUT
#define CFG_HAL_UART_TXCHAR_TIMEOUT    250
#endif

#if defined(CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API == 1)


/** UART device */
typedef struct
{
   const hal_uart_def_t *def;

   hal_uart_t uart;
   hal_uart_recv_cb_t recv_cb;

   osMessageQId tx_queue_id;
   osMessageQId rx_queue_id;

} hal_uart_device_t;


// Prototypes:
static hal_uart_device_t *hal_uart_get_device(USART_TypeDef *usart);

// Locals:
static const hal_uart_def_t hal_uart_def[] = CFG_HAL_UART_DEF;
#define NUM_UARTS   (sizeof(hal_uart_def) / sizeof(hal_uart_def_t))

static hal_uart_device_t hal_uart_dev[NUM_UARTS];

static const osMessageQDef(txQueue, CFG_HAL_UART_TX_BUFSIZE, uint8_t);
static const osMessageQDef(rxQueue, CFG_HAL_UART_RX_BUFSIZE, uint8_t);



/** Initialise ZW uart */
int hal_uart_init(hal_uart_t uart)
{
   int ix;
   const hal_uart_def_t *def;
   hal_uart_device_t *dev;
   USART_InitTypeDef USART_InitStructure;
   GPIO_InitTypeDef GPIO_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   ASSERT(uart < NUM_UARTS);
   def = &hal_uart_def[uart];
   dev = &hal_uart_dev[uart];
   dev->def = def;

   dev->uart = uart;
   dev->recv_cb = NULL;

  // Create TX queue
   dev->tx_queue_id = osMessageCreate(osMessageQ(txQueue), 0);
   if (dev->tx_queue_id == NULL)
   {
      TRACE_ERROR("Create TX queue failed");
      return -1;
   }

   // Create RX queue
   dev->rx_queue_id = osMessageCreate(osMessageQ(rxQueue), 0);
   if (dev->rx_queue_id == NULL)
   {
      TRACE_ERROR("Create RX queue failed");
      return -1;
   }

   // Enable the clock
   switch(def->bus_periph)
   {
      case RCC_PERIPH_APB1:
         RCC_APB1PeriphClockCmd(def->clk_periph, ENABLE);
         break;

      case RCC_PERIPH_APB2:
         RCC_APB2PeriphClockCmd(def->clk_periph, ENABLE);
         break;

      default:
         TRACE_ERROR("Not supported periph clk: %d", def->clk_periph);
         return -1;
   }

   // GPIO Configuration
   GPIO_StructInit(&GPIO_InitStructure);
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

   for (ix = 0; ix < NUM_UART_PINS; ix++)
   {
      GPIO_PinAFConfig(def->pins[ix].port, hal_gpio_pin_source(def->pins[ix].pin), def->pins[ix].alternate_function);
      GPIO_InitStructure.GPIO_Pin = def->pins[ix].pin;
      GPIO_Init(def->pins[ix].port, &GPIO_InitStructure);
   }

   // USART configuration
   USART_StructInit(&USART_InitStructure);
   USART_InitStructure.USART_BaudRate = def->baudrate;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   USART_Init(def->usart, &USART_InitStructure);

   // Disable TXE interrupt default state after reset
   USART_ClearITPendingBit(def->usart, USART_IT_TXE);

   // Enable USART receive interrupt
   USART_ITConfig(def->usart, USART_IT_RXNE, ENABLE);

   // Enable the USART Interrupt
   NVIC_InitStructure.NVIC_IRQChannel = def->irqn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CFG_HAL_UART_IRQ_PRIORITY;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   // Enable USART
   USART_Cmd(def->usart, ENABLE);

   return 0;
}

/** Close uart */
int hal_uart_deinit(hal_uart_t uart)
{
   hal_uart_device_t *dev;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

   DISABLE_INTERRUPTS();

   // Disable USART receive interrupt
   USART_ITConfig(dev->def->usart, USART_IT_RXNE, DISABLE);
   USART_ClearITPendingBit(dev->def->usart, USART_IT_RXNE);
   USART_Cmd(dev->def->usart, DISABLE);
   USART_DeInit(dev->def->usart);

   ENABLE_INTERRUPTS();

   return 0;
}


/** Register receive callback */
int hal_uart_recv(hal_uart_t uart, hal_uart_recv_cb_t recv_cb)
{
   hal_uart_device_t *dev;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

   DISABLE_INTERRUPTS();
   dev->recv_cb = recv_cb;
   ENABLE_INTERRUPTS();
}

/** Put char through uart */
void hal_uart_putchar(hal_uart_t uart, uint8_t c)
{
   hal_uart_device_t *dev;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

  // Add char to queue
   osMessagePut(dev->tx_queue_id, c, osWaitForever);

   // Enable TX buffer empty interrupt
   USART_ITConfig(dev->def->usart, USART_IT_TXE, ENABLE);
}

/** Get char from uart */
int hal_uart_getchar(hal_uart_t uart)
{
   hal_uart_device_t *dev;
   osEvent event;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

  event = osMessageGet(dev->rx_queue_id, CFG_HAL_UART_RXCHAR_TIMEOUT);
  return (event.status == osEventMessage) ? event.value.v : -1;
}

/** Write buffer */
int hal_uart_write(hal_uart_t uart, void *buf, uint16_t count)
{
   hal_uart_device_t *dev;
   int total = 0;
   uint8_t *pbuf = buf;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

   while(count > 0)
   {
      // Add char to queue
      if (osMessagePut(dev->tx_queue_id, *pbuf, CFG_HAL_UART_TXCHAR_TIMEOUT) != osOK)
      {
         // Enable TX buffer empty interrupt
         USART_ITConfig(dev->def->usart, USART_IT_TXE, ENABLE);
         continue;
      }

      pbuf++;
      total++;
      count--;
   }

   // Enable TX buffer empty interrupt
   USART_ITConfig(dev->def->usart, USART_IT_TXE, ENABLE);

   return total;
}

/** Read buffer */
int hal_uart_read(hal_uart_t uart, void *buf, uint16_t count, uint16_t timeout)
{
   hal_uart_device_t *dev;
   osEvent event;
   int total = 0;
   uint8_t *pbuf = buf;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

   while(count--)
   {
      event = osMessageGet(dev->rx_queue_id, timeout);
      if (event.status != osEventMessage)
         break;

      *pbuf++ = event.value.v;
      total++;
   }

   return total;
}

static hal_uart_device_t *hal_uart_get_device(USART_TypeDef *usart)
{
   int ix;

   for (ix = 0; ix < NUM_UARTS; ix++)
   {
      if (hal_uart_dev[ix].def->usart == usart)
         return &hal_uart_dev[ix];
   }

   return NULL;
}

/** UART device IRQ handler */
static inline void usart_irq_handler(hal_uart_device_t *dev)
{
   // Transmit data register empty
   if (USART_GetITStatus(dev->def->usart, USART_IT_TXE) != RESET)
   {
      osEvent event;

      event = osMessageGet(dev->tx_queue_id, CFG_HAL_UART_TXCHAR_TIMEOUT);
      if (event.status == osEventMessage)
      {
         // Send char
         USART_SendData(dev->def->usart, event.value.v);
      }
      else
      {
         // Disable transmit data register empty interrupt
         USART_ITConfig(dev->def->usart, USART_IT_TXE, DISABLE);
      }

      USART_ClearITPendingBit(dev->def->usart, USART_IT_TXE);
   }

   // Receive char
   if (USART_GetITStatus(dev->def->usart, USART_IT_RXNE) != RESET)
   {
      uint8_t c;

      c = USART_ReceiveData(dev->def->usart);

      if (dev->recv_cb != NULL)
      {
         dev->recv_cb(dev->uart, c);
      }
      else
      {
         // Add received char to queue
         osMessagePut(dev->rx_queue_id, c, 0);
      }

      USART_ClearITPendingBit(dev->def->usart, USART_IT_RXNE);
   }
}


//
// Native IRQ handlers
//

#define USART_IRQ_HANDLER(usart)          \
{                                         \
   static hal_uart_device_t *dev = NULL;  \
   if (dev == NULL)                       \
      dev = hal_uart_get_device(usart);   \
   if (dev != NULL)                       \
      usart_irq_handler(dev);             \
}

void USART1_IRQHandler(void)
{
   USART_IRQ_HANDLER(USART1);
}

void USART2_IRQHandler(void)
{
   USART_IRQ_HANDLER(USART2);
}

void USART3_IRQHandler(void)
{
   USART_IRQ_HANDLER(USART3);
}

void UART4_IRQHandler(void)
{
   USART_IRQ_HANDLER(UART4);
}

void UART5_IRQHandler(void)
{
   USART_IRQ_HANDLER(UART5);
}

void USART6_IRQHandler(void)
{
   USART_IRQ_HANDLER(USART6);
}

#else

/** UART device */
typedef struct
{
   const hal_uart_def_t *def;

   hal_uart_t uart;
   hal_uart_recv_cb_t recv_cb;

} hal_uart_device_t;


// Prototypes:
static hal_uart_device_t *hal_uart_get_device(USART_TypeDef *usart);

// Locals:
static const hal_uart_def_t hal_uart_def[] = CFG_HAL_UART_DEF;
#define NUM_UARTS   (sizeof(hal_uart_def) / sizeof(hal_uart_def_t))

static hal_uart_device_t hal_uart_dev[NUM_UARTS];


int hal_uart_init(hal_uart_t uart)
{
   int ix;
   const hal_uart_def_t *def;
   hal_uart_device_t *dev;
   USART_InitTypeDef USART_InitStructure;
   GPIO_InitTypeDef GPIO_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   ASSERT(uart < NUM_UARTS);
   def = &hal_uart_def[uart];
   dev = &hal_uart_dev[uart];
   dev->def = def;

   dev->uart = uart;
   dev->recv_cb = NULL;

   // Enable the clock
   switch(def->bus_periph)
   {
      case RCC_PERIPH_APB1:
         RCC_APB1PeriphClockCmd(def->clk_periph, ENABLE);
         break;

      case RCC_PERIPH_APB2:
         RCC_APB2PeriphClockCmd(def->clk_periph, ENABLE);
         break;

      default:
         TRACE_ERROR("Not supported periph clk: %d", def->clk_periph);
         return -1;
   }

   // GPIO Configuration
   GPIO_StructInit(&GPIO_InitStructure);
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

   for (ix = 0; ix < NUM_UART_PINS; ix++)
   {
      GPIO_PinAFConfig(def->pins[ix].port, hal_gpio_pin_source(def->pins[ix].pin), def->pins[ix].alternate_function);
      GPIO_InitStructure.GPIO_Pin = def->pins[ix].pin;
      GPIO_Init(def->pins[ix].port, &GPIO_InitStructure);
   }

   // USART configuration
   USART_StructInit(&USART_InitStructure);
   USART_InitStructure.USART_BaudRate = def->baudrate;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   USART_Init(def->usart, &USART_InitStructure);

   // Disable TXE interrupt default state after reset
   USART_ClearITPendingBit(def->usart, USART_IT_TXE);

   // Enable USART receive interrupt
   //USART_ITConfig(def->usart, USART_IT_RXNE, ENABLE);

   // Enable the USART Interrupt
   //NVIC_InitStructure.NVIC_IRQChannel = def->irqn;
   //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CFG_HAL_UART_IRQ_PRIORITY;
   //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   //NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   //NVIC_Init(&NVIC_InitStructure);

   // Enable USART
   USART_Cmd(def->usart, ENABLE);

   return 0;
}

/** Put char through uart */
void hal_uart_putchar(hal_uart_t uart, uint8_t c)
{
   hal_uart_device_t *dev;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

   while (USART_GetFlagStatus(dev->def->usart, USART_FLAG_TC) != SET);
   USART_SendData(dev->def->usart, c);
}



#endif  // CFG_CMSIS_OS_API