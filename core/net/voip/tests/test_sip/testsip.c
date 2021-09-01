/**
 * \file testsip.c      \brief Test SIP and RTP protocol
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arch/avr32/network-device.h>
#include <vs1053/duplex.h>

#include "process.h"
#include "debug.h"
#include "testsip.h"


#define GEN_SINWAVE                0

#define SAMPLES_PER_FRAME          160             //! G711 samples per frame
#define CLOCK_RATE                 8000            //! sampling freq

//----------------------------------------------------------------------------------------
// locale prototypes:

static void on_register(sip_con_t *sc, const sip_profile_t *prof);
static void on_unregister(sip_con_t *sc, const sip_profile_t *prof);
static void on_error(sip_con_t *sc, uint16_t sipErrCode, uint8_t errcode);
static void on_invite(sip_con_t *sc, const char *remoteUri, const char *remoteAddr, uint16_t localPort, uint16_t remotePort);
static void on_invite_ringing(sip_con_t *sc, const char *remoteUri);
static void on_invite_declined(sip_con_t *sc, const char *remoteUri);
static void on_hangup(sip_con_t *sc, const char *remoteUri);
static void on_incomming_invite_ringing(sip_con_t *sc, const char *remoteUri);

static uint16_t on_rtp_send_frame_data(void *buf, uint16_t buflen);
static void on_rtp_receive_frame_data(void *buf, uint16_t count);

//-----------------------------------------------------------------------------------------

#if GEN_SINWAVE
static const uint8_t sinwave[] = {128,38,1,38,128,218,255,218};
static int sinwave_index = 0;
#endif

//
// SIP profile settings
//
#if 1
static const sip_profile_t sprofile =
{
   .srv_addr = "213.168.165.12",
   .srv_realm = "sip2.fayn.cz",
   .username = "212241405",
   .passwd = "MHTDrE8WQD",
};
#else
static const sip_profile_t sprofile =
{
   .srv_addr = "89.185.255.43",
   .srv_realm = "89.185.255.43", //"asterisk",
   .username = "394776",
   .passwd = "42bWjauwJ4yc8f",
};
#endif

//
// SIP callbacks definitions
//
static const sip_con_events_t sip_events =
{
   .on_register = on_register,
   .on_unregister = on_unregister,
   .on_invite = on_invite,
   .on_invite_ringing = on_invite_ringing,
   .on_invite_declined = on_invite_declined,
   .on_incomming_invite_ringing = on_incomming_invite_ringing,
   .on_hangup = on_hangup,
   .on_error = on_error,
};

//
// RTP callbacks
//
static const rtp_connection_events_t rtp_events =
{
   .on_send_frame_data = on_rtp_send_frame_data,
   .on_receive_frame_data = on_rtp_receive_frame_data,
};

//---------------------------------------------------------------------

/** calling event when connection success registered */
static void on_register(sip_con_t *sc, const sip_profile_t *prof)
{
   debug("SIP_EVENT: %s - %s", __FUNCTION__, prof->srv_realm);
}

/** calling when connection success unregistered */
static void on_unregister(sip_con_t *sc, const sip_profile_t *prof)
{
   debug("SIP_EVENT: %s - %s", __FUNCTION__, prof->srv_realm);
}

static void on_invite(sip_con_t *sc, const char *remoteUri, const char *remoteAddr, uint16_t remotePort, uint16_t localPort)
{
   debug("SIP_EVENT: %s - uri:%s  %s:%d  *:%d",
         __FUNCTION__, remoteUri, remoteAddr, remotePort, localPort);

   // create RTP connection
   testsip_state_t *s = sc->userData;
   s->rtpcon = rtp_create_connection(
                     remoteAddr,
                     remotePort,
                     localPort,
                     RTP_PCMA_PAYLOADTYPE,
                     SAMPLES_PER_FRAME,
                     CLOCK_RATE,
                     &rtp_events);

   s->acceptCall = 0;
}

static void on_invite_ringing(sip_con_t *sc, const char *remoteUri)
{
   debug("SIP_EVENT: %s - %s", __FUNCTION__, remoteUri);
}

static void on_incomming_invite_ringing(sip_con_t *sc, const char *remoteUri)
{
   debug("SIP_EVENT: %s - %s", __FUNCTION__, remoteUri);
   testsip_state_t *s = sc->userData;
   timer_set(&s->acptimer, ACCEPT_TIMER_INTERVAL);
   s->acceptCall = 1;
}

static void on_invite_declined(sip_con_t *sc, const char *remoteUri)
{
   debug("SIP_EVENT: %s - %s", __FUNCTION__, remoteUri);
   testsip_state_t *s = sc->userData;
   s->acceptCall = 0;
}

static void on_hangup(sip_con_t *sc, const char *remoteUri)
{
   debug("SIP_EVENT: %s - %s", __FUNCTION__, remoteUri);

   // destroy RTP connection
   testsip_state_t *s = sc->userData;
   s->acceptCall = 0;

   if (s->rtpcon != NULL)
   {
      rtp_destroy_connection(s->rtpcon);
      s->rtpcon = NULL;
   }
}

static void on_error(sip_con_t *sc, uint16_t sipErrCode, uint8_t errcode)
{
   error("SIP_EVENTS: %s - sipErrCode = %d  errCode = %d", __FUNCTION__, sipErrCode, errcode);
   testsip_state_t *s = sc->userData;
   s->acceptCall = 0;

   // destroy RTP connection
    if (s->rtpcon != NULL)
   {
      rtp_destroy_connection(s->rtpcon);
      s->rtpcon = NULL;
   }
}

//
// RTP callbacks
//

/** send audio samples */
static uint16_t on_rtp_send_frame_data(void *buf, uint16_t buflen)
{
#if GEN_SINWAVE
   int ix;
   pj_size_t len;

   // copy sin wave to buffer
   for (ix = 0; ix < SAMPLES_PER_FRAME * 2; ix++)
   {
      pcmBuf[ix] = sinwave[sinwave_index++] * 10;
      if (sinwave_index > sizeof(sinwave))
         sinwave_index = 0;
   }

   // G711 encoding
   g711_enc_encode(pcmBuf, SAMPLES_PER_FRAME, outBuf, &len);

   *pbuf = outBuf;
   return len;
#else

   // read G711 PCMA data
   return vs_duplex_read_pcma(buf, SAMPLES_PER_FRAME);

#endif
}

static void on_rtp_receive_frame_data(void *buf, uint16_t count)
{
   // write G711 PCMA samples
   vs_duplex_write_pcma(buf, count);
   debug("RTP receive %d bytes", count);
}

static PT_THREAD(testsip_process(testsip_state_t *s))
{
   PT_BEGIN(&s->pt);

   // wait for network device ia sctive
   PT_WAIT_UNTIL(&s->pt, network_device_active());
   debug("\nStart testsip - network device is active");

   // create SIP connection
   if ((s->scon = sip_create_con(sprofile.srv_addr, &sip_events, s)) == NULL)
   {
      error("create SIP connection failed");
      PT_EXIT(&s->pt);
   }

   // register SIP connection
   sip_register(s->scon, &sprofile);
   // wait for succsee registration
   PT_WAIT_UNTIL(&s->pt, sip_con_registered(s->scon));
   debug("SIP connection success registered");

   while(1)
   {
      if (s->cmd != TESTSIP_CMD_NONE)
      {
         if (s->cmd == TESTSIP_CMD_CALL)
         {
            sip_invite(s->scon, s->callcontact);
         }
         else if (s->cmd == TESTSIP_CMD_HANGUP)
         {
            sip_hangup(s->scon);
            PT_WAIT_WHILE(&s->pt, sip_con_invited(s->scon));
         }
         else if (s->cmd == TESTSIP_CMD_ACCEPT)
         {
            sip_accept_invite(s->scon);
         }

         s->cmd = TESTSIP_CMD_NONE;
      }

      if (s->acceptCall && timer_expired(&s->acptimer))
      {
         debug("--- auto accept timer ---");
         sip_accept_invite(s->scon);
         s->acceptCall = 0;
      }

      PT_YIELD(&s->pt);
   }

   PT_END(&s->pt);
}

void testsip_init(testsip_state_t *s)
{
   memset(s, 0, sizeof(testsip_state_t));
   PT_INIT(&s->pt);

    // initialize duplex for play and recording voice
   vs_duplex_init();
   vs_duplex_setvolume(100);

   // initialize SIP stack
   sip_init();

   create_process("testsip", (process_function_t)testsip_process, s);
}

void testsip_call(testsip_state_t *s, char *contact)
{
   if (s->cmd != TESTSIP_CMD_NONE)
   {
      error("testsip call is busy");
      return;
   }

   s->callcontact = contact;
   s->cmd = TESTSIP_CMD_CALL;
}

void testsip_hangup(testsip_state_t *s)
{
   if (s->cmd != TESTSIP_CMD_NONE)
   {
      error("testsip call is busy");
      return;
   }

   s->cmd = TESTSIP_CMD_HANGUP;
}

void testsip_accept(testsip_state_t *s)
{
   if (s->cmd != TESTSIP_CMD_NONE)
   {
      error("testsip call is busy");
      return;
   }

   s->cmd = TESTSIP_CMD_ACCEPT;
}
