
#include "system.h"

// Prototypes:
static void hal_uart_rs485_enable_tx(hal_uart_t uart, uint8_t enable);

// Locals:
static volatile hal_uart_recv_cb_t recv_cb = NULL;

/** Open uart */
int hal_uart_init(hal_uart_t uart, uint32_t baudrate, uint32_t settings)
{      
   UNUSED(uart);
   UNUSED(baudrate);
   UNUSED(settings);
   
   // TODO: set params from args
   
   // Configure the UART1 
   UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO, UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
   UART1_ITConfig(UART1_IT_RXNE_OR, ENABLE);
   UART1_Cmd(ENABLE);
   
   // Enable RX
   hal_uart_rs485_enable_tx(uart, 0);
         
   return 0;
}

/** Close uart */
int hal_uart_deinit(hal_uart_t uart)
{
   UNUSED(uart);
   recv_cb = NULL;
   return 0;
}

/** Put char */
void hal_uart_putchar(hal_uart_t uart, uint8_t c)
{
   UNUSED(uart);
   while(!(UART1->SR & UART1_FLAG_TXE));
   UART1->DR = c;
}

/** Get char */
int hal_uart_getchar(hal_uart_t uart)
{
   uint8_t c;
   
   UNUSED(uart);
   
   while(!(UART1->SR & UART1_FLAG_RXNE));
   c = UART1->DR;
   
   return c;
}

/** Write buffer */
int hal_uart_write(hal_uart_t uart, void *buf, uint16_t count)
{
   int total = count;
   uint8_t *pbuf = buf;
   
   // Enable TX
   hal_uart_rs485_enable_tx(uart, 1);
   
   while(count--)
   {
      while(!(UART1->SR & UART1_FLAG_TXE));
      UART1->DR = *pbuf++;
   }
   
   // Enable RX
   hal_uart_rs485_enable_tx(uart, 0);
   
   return total;
}

/** Read buffer */
int hal_uart_read(hal_uart_t uart, void *buf, uint16_t count, uint16_t timeout)
{
   uint8_t *pbuf = buf;
   int total = 0;
   
   UNUSED(uart);
   UNUSED(timeout);
   
   while(count--)
   {
      while(!(UART1->SR & UART1_FLAG_RXNE));
      *pbuf = UART1->DR;   
      pbuf++;
      total++;
   }
      
   return total;
}


/** FLush buffers */
void hal_uart_flush(hal_uart_t uart)
{
   UNUSED(uart);
}

/** Resgiter receive callback */
void hal_uart_recv(hal_uart_t uart, hal_uart_recv_cb_t _recv_cb)
{
   UNUSED(uart);
   
   DISABLE_INTERRUPTS();
   recv_cb = _recv_cb;
   ENABLE_INTERRUPTS();
}


static void hal_uart_rs485_enable_tx(hal_uart_t uart, uint8_t enable)
{
   UNUSED(uart);
   
   if (enable)
   {
      // Enable TX
      hal_gpio_set(GPIO_RS485_DIR, 1);
   }
   else
   {
      hal_delay_ms(CFG_HAL_UART_RS485_AFTER_TX_DELAY);          
     
      // Enable RX
      hal_gpio_set(GPIO_RS485_DIR, 0);
   }
}


/** UART1 RX Interrupt routine */
INTERRUPT_HANDLER(UART1_RX_IRQHandler, 18)
{
   uint8_t c;
   
   c = UART1->DR;
     
   if (recv_cb != NULL)
      recv_cb(HAL_UART0, c);
}
