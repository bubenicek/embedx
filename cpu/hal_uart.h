
#ifndef __HAL_UART_H
#define __HAL_UART_H

#define HAL_UART_2STOPBITS   0000100      // 2 stop bits (oterwise 1 stop bit)

typedef enum
{
   HAL_UART0,
   HAL_UART1,
   HAL_UART2,
   HAL_UART3,
   HAL_UART_MAX

} hal_uart_t;

/** Recv char callback */
typedef void (*hal_uart_recv_cb_t)(hal_uart_t uart, uint8_t c);


/** Open uart */
int hal_uart_init(hal_uart_t uart);

/** Close uart */
int hal_uart_deinit(hal_uart_t uart);

/** Configure UART */
int hal_uart_configure(hal_uart_t uart, uint32_t baudrate, uint32_t settings);

/** Write buffer */
int hal_uart_write(hal_uart_t uart, void *buf, uint16_t count);

/** Read buffer */
int hal_uart_read(hal_uart_t uart, void *buf, uint16_t count, uint16_t timeout);

/** Register receive callback */
int hal_uart_recv(hal_uart_t uart, hal_uart_recv_cb_t recv_cb);

/** Put char */
void hal_uart_putchar(hal_uart_t uart, uint8_t c);

/** Get char */
int hal_uart_getchar(hal_uart_t uart);

/** Synchronize output/input buffers */
void hal_uart_sync(hal_uart_t uart);

#endif // __HAL_UART_H

