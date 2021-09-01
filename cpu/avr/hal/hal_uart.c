
#include "system.h"

//
// mapovani registru pro UART0
//
#define UDR     UDR0
#define UCSRA   UCSR0A
#define UCSRB   UCSR0B
#define UBRRL   UBRR0L
#define UBRRH   UBRR0H
#define URSEL   7
#define UCSRC   UCSR0C


#define U2X 	U2X0
#define TXEN 	TXEN0
#define RXEN	RXEN0
#define UDRE	UDRE0
#define RXC		RXC0
#define TXC		TXC0
#define FE		FE0
#define RXCIE	RXCIE0
#define TXCIE	TXCIE0
#define UDRIE	UDRIE0
#define MPCM    MPCM0

#define UART_BAUD_SELECT(baudRate) ((F_CPU/baudRate/8)-1)

// Prototypes:
static void hal_uart_rs485_enable_tx(hal_uart_t uart, uint8_t enable);

// Locals:
static hal_uart_recv_cb_t recv_cb = NULL;

/** Open uart */
int hal_uart_init(hal_uart_t uart)
{
   return 0;
}

/** Configure UART */
int hal_uart_configure(hal_uart_t uart, uint32_t baudrate, uint32_t settings)
{
   baudrate = UART_BAUD_SELECT(baudrate);

   // the doublespeed selector
   if (baudrate > 0x7ff)
   {
     baudrate += 1;
     baudrate = baudrate >> 1;
     baudrate -= 1;
     UCSRA = 0;
   }
   else
   {
      UCSRA = (1 << U2X);
   }

   // Set the baud rate
   UBRRH = (uint8_t)(0x0f &(baudrate >> 8));
   UBRRL = (uint8_t) baudrate;

   // Set frame format: data bits, stop bits,parity etc
   UCSRC = (1 << URSEL) | settings ;

   // Enable UART receiver and transmmitter
   UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);

   // Enable RX
   hal_uart_rs485_enable_tx(uart, 0);

   return 0;
}


/** Close uart */
int hal_uart_deinit(hal_uart_t uart)
{
   recv_cb = NULL;

   return 0;
}

/** Put char */
void hal_uart_putchar(hal_uart_t uart, uint8_t c)
{
   // wait for empty transmit buffer
   while (!(UCSRA & (1 << UDRE)));
   UDR = c;
}

/** Get char */
int hal_uart_getchar(hal_uart_t uart)
{
   uint8_t c;

   // wait for receive byte
   while (!(UCSRA & (1 << RXC)));
   c = UDR;

   return c;
}

/** Write buffer */
int hal_uart_write(hal_uart_t uart, void *buf, uint16_t count)
{
   uint8_t *pbuf = buf;

   // Enable TX
   hal_uart_rs485_enable_tx(uart, 1);

   while(count--)
   {
      // wait for empty transmit buffer
      while (!(UCSRA & (1 << UDRE)));
      UCSRA |= (1 << TXC);
      UDR = *pbuf++;
   }

   // Enable RX
   hal_uart_rs485_enable_tx(uart, 0);
}

/** Read buffer */
int hal_uart_read(hal_uart_t uart, void *buf, uint16_t count, uint16_t timeout)
{
   uint8_t *pbuf = buf;
   int total = 0;

   while(count--)
   {
      // wait for receive byte
      while (!(UCSRA & (1 << RXC)));
      *pbuf = UDR;
      pbuf++;
      total++;
   }

   return total;
}


/** FLush buffers */
void hal_uart_flush(hal_uart_t uart)
{
}

/** Resgiter receive callback */
int hal_uart_recv(hal_uart_t uart, hal_uart_recv_cb_t _recv_cb)
{
   DISABLE_INTERRUPTS();
   recv_cb = _recv_cb;
   ENABLE_INTERRUPTS();

   return 0;
}


static void hal_uart_rs485_enable_tx(hal_uart_t uart, uint8_t enable)
{
   if (enable)
   {
      // Enable TX
      hal_gpio_set(GPIO_RS485_DIR, 1);
   }
   else
   {
      _delay_ms(CFG_HAL_UART_RS485_AFTER_TX_DELAY);

      // Enable RX
      hal_gpio_set(GPIO_RS485_DIR, 0);
   }
}


/** RX Interrupt handler */
ISR(USART_RX_vect)
{
   uint8_t c;

   c = UDR;

   if (recv_cb != NULL)
      recv_cb(HAL_UART0, c);
}
