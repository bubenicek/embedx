
#include "system.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <linux/serial.h>
#include "hal_uart_baudrate.h"

#if ENABLE_TRACE_HAL
TRACE_TAG(hal_uart);
#else
#include "trace_undef.h"
#endif

#ifndef CFG_HAL_UART_READ_TIMEOUT
#define CFG_HAL_UART_READ_TIMEOUT      1000
#endif // CFG_HAL_UART_READ_TIMEOUT

static hal_uart_def_t uart_def[] = CFG_HAL_UART_DEF;
#define NUM_UARTS  (sizeof(uart_def) / sizeof(hal_uart_def_t))


/** Open uart */
int hal_uart_init(hal_uart_t uart)
{
   ASSERT(uart < NUM_UARTS);

   if (uart_def[uart].fd != 0)
   {
      // Uart already initialized
      return 0;
   }

   if ((uart_def[uart].fd = open(uart_def[uart].devname,  O_RDWR | O_NDELAY | O_NOCTTY)) < 0)
   {
      TRACE_ERROR("Failed openu uart[%d] device: %s", uart, uart_def[uart].devname);
      return -1;
   }

   // Configure UART
   if (hal_uart_configure(uart, uart_def[uart].baudrate, uart_def[uart].settings) != 0)
   {
      TRACE_ERROR("Uart[%d] configure failed", uart);
      return -1;
   }

   TRACE("Uart[%d] '%s' baudrate: %d init, fd: %d", uart, uart_def[uart].devname, uart_def[uart].baudrate, uart_def[uart].fd);

   return 0;
}

/** Close uart */
int hal_uart_deinit(hal_uart_t uart)
{
   ASSERT(uart < NUM_UARTS);
   close(uart_def[uart].fd);
   return 0;
}

/** Configure UART */
int hal_uart_configure(hal_uart_t uart, uint32_t baudrate, uint32_t settings)
{
   struct termios options;

   ASSERT(uart < NUM_UARTS);

   // Get the current options for the port...
   tcgetattr(uart_def[uart].fd, &options);

   // Enable the receiver and set local mode
   options.c_cflag |= (CLOCAL | CREAD);
   options.c_cflag &= ~PARENB;
   options.c_cflag &= ~CSTOPB;
   options.c_cflag &= ~CSIZE;
   options.c_cflag |= CS8;

   if (settings & HAL_UART_2STOPBITS)
      options.c_cflag |= CSTOPB;

   // Disable terminal control line
   options.c_iflag = IGNPAR;
   options.c_oflag = 0;
   options.c_lflag = 0;
   options.c_cc[VMIN] = 1;
   options.c_cc[VTIME] = 0;

   // Set the new options for the port...
   tcsetattr(uart_def[uart].fd, TCSANOW, &options);

   // Set baudrate
   if (uart_set_custom_baudrate(uart_def[uart].fd, baudrate) != 0)
   {
      TRACE_ERROR("Uart[%d] set baudrate: %d failed", uart, baudrate);
      return -1;
   }

   return 0;
}

/** Put char */
void hal_uart_putchar(hal_uart_t uart, uint8_t c)
{
   int res;
   ASSERT(uart < NUM_UARTS);
   res = write(uart_def[uart].fd, &c, 1);
   UNUSED(res);
}

/** Get char */
int hal_uart_getchar(hal_uart_t uart)
{
   int res;
   fd_set read_fds;
   struct timeval tv;
   uint8_t c;

   ASSERT(uart < NUM_UARTS);

   // wait for data
   FD_ZERO(&read_fds);
   FD_SET(uart_def[uart].fd, &read_fds);
   tv.tv_sec = 0;
   tv.tv_usec = CFG_HAL_UART_READ_TIMEOUT * 1000;

   if ((res = select(uart_def[uart].fd+1, &read_fds, NULL, NULL,  &tv)) == 0)
   {
      // timeout
      return -2;
   }
   else if (res < 0)
   {
      TRACE_ERROR("select() failed");
      return -1;
   }

   res = read(uart_def[uart].fd, &c, 1);
   if (res < 0)
   {
      TRACE_ERROR("Read failed");
      return -1;
   }

   return c;
}

int hal_uart_write(hal_uart_t uart, void *buf, uint16_t count)
{
   ASSERT(uart < NUM_UARTS);
   return write(uart_def[uart].fd, buf, count);
}

int hal_uart_read(hal_uart_t uart, void *buf, uint16_t count, uint16_t timeout)
{
   int res;
   fd_set read_fds;
   struct timeval tv;

   ASSERT(uart < NUM_UARTS);

    // wait for data
    FD_ZERO(&read_fds);
    FD_SET(uart_def[uart].fd, &read_fds);
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;

    if ((res = select(uart_def[uart].fd+1, &read_fds, NULL, NULL,  &tv)) == 0)
    {
        // timeout
        return -2;
    }
    else if (res < 0)
    {
        TRACE_ERROR("select() failed");
        return -1;
    }

    res = read(uart_def[uart].fd, buf, count);
    if (res < 0)
    {
        TRACE_ERROR("Read failed, errno: %d", errno);
        return -1;
    }

    return res;
}


/** Sync output buffers */
void hal_uart_sync(hal_uart_t uart)
{
   ASSERT(uart < NUM_UARTS);
   tcdrain(uart_def[uart].fd);
}

/** Register receive callback */
int hal_uart_recv(hal_uart_t uart, hal_uart_recv_cb_t _recv_cb)
{
   ASSERT(uart < NUM_UARTS);
   return 0;
}
