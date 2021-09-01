
#include <string.h>

#include "system.h"
#include "modbus.h"

TRACE_TAG(modbus);
#if !ENABLE_TRACE_MODBUS
#include "trace_undef.h"
#endif


/** Delay after data ws transmited */
#ifndef CFG_MODBUS_DELAY_AFTER_TX
#define CFG_MODBUS_DELAY_AFTER_TX    	       0
#endif

/** Retry count of transmit request */
#ifndef CFG_MODBUS_REQUEST_RETRY_CNT
#define CFG_MODBUS_REQUEST_RETRY_CNT	       3
#endif

#ifndef CFG_MODBUS_USE_RX_CALBACK
#define CFG_MODBUS_USE_RX_CALBACK             0
#endif // CFG_MODBUS_RX_CLBK


#ifndef PROGMEM
#define PROGMEM
#define pgm_read_byte(x) *x
#endif // PROGMEM


#if defined (CFG_MODBUS_USE_RX_CALBACK) && (CFG_MODBUS_USE_RX_CALBACK == 1)
typedef struct
{
   uint8_t addr;
   hal_uart_t uart;
   modbus_rtu_recv_cb_t recv_cb;
   uint16_t recv_timeout;
   os_timer_t timer;

   struct
   {
      volatile uint8_t state;
      uint16_t crc_calc;

      uint8_t addr;
      uint8_t func;
      uint16_t regaddr;
      uint16_t regdata;
      uint16_t crc;

   } rx;

} modbus_rtu_context_t;

static void hal_uart_recv_cb(hal_uart_t uart, uint8_t c);

#else

typedef struct
{
   uint16_t recv_timeout;

} modbus_rtu_context_t;

#endif

static modbus_rtu_context_t rtu_ctx[HAL_UART_MAX];

/* Table of CRC values for high-order byte */
static const PROGMEM uint8_t table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
static const PROGMEM uint8_t table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

static uint16_t modbus_crc16(uint16_t crc, uint8_t *buffer, uint16_t buffer_length)
{
    uint8_t crc_hi = crc >> 8; /* high CRC byte initialized */
    uint8_t crc_lo = crc & 0xFF; /* low CRC byte initialized */
    unsigned int i; /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_hi ^ *buffer++; /* calculate the CRC  */
        crc_hi = crc_lo ^ pgm_read_byte(&table_crc_hi[i]);
        crc_lo = pgm_read_byte(&table_crc_lo[i]);
    }

    return (crc_hi << 8 | crc_lo);
}

/** Initialize MODBUS RTU */
int modbus_rtu_init(hal_uart_t uart, uint32_t baudrate, uint32_t uart_settings, uint16_t recv_timeout)
{
   ASSERT(uart < HAL_UART_MAX);

#if defined (CFG_MODBUS_USE_RX_CALBACK) && (CFG_MODBUS_USE_RX_CALBACK == 1)
   memset(&rtu_ctx, 0, sizeof(rtu_ctx));
#endif

   rtu_ctx[uart].recv_timeout = recv_timeout;

   if (hal_uart_init(uart) != 0)
      return -1;

   if (hal_uart_configure(uart, baudrate, 0) != 0)
      return -1;

   TRACE("RTU uart[%d] init, recv_timeout: %d", uart, rtu_ctx[uart].recv_timeout);

   return 0;
}

/** Read RTU response */
int modbus_rtu_read_response(hal_uart_t uart, int addr, int rspsize, uint8_t *buf, int bufsize)
{
   int res, length;
   uint16_t crc_calculated;
   uint16_t crc_received;

   UNUSED(bufsize);
   UNUSED(addr);

   ASSERT(uart < HAL_UART_MAX);

   length = rspsize + 2;  // included CRC

   //TRACE("Wait for response from addr: %d   rspsize: %d", addr, rspsize);

   if ((res = hal_uart_read(uart, buf, length, rtu_ctx[uart].recv_timeout)) != length)
   {
      TRACE_ERROR("Read response from device addr: %d (expected %d bytes, recv %d bytes)", addr, length, res);
      return -1;
   }

#if ENABLE_TRACE_MODBUS_DATA
{
   int ix;
   TRACE_PRINTFF("RX resp: ");
   for (ix = 0; ix < length; ix++)
   {
      TRACE_PRINTF("%2.2X ", buf[ix]);
   }
   TRACE_PRINTF("\n");
}
#endif

   crc_calculated = modbus_crc16(0xFFFF, buf, rspsize);
   crc_received = (buf[length - 2] << 8) | buf[length - 1];

   if (crc_calculated != crc_received)
   {
      TRACE_ERROR("Bad response CRC from device addr: %d", addr);
      return -1;
   }

   return rspsize;
}

/** Write RTU response */
int modbus_rtu_write_response(hal_uart_t uart, int addr, uint8_t func, uint8_t *data, int datalen)
{
   int len = 0;
   uint16_t crc;
   uint8_t buf[64];

   buf[len++] = addr;
   buf[len++] = func;
   while(datalen--)
      buf[len++] = *data++;

   crc = modbus_crc16(0xFFFF, buf, len);
   buf[len++] = crc >> 8;
   buf[len++] = crc & 0x00FF;

#if ENABLE_TRACE_MODBUS_DATA
{
   int ix;
   TRACE_PRINTFF("TX resp: ");
   for (ix = 0; ix < len; ix++)
   {
      TRACE_PRINTF("%2.2X ", buf[ix]);
   }
   TRACE_PRINTF("\n");
}
#endif

   return hal_uart_write(uart, buf, len);
}

/** Write RTU request */
int modbus_rtu_write_request(hal_uart_t uart, int addr, uint8_t func, uint16_t regaddr, uint16_t regdata)
{
   int len = 0;
   uint16_t crc;
   uint8_t buf[64];

   buf[len++] = addr;
   buf[len++] = func;
   buf[len++] = (regaddr >> 8);
   buf[len++] = regaddr & 0xFF;
   buf[len++] = (regdata >> 8);
   buf[len++] = regdata & 0xFF;

   crc = modbus_crc16(0xFFFF, buf, len);
   buf[len++] = crc >> 8;
   buf[len++] = crc & 0x00FF;

#if ENABLE_TRACE_MODBUS_DATA
{
   int ix;
   TRACE_PRINTFF("TX req: ");
   for (ix = 0; ix < len; ix++)
   {
      TRACE_PRINTF("%2.2X ", buf[ix]);
   }
   TRACE_PRINTF("\n");
}
#endif

   return hal_uart_write(uart, buf, len);
}

/** Write RTU coils state */
int modbus_rtu_write_coil(hal_uart_t uart, int addr, int coil, int state)
{
   uint8_t buf[32];
   int retry;

   for (retry = 0; retry < CFG_MODBUS_REQUEST_RETRY_CNT; retry++)
   {
      // Write request fix request
      if (modbus_rtu_write_request(uart, addr, MODBUS_FUNC_WRITE_COIL, coil, state) < 0)
         return -1;
#if CFG_MODBUS_DELAY_AFTER_TX > 0
      hal_delay_ms(CFG_MODBUS_DELAY_AFTER_TX);
#endif
      // Read response
      if (modbus_rtu_read_response(uart, addr, 6, buf, sizeof(buf)) > 0)
         return 0;
   }

   return -1;
}

/** Read RTU coils state */
int modbus_rtu_read_coils_state(hal_uart_t uart, int addr, int start_coil, int count, uint16_t *state)
{
   int retry, rsplen;
   uint8_t buf[64];

   *state = 0;

   for (retry = 0; retry < CFG_MODBUS_REQUEST_RETRY_CNT; retry++)
   {
      // Write request
      if (modbus_rtu_write_request(uart, addr, MODBUS_FUNC_READ_COILS, start_coil, count) < 0)
         return -1;
#if CFG_MODBUS_DELAY_AFTER_TX > 0
      hal_delay_ms(CFG_MODBUS_DELAY_AFTER_TX);
#endif
      rsplen = 3; // addr + func + size
      rsplen += count / 8;

      // Read response
      if (modbus_rtu_read_response(uart, addr, rsplen, buf, sizeof(buf)) > 0)
      {
         *state = buf[MODBUS_RTU_DATA_IDX+1];
         if (count / 8 == 2)
            *state |= (buf[MODBUS_RTU_DATA_IDX+2] << 8);

         return 0;
      }
   }

   return -1;
}

/** Read discrete inputs state */
int modbus_rtu_read_inputs(hal_uart_t uart, int addr, int start_input, int count, uint16_t *state)
{
   int retry, rsplen;
   uint8_t buf[32];

   //TRACE("%s   addr: %d  start: %d  count: %d", __FUNCTION__, addr, start_input, count);

   for (retry = 0; retry < CFG_MODBUS_REQUEST_RETRY_CNT; retry++)
   {
      // Write request fix request
      if (modbus_rtu_write_request(uart, addr, MODBUS_READ_DISCRETE_INPUTS, start_input, count) < 0)
         return -1;
#if CFG_MODBUS_DELAY_AFTER_TX > 0
      hal_delay_ms(CFG_MODBUS_DELAY_AFTER_TX);
#endif
      rsplen = 3; // addr + func + size
      rsplen += count / 8;

      // Read response
      if (modbus_rtu_read_response(uart, addr, rsplen, buf, sizeof(buf)) > 0)
      {
         *state = buf[MODBUS_RTU_DATA_IDX+1];
         if (count / 8 == 2)
            *state |= (buf[MODBUS_RTU_DATA_IDX+2] << 8);

         return 0;
      }
   }

   return -1;
}

int modbus_rtu_write_sigle_register(hal_uart_t uart, int addr, int regaddr, int value)
{
   uint8_t buf[16];
   int retry;

   for (retry = 0; retry < CFG_MODBUS_REQUEST_RETRY_CNT; retry++)
   {
      // Write request fix request
      if (modbus_rtu_write_request(uart, addr, MODBUS_FUNC_WRITE_SINGLE_REGISTER, regaddr, value) < 0)
         return -1;

#if CFG_MODBUS_DELAY_AFTER_TX > 0
      hal_delay_ms(CFG_MODBUS_DELAY_AFTER_TX);
#endif
      // Read response
      if (modbus_rtu_read_response(uart, addr, 6, buf, sizeof(buf)) > 0)
         return 0;
   }

   return -1;
}

int modbus_rtu_read_holding_register(hal_uart_t uart, int addr, int start, int count, uint16_t *regs)
{
   int ib, ir;
   int retry, rsplen;
   uint8_t buf[255];

   for (retry = 0; retry < CFG_MODBUS_REQUEST_RETRY_CNT; retry++)
   {
      // Write request fix request
      if (modbus_rtu_write_request(uart, addr, MODBUS_READ_HOLDING_REGISTER, start, count) < 0)
         return -1;

#if CFG_MODBUS_DELAY_AFTER_TX > 0
      hal_delay_ms(CFG_MODBUS_DELAY_AFTER_TX);
#endif
      rsplen = 3; // addr + func + size
      rsplen += count * 2;

      // Read response
      if (modbus_rtu_read_response(uart, addr, rsplen, buf, sizeof(buf)) > 0)
      {
         if (buf[MODBUS_RTU_DATA_IDX] != count * 2)
         {
            TRACE_ERROR("Returned bad number of registers");
            return -1;
         }

         for (ib = 0, ir = 0; ib < buf[MODBUS_RTU_DATA_IDX]; ib += 2, ir++)
         {
            regs[ir] = (buf[MODBUS_RTU_DATA_IDX+1+ib] << 8) | buf[MODBUS_RTU_DATA_IDX+2+ib];
         }

         return 0;
      }
   }

   return -1;
}



#if defined (CFG_MODBUS_USE_RX_CALBACK) && (CFG_MODBUS_USE_RX_CALBACK == 1)

/** Register RTU recv request callback */
int modbus_rtu_recv(hal_uart_t uart, uint8_t addr, modbus_rtu_recv_cb_t recv_cb)
{
   ASSERT(uart < CFG_HAL_NUM_UARTS);

   rtu_ctx[uart].addr = addr;
   rtu_ctx[uart].uart = uart;
   rtu_ctx[uart].recv_cb = recv_cb;
   hal_uart_recv(uart, hal_uart_recv_cb);

   TRACE("RTU receiver init, addr:0x%X", addr);

   return 0;
}

static void modbus_rtu_request_task(void *arg)
{
   modbus_rtu_context_t *ctx = arg;

//   TRACE("RX addr: 0x%X  crc: 0x%X  calc_crc: 0x%X", ctx->rx.addr, ctx->rx.crc, ctx->rx.crc_calc);

   if (ctx->recv_cb != NULL)
   {
      if (ctx->recv_cb(ctx->uart, ctx->rx.addr, ctx->rx.func, ctx->rx.regaddr, ctx->rx.regdata) < 0)
      {
         // Send error response
         uint8_t data = 0x1;
         modbus_rtu_write_response(ctx->uart, ctx->rx.addr, ctx->rx.func | 0x80, &data, 1);
      }
   }
}

static void modbus_rtu_rx_timeout_cb(void *arg)
{
   modbus_rtu_context_t *ctx = arg;
   ctx->rx.state = 0;
   hal_led_set(LED_SYSTEM, 0);
}

/** Receive char from UART (executed in IRQ context) */
static void hal_uart_recv_cb(hal_uart_t uart, uint8_t c)
{
   modbus_rtu_context_t *ctx = &rtu_ctx[uart];

   switch(ctx->rx.state)
   {
      case 0:
         ctx->rx.addr = c;
         ctx->rx.crc_calc = modbus_crc16(0xFFFF, &c, 1);
         os_timer_start(&ctx->timer, OS_TIMER_ONESHOT, ctx->recv_timeout, modbus_rtu_rx_timeout_cb, ctx);
         ctx->rx.state++;
         break;

      case 1:
         os_timer_reset(&ctx->timer);
         ctx->rx.func = c;
         ctx->rx.crc_calc = modbus_crc16(ctx->rx.crc_calc, &c, 1);
         ctx->rx.state++;
         break;

      case 2:
         os_timer_reset(&ctx->timer);
         ctx->rx.regaddr = (c << 8);
         ctx->rx.crc_calc = modbus_crc16(ctx->rx.crc_calc, &c, 1);
         ctx->rx.state++;
         break;

      case 3:
         os_timer_reset(&ctx->timer);
         ctx->rx.regaddr |= c;
         ctx->rx.crc_calc = modbus_crc16(ctx->rx.crc_calc, &c, 1);
         ctx->rx.state++;
         break;

      case 4:
         os_timer_reset(&ctx->timer);
         ctx->rx.regdata = (c << 8);
         ctx->rx.crc_calc = modbus_crc16(ctx->rx.crc_calc, &c, 1);
         ctx->rx.state++;
         break;

      case 5:
         os_timer_reset(&ctx->timer);
         ctx->rx.regdata |= c;
         ctx->rx.crc_calc = modbus_crc16(ctx->rx.crc_calc, &c, 1);
         ctx->rx.state++;
         break;

      case 6:
         os_timer_reset(&ctx->timer);
         ctx->rx.crc = (c << 8);
         ctx->rx.state++;
         break;

      case 7:
      {
         ctx->rx.crc |= c;
         ctx->rx.state = 0;

         if (ctx->addr == ctx->rx.addr && ctx->rx.crc == ctx->rx.crc_calc)
         {
            // Process incomming request in task context
            os_scheduler_push_task(modbus_rtu_request_task, OS_TASKPRIO_MAX, ctx);
            hal_led_toggle(LED_SYSTEM);
         }
      }
      break;

      default:
         ctx->rx.state = 0;
   }
}

#endif   // CFG_MODBUS_USE_RX_CALBACK
