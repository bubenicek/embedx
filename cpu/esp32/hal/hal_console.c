
#include "system.h"

int hal_console_init(void)
{
  return 0;
}

void hal_console_putchar(char c)
{
   putchar(c);
   if (c == '\n')
      fflush(stdout);
}

void hal_console_flush(void)
{
   fflush(stdout);
}
