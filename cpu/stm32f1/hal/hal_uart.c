/**
 * \file hal_uart.c     \brief UART hal driver
 */

#include "system.h"

TRACE_TAG(hal_uart);
#if !ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

#ifndef CFG_HAL_UART_RX_BUFSIZE
#define CFG_HAL_UART_RX_BUFSIZE        1024
#endif 

#ifndef CFG_HAL_UART_TX_BUFSIZE
#define CFG_HAL_UART_TX_BUFSIZE        1024
#endif 

#ifndef CFG_HAL_UART_RXTXCHAR_TIMEOUT
#define CFG_HAL_UART_RXTXCHAR_TIMEOUT    250
#endif

/** UART device */
typedef struct
{
   const hal_uart_def_t *def;
   UART_HandleTypeDef huart;

   osMessageQId tx_queue_id;
   osMessageQId rx_queue_id;
   
   hal_uart_t uart;
   hal_uart_recv_cb_t recv_cb;
   
} hal_uart_device_t;


// Prototypes:
static hal_uart_device_t *hal_uart_get_device(USART_TypeDef *usart);
static int hal_uart_clock_enable(hal_uart_t uart, bool enable);

// Locals:
static const osMessageQDef(txQueue, CFG_HAL_UART_TX_BUFSIZE, uint8_t);
static const osMessageQDef(rxQueue, CFG_HAL_UART_RX_BUFSIZE, uint8_t);

static const hal_uart_def_t hal_uart_def[] = CFG_HAL_UART_DEF;
#define NUM_UARTS   (sizeof(hal_uart_def) / sizeof(hal_uart_def_t))

/** UART instances context */
static hal_uart_device_t hal_uart_dev[NUM_UARTS];


/** Initialise ZW uart */
int hal_uart_init(hal_uart_t uart)
{
   GPIO_InitTypeDef GPIO_InitStruct;
   hal_uart_device_t *dev;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];
   dev->def = &hal_uart_def[uart];
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

   // Enable clock
   if (hal_uart_clock_enable(uart, true) != 0)
      return -1;

   // GPIO Configuration
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

   // TX pin
   GPIO_InitStruct.Pin = dev->def->tx_pin.pin;
   HAL_GPIO_Init(dev->def->tx_pin.port, &GPIO_InitStruct);

   // RX pin
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pin = dev->def->rx_pin.pin;
   HAL_GPIO_Init(dev->def->rx_pin.port, &GPIO_InitStruct);

 __HAL_AFIO_REMAP_USART1_ENABLE();

   dev->huart.Instance = dev->def->usart;
   dev->huart.Init.BaudRate = dev->def->baudrate;
   dev->huart.Init.WordLength = UART_WORDLENGTH_8B;
   dev->huart.Init.StopBits = UART_STOPBITS_1;
   dev->huart.Init.Parity = UART_PARITY_NONE;
   dev->huart.Init.Mode = UART_MODE_TX_RX;
   dev->huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   dev->huart.Init.OverSampling = UART_OVERSAMPLING_16;

   if (HAL_UART_Init(&dev->huart) != HAL_OK)
   {
      TRACE_ERROR("Init uart failed");
      return -1;
   }

   // Disable TXE interrupt default state after reset
   __HAL_UART_CLEAR_FLAG(&dev->huart, UART_FLAG_TXE);

   // Enable USART receive interrupt
   __HAL_UART_ENABLE_IT(&dev->huart, UART_IT_RXNE);

   // Interrupt init
   HAL_NVIC_SetPriority(dev->def->irqn, CFG_HAL_UART_PRIORITY, 0);
   HAL_NVIC_EnableIRQ(dev->def->irqn);

   return 0;
}

/** Close uart */
int hal_uart_deinit(hal_uart_t uart)
{
   // TODO:
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

   return 0;
}

/** Put char through uart */
inline void hal_uart_putchar(hal_uart_t uart, uint8_t c)
{
   hal_uart_device_t *dev;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

   // Add char to queue
   osMessagePut(dev->tx_queue_id, c, osWaitForever);

   // Enable TX buffer empty interrupt
   __HAL_UART_ENABLE_IT(&dev->huart, UART_IT_TXE);
}

/** Get char from uart */
int hal_uart_getchar(hal_uart_t uart)
{
   hal_uart_device_t *dev;
   osEvent event;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

   event = osMessageGet(dev->rx_queue_id, CFG_HAL_UART_RXTXCHAR_TIMEOUT);
   return (event.status == osEventMessage) ? event.value.v : -1;
}

/** Write buffer */
int hal_uart_write(hal_uart_t uart, void *buf, uint16_t count)
{
   int total = 0;
   uint16_t nxthead;
   hal_uart_device_t *dev;
   uint8_t *pbuf = buf;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

   while(count > 0)
   {
      // Add char to queue
      if (osMessagePut(dev->tx_queue_id, *pbuf, CFG_HAL_UART_RXTXCHAR_TIMEOUT) != osOK)
      {
         // Enable TX buffer empty interrupt
         __HAL_UART_ENABLE_IT(&dev->huart, UART_IT_TXE);
         continue;
      }

      pbuf++;
      total++;
      count--;
   }

   // Enable TX buffer empty interrupt
   __HAL_UART_ENABLE_IT(&dev->huart, UART_IT_TXE);

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

/** Synchronize output/input buffers */
void hal_uart_sync(hal_uart_t uart)
{
   hal_uart_device_t *dev;
   osEvent event;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

   do
   {
      event = osMessageGet(dev->rx_queue_id, 10);
   } while (event.status == osEventMessage);
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

/** Enable/disable clock */
static int hal_uart_clock_enable(hal_uart_t uart, bool enable)
{
   hal_uart_device_t *dev;

   ASSERT(uart < NUM_UARTS);
   dev = &hal_uart_dev[uart];

   if (dev->def->usart == USART1)
   {
      // Peripheral clock enable
      __HAL_RCC_USART1_CLK_ENABLE();
   }
   else if (dev->def->usart == USART2)
   {
      // Peripheral clock enable
      __HAL_RCC_USART2_CLK_ENABLE();
   }
   else if (dev->def->usart == USART3)
   {
      // Peripheral clock enable
      __HAL_RCC_USART3_CLK_ENABLE();
   }
   else
   {
      TRACE_ERROR("Not supported UART[%d] device", uart);
      return -1;
   }

   return 0;
}

static inline void usart_irq_handler(hal_uart_device_t *dev)
{
   // Transmit data register empty
   if (__HAL_UART_GET_FLAG(&dev->huart, UART_FLAG_TXE) != RESET)
   {
      osEvent event;

      __HAL_UART_CLEAR_FLAG(&dev->huart, UART_FLAG_TXE);

      event = osMessageGet(dev->tx_queue_id, CFG_HAL_UART_RXTXCHAR_TIMEOUT);
      if (event.status == osEventMessage)
      {
         // Send char
         dev->huart.Instance->DR = event.value.v;
      }
      else
      {
         // Disable transmit data register empty interrupt
         __HAL_UART_DISABLE_IT(&dev->huart, UART_IT_TXE);
      }
   }

   // Receive char
   if (__HAL_UART_GET_FLAG(&dev->huart, UART_FLAG_RXNE) != RESET)
   {
      uint8_t c = dev->huart.Instance->DR;

      __HAL_UART_CLEAR_FLAG(&dev->huart, UART_FLAG_RXNE);

      if (dev->recv_cb != NULL)
      {
         // Invoke recv callback
         dev->recv_cb(dev->uart, c);
      }
      else
      {
         // Add received char to queue
         osMessagePut(dev->rx_queue_id, c, 0);
      }
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
