
#include "system.h"
#include "ebus.h"
#include "crc8.h"
#include "bcd.h"

#define TRACE_TAG "ebus"
#if !ENABLE_TRACE_EBUS
#include "trace_undef.h"
#endif


extern const ebus_cmd_def_t ebus_cmds[];

/** Initialize ebus */
int ebus_init(ebus_t *ebus, hal_uart_t uart, ebus_addr_t src_addr)
{
   memset(ebus, 0, sizeof(ebus_t));
   ebus->uart = uart;
   ebus->src_addr = src_addr;

   if (hal_uart_init(ebus->uart, EBUS_UART_BAUDRATE, 0) != 0)
   {
      TRACE_ERROR("Init uart[%d] failed", ebus->uart);
      throw_exception(fail);
   }

   TRACE("EBUS link initialized");

   return 0;

fail:
   return -1;
}


static int ebus_putchar(ebus_t *ebus, uint8_t c)
{
   int ix, len = 0;
   uint8_t buf[2];

   if (c == 0xAA || c == 0xA9)
   {
      buf[len++] = 0xA9;
      buf[len++] = (c == 0xA9) ? 0 : 1;
   }
   else
   {
      buf[len++] = c;
   }

   for (ix = 0; ix < len; ix++)
   {
      hal_uart_putchar(ebus->uart, buf[ix]);
      if (hal_uart_getchar(ebus->uart) != buf[ix])
         return -1;
   }

   return 0;
}

static int ebus_getchar(ebus_t *ebus)
{
   int c;

   c = hal_uart_getchar(ebus->uart);
   if (c < 0)
   {
      return -1;
   }

   if (c == 0xA9)
   {
      // Escaped char
      c = hal_uart_getchar(ebus->uart);
      if (c == 0x00)
      {
         c = 0xA9;
      }
      else if (c == 0x01)
      {
         c = 0xAA;
      }
      else
      {
        c = -1;
      }
   }

   return c;
}


/** Send request and wait for response is received  */
static int ebus_sendrecv_raw_packet(ebus_t *ebus, uint8_t *txbuf, int txbufsize, uint8_t *rxbuf, int rxbufsize)
{
   int ix, c, rxlen, sync_count = 0;
   uint32_t tmo;
   uint8_t crc;
   int max_sync_count = rand() % CFG_EBUS_VALID_SYNC_COUNT;

   // Count TX buffer CRC
   crc = crc8_buf(txbuf, txbufsize);

   // Wait for SYNC
   tmo = hal_time_ms() + CFG_EBUS_WAITFOR_SYNC_TIMEOUT;
   while(1)
   {
      c = hal_uart_getchar(ebus->uart);
      if (c == EBUS_SYNC)
      {
         if (++sync_count >= max_sync_count)
            break;
      }
      else
      {
         sync_count = 0;
         if (hal_time_ms() > tmo)
         {
            TRACE_ERROR("RX SYNC timeouted, max_sync_count: %d", max_sync_count);
            return -1;
         }
      }
   }

   // Send first data byte and wait for receive echo
   hal_uart_putchar(ebus->uart, *txbuf);
   while(1)
   {
      c = hal_uart_getchar(ebus->uart);
      if (c == *txbuf)
      {
         break;
      }
      else if (c != EBUS_SYNC)
      {
         TRACE_ERROR("TX colision detected");
         return -2;
      }
   }
   txbuf++;
   txbufsize--;

   // Send txbuf
   for(; txbufsize > 0; txbufsize--, txbuf++)
   {
      if (ebus_putchar(ebus, *txbuf) != 0)
      {
         TRACE_ERROR("TX colisin detected, remain send %d bytes", txbufsize);
         return -2;
      }
   }

   // Send CRC
   ebus_putchar(ebus, crc);

   // Read ACK
   c = ebus_getchar(ebus);
   if (c != EBUS_ACK)
   {
      TRACE_ERROR("TX not acked");
      return -1;
   }

   // Read response length
   if ((rxlen = ebus_getchar(ebus)) < 0)
   {
      TRACE_ERROR("RX response len failed");
      return -1;
   }

   // Start count RX CRC
   crc = crc8(rxlen, 0);
   rxbuf[0] = rxlen;

   // Read response data
   for (ix = 0; ix < rxlen; ix++)
   {
      if ((c = ebus_getchar(ebus)) < 0)
      {
         TRACE_ERROR("RX response data failed");
         return -1;
      }

      rxbuf[ix+1] = c;
      crc = crc8(c, crc);
   }

   // Read response CRC
   if ((c = ebus_getchar(ebus)) < 0)
   {
      TRACE_ERROR("RX response CRC failed");
      return -1;
   }

   // Send ACK packet
   ebus_putchar(ebus, EBUS_ACK);

   // Release BUS
   ebus_putchar(ebus, EBUS_SYNC);

   if (c == crc)
   {
      TRACE("RX response datalen: %d  CRC: 0x%X  count CRC: 0x%X", rxlen, c, crc);
      return 0;
   }
   else
   {
      TRACE_ERROR("RX bad response CRC datalen: %d  CRC: 0x%X != CRC: 0x%X", rxlen, c, crc);
      return -1;
   }
}

int ebus_sendrecv_packet(ebus_t *ebus, ebus_request_packet_t *req, ebus_response_packet_t *resp)
{
   int res;
   int trycnt = CFG_EBUS_TXRX_RETRY;

   if (req->src == 0)
      req->src = ebus->src_addr;

   do
   {
      res = ebus_sendrecv_raw_packet(ebus, (uint8_t *)req, EBUS_REQUEST_PACKET_SIZE(req), (uint8_t *)resp, sizeof(ebus_response_packet_t));
      if (res == 0)
         break;

      trycnt--;

   } while(trycnt > 0);

   return res;
}


int ebus_sendrecv_command(ebus_t *ebus, const char *name, const ebus_value_t *param, ebus_value_t *value)
{
    ebus_response_packet_t resp;
    ebus_request_packet_t req;
    const ebus_cmd_def_t *def = NULL;

    for (def = &ebus_cmds[0]; def->name != NULL; def++)
    {
        if (!strcmp(def->name, name))
            break;
    }

    if (def->name == NULL)
    {
        TRACE_ERROR("Undefined command %s", name);
        return -1;
    }

    ASSERT(def->datalen < CFG_EBUS_PACKETLEN);
    ASSERT(def->param.pos < CFG_EBUS_PACKETLEN);
    ASSERT(def->value.pos < CFG_EBUS_PACKETLEN);

    // Fill request
    req.src = ebus->src_addr;
    req.dst = def->dst;
    req.cmd_primary = def->cmd[0];
    req.cmd_second = def->cmd[1];
    req.datalen = def->datalen;
    memcpy(req.data, def->data, def->datalen);

    // Fill param
    if (param != NULL && param->type != EBUS_VALUE_NULL)
    {
        switch(def->param.type)
        {
            case EBUS_VALUE_BYTE:
                req.data[def->param.pos] = ebus_value_byte(param);
                break;

            case EBUS_VALUE_DATA1C:
            {
                req.data[def->param.pos] = ebus_value_data1c(param) * 2;
            }
            break;

            case EBUS_VALUE_DATA2C:
            {
                uint16_t bcd_value;

                bcd_value = dec2bcd(ebus_value_data2c(param) * 10);

                req.data[def->param.pos] = bcd_value & 0xFF;
                req.data[def->param.pos+1] = (bcd_value >> 8) & 0xFF;
            }
            break;

            default:
                break;
        }
    }

    // Send command
    if (ebus_sendrecv_packet(ebus, &req, &resp) != 0)
    {
        TRACE("Send command '%s'", name);
        return -1;
    }

    TRACE("RX %d bytes", resp.datalen);
    TRACE_DUMP(resp.data, resp.datalen);

    // Set reponse value
    if (value != NULL)
    {
        switch(def->value.type)
        {
            case EBUS_VALUE_BYTE:
                ebus_value_set_byte(value, resp.data[def->value.pos]);
                break;

            case EBUS_VALUE_DATA1C:
            {
                float float_value = resp.data[def->value.pos] / 2;
                ebus_value_set_data1c(value, float_value);
            }
            break;

            case EBUS_VALUE_DATA2C:
            {
                uint16_t bcd_value = (resp.data[def->value.pos+1] << 8) | resp.data[def->value.pos];
                int raw_value = bcd2dec(bcd_value);
                float float_value = raw_value / 10.0;

                ebus_value_set_data2c(value, float_value);
            }
            break;

            default:
                ebus_value_set_null(value);
                break;
        }
    }

    return 0;
}
