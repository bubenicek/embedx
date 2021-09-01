
#include "system.h"
#include "driver/uart.h"

TRACE_TAG(hal_uart);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

#ifndef CFG_SERIAL_RX_BUFSIZE
#define CFG_SERIAL_RX_BUFSIZE       256
#endif

#ifndef CFG_SERIAL_TX_BUFSIZE
#define CFG_SERIAL_TX_BUFSIZE       256
#endif

#ifndef CFG_SERIAL_RX_TIMEOUT
#define CFG_SERIAL_RX_TIMEOUT       1000
#endif

static const hal_uart_def_t uart_def[] = CFG_HAL_UART_DEF;
#define NUM_UARTS    (sizeof(uart_def) / sizeof(hal_uart_def_t))


/** Open uart */
int hal_uart_init(hal_uart_t uart)
{
   uart_config_t uart_config =
   {
      .baud_rate = uart_def[uart].baudrate,
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
   };

   ASSERT(uart < NUM_UARTS);

   if (uart_param_config(uart_def[uart].uart, &uart_config) != ESP_OK)
   {
      TRACE_ERROR("uart_param_config failed");
      return -1;
   }

   if (uart_set_pin(uart_def[uart].uart, uart_def[uart].txpin, uart_def[uart].rxpin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK)
   {
      TRACE_ERROR("uart_set_pin failed");
      return -1;
   }

   if (uart_driver_install(uart_def[uart].uart, CFG_SERIAL_RX_BUFSIZE, CFG_SERIAL_TX_BUFSIZE, 0, NULL, 0) != ESP_OK)
   {
      TRACE_ERROR("uart_driver_install failed");
      return -1;
   }

   return 0;
}

/** Close uart */
int hal_uart_deinit(hal_uart_t uart)
{
   return 0;
}

/** Set baudrate */
int hal_uart_configure(hal_uart_t uart, uint32_t baudrate, uint32_t settings)
{
   ASSERT(uart < NUM_UARTS);
   return uart_set_baudrate(uart_def[uart].uart, baudrate);
}

/** Put char */
void hal_uart_putchar(hal_uart_t uart, uint8_t c)
{
   ASSERT(uart < NUM_UARTS);
   uart_write_bytes(uart_def[uart].uart, (const char *) &c, 1);
}

/** Get char */
int hal_uart_getchar(hal_uart_t uart)
{
   uint8_t c;
   ASSERT(uart < NUM_UARTS);
   return (uart_read_bytes(uart_def[uart].uart, &c, 1, CFG_SERIAL_RX_TIMEOUT/ portTICK_RATE_MS) == 1) ? c : -1;
}

int hal_uart_write(hal_uart_t uart, void *buf, uint16_t count)
{
   ASSERT(uart < NUM_UARTS);
   return uart_write_bytes(uart_def[uart].uart, (const char *) buf, count);
}

int hal_uart_read(hal_uart_t uart, void *buf, uint16_t count, uint16_t timeout)
{
   ASSERT(uart < NUM_UARTS);
   return uart_read_bytes(uart_def[uart].uart, buf, count, timeout > 0 ? (timeout / portTICK_RATE_MS) : portMAX_DELAY);
}

/** Sync output buffers */
void hal_uart_sync(hal_uart_t uart)
{
   ASSERT(uart < NUM_UARTS);
   uart_flush(uart_def[uart].uart);
}

/** Register receive callback */
int hal_uart_recv(hal_uart_t uart, hal_uart_recv_cb_t _recv_cb)
{
   TRACE_ERROR("Not supported !!!");
   return -1;
}
