
#include "system.h"

int hal_console_init(void)
{
   return hal_uart_init(CONSOLE_UART);
}

void hal_console_putchar(char c)
{
   hal_uart_putchar(CONSOLE_UART, c);
}

int hal_console_getchar(void)
{
   return 0;
}

/** Flush console output */
void hal_console_flush(void)
{
}
