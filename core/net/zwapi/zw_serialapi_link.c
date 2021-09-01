
#include <stdint.h>
#include <string.h>

#include "zw_config.h"
#include "zw_serialapi_link.h"

#define TRACE_TAG    "zw_link"

#if !ENABLE_TRACE_ZW_SERIALAPI_LINK
#undef TRACE
#undef TRACE_PRINTFF
#undef TRACE_PRINTF
#define TRACE(...)
#define TRACE_PRINTFF(...)
#define TRACE_PRINTF(...)
#endif

// Prototypes:
static void zw_serialapi_link_send_frame(zw_serialapi_link_t *link);
static void zw_serialapi_link_thread(const void *param);
static void ack_timer_callback(void const *arg);
static void data_timer_callback(void const *arg);


/** Open ZW link */
int zw_serialapi_link_open(zw_serialapi_link_t *link)
{
   const osThreadDef(ZW_LINK, zw_serialapi_link_thread, ZW_SERIALAPI_LINK_THREAD_PRIORITY, 0, ZW_SERIALAPI_LINK_THREAD_STACK_SIZE);
   const osMessageQDef(rxQueue, ZW_LINK_RX_FRAMES_SIZE, zw_serialapi_frame_t *);
   const osTimerDef(ack_timer, ack_timer_callback);
   const osTimerDef(data_timer, data_timer_callback);

   memset(link, 0, sizeof(zw_serialapi_link_t));

   // Create timer
   if ((link->ack_timer = osTimerCreate(osTimer(ack_timer), osTimerOnce, link)) == NULL)
   {
      TRACE_ERROR("Create ACK timer");
      goto fail_create_ack_timer;
   }

   if ((link->data_timer = osTimerCreate(osTimer(data_timer), osTimerOnce, link)) == NULL)
   {
      TRACE_ERROR("Create Data timer");
      goto fail_create_data_timer;
   }

   // Create output msg queue
   link->rx_frames.queue_id = osMessageCreate(osMessageQ(rxQueue), 0);
   if (link->rx_frames.queue_id == NULL)
   {
      TRACE_ERROR("Create rx frames queue");
      goto fail_create_rx_queue;
   }

   link->txframe_sem = osSemaphoreCreate(NULL, 1);
   if (link->txframe_sem == NULL)
   {
      TRACE_ERROR("Create txframe semaphore");
      goto fail_create_txframe_sem;
   }
   osSemaphoreWait(link->txframe_sem, osWaitForever);

   if (zw_uart_open() != 0)
   {
      TRACE_ERROR("Open uart");
      goto fail_open_uart;
   }

   // Create Start thread
   link->thread_id = osThreadCreate(osThread(ZW_LINK), link);
   if (link->thread_id == NULL)
   {
      TRACE_ERROR("Start link thread");
      goto fail_create_thread;
   }

   // Reset Zwave module
   hal_gpio_set(GPIO_ZW_RESET, 0);
   osDelay(1);
   hal_gpio_set(GPIO_ZW_RESET, 1);
   osDelay(500);
   
   TRACE("open link");

   return 0;

fail_create_thread:
   VERIFY(zw_uart_close() == 0);
fail_open_uart:
   VERIFY(osSemaphoreDelete(link->txframe_sem) == osOK);
fail_create_txframe_sem:
fail_create_rx_queue:
   VERIFY(osTimerDelete(link->data_timer));
fail_create_data_timer:
   VERIFY(osTimerDelete(link->ack_timer));
fail_create_ack_timer:
   return -1;
}

/** Close ZW link */
int zw_serialapi_link_close(zw_serialapi_link_t *link)
{
   VERIFY(osThreadTerminate(link->thread_id) == osOK);
   VERIFY(zw_uart_close() == 0);
   VERIFY(osSemaphoreDelete(link->txframe_sem) == osOK);
   VERIFY(osTimerDelete(link->ack_timer) == osOK);
   VERIFY(osTimerDelete(link->data_timer) == osOK);

   TRACE("Close link");

   return 0;
}

/** Transmit frame via serial port by adding SOF, Len, Type, cmd and Chksum. */
int zw_serialapi_link_send(zw_serialapi_link_t *link, uint8_t type, uint8_t cmd, uint8_t *data, int datalen)
{
   // Set TX frame data
   link->tx_frame.type = type;
   link->tx_frame.cmd = cmd;
   link->tx_frame.data = data;
   link->tx_frame.datalen = datalen;
   link->tx_frame.retry_cnt = 0;

   // Send frame
   zw_serialapi_link_send_frame(link);

   // Wait until ACK is not received or timeout
   if (osSemaphoreWait(link->txframe_sem, osWaitForever) != osOK)
   {
      TRACE_ERROR("Wait for txframe sem");
      link->tx_state = ZW_SERIALAPI_LINK_TX_STATE_IDLE;
      return -1;
   }

   if (link->tx_state != ZW_SERIALAPI_LINK_TX_STATE_DONE)
   {
      link->tx_state = ZW_SERIALAPI_LINK_TX_STATE_IDLE;
      return -1;
   }

   link->tx_state = ZW_SERIALAPI_LINK_TX_STATE_IDLE;

   return 0;
}

/** Receive frame */
int zw_serialapi_link_recv(zw_serialapi_link_t *link, zw_serialapi_frame_t **frame)
{
   osEvent event;

   // Wait for receive frame
   event = osMessageGet(link->rx_frames.queue_id, osWaitForever);
   if (event.status != osEventMessage)
   {
      return -1;
   }

   *frame = event.value.p;
   link->rx_frames.count--;

   return 0;
}

static void zw_serialapi_link_send_frame(zw_serialapi_link_t *link)
{
   int ix;
   uint8_t crc;

   crc = 0xFF;                                     // Initialize checksum
   zw_uart_putchar(ZW_SOF);
   zw_uart_putchar(link->tx_frame.datalen + 3);    // Remember the 'len', 'type' and 'cmd' bytes
   crc ^= link->tx_frame.datalen + 3;
   zw_uart_putchar(link->tx_frame.type);
   crc ^= link->tx_frame.type;
   zw_uart_putchar(link->tx_frame.cmd);
   crc ^= link->tx_frame.cmd;

   for (ix = 0; ix < link->tx_frame.datalen; ix++)
   {
      zw_uart_putchar(link->tx_frame.data[ix]);
      crc ^= link->tx_frame.data[ix];
   }
   zw_uart_putchar(crc);       // XOR checksum 

   VERIFY(osTimerStart(link->ack_timer, ZW_RX_ACK_TIMEOUT) == osOK);
   link->tx_state = ZW_SERIALAPI_LINK_TX_STATE_ACK;

   TRACE("Send frame type: %d  cmd: 0x%X  len: %d  retry: %d", link->tx_frame.type, link->tx_frame.cmd, link->tx_frame.datalen, link->tx_frame.retry_cnt);
}

static void zw_serialapi_link_resend_frame(zw_serialapi_link_t *link)
{
   if (++link->tx_frame.retry_cnt < ZW_LINK_TX_RETRY_COUNT)
   {
      // Resend frame
      zw_serialapi_link_send_frame(link);
   }
   else
   {
      link->tx_state = ZW_SERIALAPI_LINK_TX_STATE_ERROR;
      VERIFY(osTimerStop(link->ack_timer) == osOK);
      VERIFY(osSemaphoreRelease(link->txframe_sem) == osOK);
   }
}

static void ack_timer_callback(void const *arg)
{
   zw_serialapi_link_t *link = pvTimerGetTimerID((TimerHandle_t *)arg); // arg is handler to freertos timer !!

   TRACE_ERROR("Recv ACK timeout");
   zw_serialapi_link_resend_frame(link);

   hal_wdg_reset();
}

static void data_timer_callback(void const *arg)
{
   zw_serialapi_link_t *link = pvTimerGetTimerID((TimerHandle_t *)arg); // arg is handler to freertos timer !!

   // Reset to SOF hunting
   TRACE_ERROR("Recv DATA timeout  rx_state: %d", link->rx_state);
   link->rx_state = ZW_SERIALAPI_LINK_RX_STATE_SOF;
   link->rx_frames.nerr++;

   hal_wdg_reset();
}

/** ZW link task, should be called periodically */
static void zw_serialapi_link_thread(const void *param)
{
   int c;
   zw_serialapi_link_t *link = (zw_serialapi_link_t *)param;

   TRACE("ZW link thread is running ...");

   while(1)
   {
      // Wait until char is not received 
      if ((c = zw_uart_getchar(osWaitForever)) == -1)
      {
         TRACE_ERROR("ZW uart getchar");
         continue;
      }
         
      switch (link->rx_state)
      {
         case ZW_SERIALAPI_LINK_RX_STATE_SOF:
         {
            if (c == ZW_SOF)
            {
               link->rx_crc = 0xFF;
               link->rxbuf = (uint8_t *)&link->rx_frames.buffer[link->rx_frames.head];
               link->rxbuf_len = 0;
               link->rx_state = ZW_SERIALAPI_LINK_RX_STATE_LEN;
               VERIFY(osTimerStart(link->data_timer, ZW_RX_DATA_TIMEOUT) == osOK);
            }
            else
            {
               if (link->tx_state == ZW_SERIALAPI_LINK_TX_STATE_ACK)
               {
                  VERIFY(osTimerStop(link->ack_timer) == osOK);

                  if (c == ZW_ACK)
                  {
                     TRACE("Recv ACK");

                     // Send done
                     link->tx_state = ZW_SERIALAPI_LINK_TX_STATE_DONE;
                     VERIFY(osSemaphoreRelease(link->txframe_sem) == osOK);
                  }
                  else if (c == ZW_NAK)
                  {
                     TRACE_ERROR("Recv NAK");
                     zw_serialapi_link_resend_frame(link);
                  }
                  else
                  {
                     // Bogus character received...
                  }
               }
            }
         }
         break;

         case ZW_SERIALAPI_LINK_RX_STATE_LEN:
         {
            // Check for length to be inside valid range
            if (c < ZW_FRAME_LENGTH_MIN || c > ZW_FRAME_LENGTH_MAX)
            {
               // Restart looking for SOF
               link->rx_state = ZW_SERIALAPI_LINK_RX_STATE_SOF;
               break;
            }
         }
         // Drop through...

         case ZW_SERIALAPI_LINK_RX_STATE_TYPE:
         {
            if (link->rxbuf_len && (c > ZW_RESPONSE))
            {
               // Restart looking for SOF
               link->rx_state = ZW_SERIALAPI_LINK_RX_STATE_SOF;
               break;
            }
         }
         // Drop through...

         case ZW_SERIALAPI_LINK_RX_STATE_CMD:
            link->rx_state++;
            // Drop through...

         case ZW_SERIALAPI_LINK_RX_STATE_DATA:
         {
            if (link->rxbuf_len < ZW_FRAME_LENGTH_MAX)
            {
               link->rxbuf[link->rxbuf_len] = c;
               link->rx_crc ^= c;
               link->rxbuf_len++;
               if (link->rxbuf_len == link->rxbuf[IDX_LENGTH])
               {
                  link->rx_state++;
               }
            }
            else
            {
               link->rx_state++;
            }
         }
         break;

         case ZW_SERIALAPI_LINK_RX_STATE_CRC:
         {
            VERIFY(osTimerStop(link->data_timer) == osOK);
            
            if (link->tx_state == ZW_SERIALAPI_LINK_TX_STATE_ACK)
            {
               // We are in the process of looking for an acknowledge to a callback request
               // Drop the new frame we received - we don't have time to handle it.
               // Send a CAN to indicate what is happening...
               TRACE_ERROR("Recv frame in ACk state");

               link->rx_frames.nerr++;
               zw_uart_putchar(ZW_NAK); // send ZW_CAN is not working 

               // Reset retry counter
               link->tx_frame.retry_cnt = 0;

               // Resend frame
               zw_serialapi_link_resend_frame(link);
            }
            else
            {
               if (c == link->rx_crc)
               {
                  // Add pointer on the receive buffer to queue
                  if (osMessagePut(link->rx_frames.queue_id,  (uint32_t)&link->rx_frames.buffer[link->rx_frames.head], 1) != osOK)
                  {
                     TRACE_ERROR("RX frames queue is full");
                     zw_uart_putchar(ZW_NAK);
                     link->rx_frames.nerr++;
                  }
                  else
                  {
                     link->rx_frames.count++;
                     TRACE_PRINTFF("Recv frame %d/%d  type: %d  cmd: 0x%X  len: %d  ", link->rx_frames.head, link->rx_frames.count,
                           link->rx_frames.buffer[link->rx_frames.head].type, link->rx_frames.buffer[link->rx_frames.head].cmd,
                           link->rx_frames.buffer[link->rx_frames.head].length);

#if ENABLE_TRACE_ZW_SERIALAPI_LINK_DATA
                     for (c = 0; c < link->rx_frames.buffer[link->rx_frames.head].length; c++)
                        TRACE_PRINTF("%02X ", link->rx_frames.buffer[link->rx_frames.head].data[c]);
#endif
                     TRACE_PRINTF(TRACE_NL);

                     zw_uart_putchar(ZW_ACK);
                     link->rx_frames.head = (link->rx_frames.head + 1) & (ZW_LINK_RX_FRAMES_SIZE-1);
                  }
               }
               else
               {
                  TRACE_ERROR("Recv frame, bad CRC");

                  // Tell them something is wrong...
                  zw_uart_putchar(ZW_NAK);
                  link->rx_frames.nerr++;
               }
            }
         }
         // Drop through

         default :
            // Restart looking for SOF
            link->rx_state = ZW_SERIALAPI_LINK_RX_STATE_SOF;
            break;
      }
   }
}
