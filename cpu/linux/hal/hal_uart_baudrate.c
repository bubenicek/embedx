
#include <stdint.h>
#include <asm/termios.h>

extern int ioctl(int d, int request, ...);
extern int tcflush(int fd, int queue_selector);


/** Set UART custom baudrate */
int uart_set_custom_baudrate(int fd, uint32_t baudrate)
{
   struct termios2 tio;

   if (ioctl(fd, TCGETS2, &tio) < 0)
      return -1;
   
   tio.c_cflag &= ~CBAUD;
   tio.c_cflag |= BOTHER;
   tio.c_ispeed = baudrate;
   tio.c_ospeed = baudrate;
   
   /* do other miscellaneous setup options with the flags here */
   if (ioctl(fd, TCSETS2, &tio) < 0)
      return -1;
   
   // flush the read/write buffer 
   tcflush(fd, TCIOFLUSH);   
   
   return 0;
}
