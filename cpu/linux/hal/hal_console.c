
#include "system.h"
#include <termios.h>

struct termios initial_settings, new_settings;

int hal_console_init(void)
{
/*
  tcgetattr(0, &initial_settings);

  new_settings.c_lflag &= ~ICANON;
  new_settings.c_lflag &= ~ECHO;
  new_settings.c_lflag &= ~ISIG;
  new_settings.c_cc[VMIN] = 0;
  new_settings.c_cc[VTIME] = 0;
 
  tcsetattr(0, TCSANOW, &new_settings);
*/
  return 0;
}

void hal_console_putchar(char c)
{
   putchar(c);
}

int hal_console_getchar(void)
{
   int c;
   
   c = getchar();
   
   // CTRL+C
   if (c == 3)
   {
      tcsetattr(0, TCSANOW, &initial_settings);
      exit(0);
   }
   
   return c;
}

void hal_console_flush(void)
{
   fflush(stdout);
}
