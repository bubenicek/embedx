
#include "system.h"

#if ENABLE_TRACE_HAL
TRACE_TAG(hal_dmx);
#else
#include "trace_undef.h"
#endif

#define CFG_HAL_DMX_BAUDRATE              250000

// Locals:
static uint32_t break_delay;


int hal_dmx_init(hal_dmx_t dmx, uint32_t dmx_break_delay)
{
   int res;

   break_delay = dmx_break_delay;

   res = hal_uart_init(dmx);
   res += hal_uart_configure(dmx, CFG_HAL_DMX_BAUDRATE, HAL_UART_2STOPBITS);

   TRACE("Init dmx[%d] break_delay: %d", dmx, dmx_break_delay);

   return res;
}

int hal_dmx_deinit(hal_dmx_t dmx)
{
   return hal_uart_deinit(dmx);
}

int hal_dmx_write(hal_dmx_t dmx, uint8_t *buf, int count)
{
   volatile uint32_t delay;

   // Wait for data was transmited
   hal_uart_sync(dmx);

   // DMX break (88us min)
   hal_uart_configure(dmx, 57600, HAL_UART_2STOPBITS);
   hal_uart_putchar(dmx, 0);

   // Mark after break (8us min)
   for (delay = 0; delay < break_delay; delay++);

   // Start code and data
   hal_uart_configure(dmx, CFG_HAL_DMX_BAUDRATE, HAL_UART_2STOPBITS);

   // Data
   return hal_uart_write(dmx, buf, count);
}
