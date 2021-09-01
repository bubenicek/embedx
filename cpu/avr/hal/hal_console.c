
#include "system.h"

#if defined(CFG_HAL_CONSOLE_SWUART) && (CFG_HAL_CONSOLE_SWUART == 1)
#include "sw_uart.h"
#else
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
#define UDRIE	UDRIE0
#define MPCM    MPCM0

#define UART_BAUD_SELECT(baudRate) ((F_CPU/baudRate/8)-1)
#define  USART_SET_8_1_N   0x006

#endif

int hal_console_init(void)
{
#if defined(CFG_HAL_CONSOLE_SWUART) && (CFG_HAL_CONSOLE_SWUART == 1)
   sw_uart_init();
#else
   uint16_t baudrate = UART_BAUD_SELECT(115200);
   uint8_t setup = USART_SET_8_1_N;

   // the doublespeed selector
   if (baudrate > 0x7ff)
   {
     baudrate += 1;
     baudrate = baudrate >> 1;
     baudrate -= 1;
     UCSRA = 0;
   }
   else
     UCSRA = (1<<U2X);

   // Set the baud rate
   UBRRH = (uint8_t)(0x0f &(baudrate >> 8));
   UBRRL = (uint8_t) baudrate;

   // Set frame format: data bits, stop bits,parity etc
   UCSRC = (1<<URSEL)| setup ; // set the uart bits,baudrate,stop bits

   // Enable UART receiver and transmmitter
   UCSRB = (1<<RXEN)|(1<<TXEN);
#endif
   
   return 0;
}

void hal_console_putchar(char c)
{
#if defined(CFG_HAL_CONSOLE_SWUART) && (CFG_HAL_CONSOLE_SWUART == 1)
   sw_uart_putchar(c);
#else
   // wait for empty transmit buffer
   while (!(UCSRA & (1 << UDRE)));
   UDR = c;
#endif   
}

void hal_console_flush(void)
{
}
