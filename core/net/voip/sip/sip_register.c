/**
 * \file sip_register.c        \brief SIP register dialog processor
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sip.h"

#define TRACE_TAG "sip-register"
#if !ENABLE_TRACE_SIP_REGISTER
#undef TRACE
#define TRACE(...)
#endif


/** close registration */
static void close_register(sip_con_t *s)
{
   // set number of register trying
   s->reg_sndTryCnt = SIP_SEND_TRYCNT;

   s->status &= ~SIPCON_STATUS_REGISTERED;
   s->regstate = SIPCON_STATE_UNREGISTERED;
}

void sip_register_timer(sip_con_t *s)
{
   int res;

   switch(s->regstate)
   {
      case SIPCON_STATE_REGISTER:         // Start register SIP connection
         // build and send register message
         res = sip_build_register_msg(s, s->buffer);
         voip_send_udp_packet(&s->udpcon, &s->registerAddr.ipaddr, s->registerAddr.port, s->buffer, res);
         TRACE("Send register msg len = %d to %s", res, s->profile->srv_addr);
         voip_timer_restart(&s->reg_rcvtimer);
         s->regstate = SIPCON_STATE_REGISTERING;
         break;

      case SIPCON_STATE_REGISTERING:      // registering
         if (voip_timer_expired(&s->reg_rcvtimer))
         {
            s->errRegCount++;
            s->reg_sndTryCnt--;

            if (!s->reg_sndTryCnt)
            {
               close_register(s);
               sip_error(s, 0, SIP_ERR_REGISTER_RCVTIMEOUT);
            }
            else
            {
               TRACE_ERROR("SIP register response timeout, try it again, tryCnt = %d", s->reg_sndTryCnt);
               s->regstate = SIPCON_STATE_REGISTER;
            }
         }
         break;

      case SIPCON_STATE_REGISTERED:

         // when is time for reregistation
         if (voip_timer_expired(&s->regtimer))
         {
            // clear current authorization
            memset(&s->auth, 0, sizeof(sip_auth_t));

            // reregister SIP connection
            s->reg_sndTryCnt = SIP_SEND_TRYCNT;
            s->regstate = SIPCON_STATE_REGISTER;
            TRACE("Reregister SIP connection");
         }
         break;

      case SIPCON_STATE_UNREGISTER:             // Unregistering
         // build and send unregister message
         res = sip_build_unregister_msg(s, s->buffer);
         voip_send_udp_packet(&s->udpcon, &s->registerAddr.ipaddr, s->registerAddr.port, s->buffer, res);
         TRACE("Send unregister msg len = %d to %s", res, s->profile->srv_addr);
         voip_timer_restart(&s->reg_rcvtimer);
         s->regstate = SIPCON_STATE_UNREGISTERING;
         break;

      case SIPCON_STATE_UNREGISTERING:       // Unregistering
         if (voip_timer_expired(&s->reg_rcvtimer))
         {
            s->errRegCount++;
            s->reg_sndTryCnt--;

            if (!s->reg_sndTryCnt)
            {
               close_register(s);
               sip_error(s, 0, SIP_ERR_REGISTER_RCVTIMEOUT);
            }
            else
            {
               TRACE_ERROR("SIP unregister response timeout, try it again, tryCnt=%d", s->reg_sndTryCnt);
               s->regstate = SIPCON_STATE_UNREGISTER;
            }
         }
         break;

      default:
        break;
   }
}

/** Incomming register response handler */
void sip_register_response_handler(sip_con_t *s, voip_ipaddr_t *addr, uint16_t port)
{
   // check reg callID
   if (strcmp(s->regCallID, s->msg.callId))
      return;

   TRACE("rcv SIP register response code = %d", s->msg.messageCode);
   switch(s->msg.messageCode)
   {
      case 100:         // Trying
         voip_timer_restart(&s->reg_rcvtimer);
         break;

      case 200:         // Success registered
         if (s->regstate == SIPCON_STATE_REGISTERING)
         {
            // if expiration of registration does't set
            if (!s->expires)
               s->expires = s->msg.expires == 0 ? SIP_EXPIRES : s->msg.expires;

            // restart next register timer
            voip_timer_set(&s->regtimer, s->expires * 1000);
            TRACE("SIP connection registered OK, expires after: %d sec, (%ld ms)", s->expires, s->regtimer.interval);

            s->regCount++;
            s->lastRegTime = voip_clock_time();
            s->status |= SIPCON_STATUS_REGISTERED;
            s->regstate = SIPCON_STATE_REGISTERED;

            if (s->events->on_register != NULL)
               s->events->on_register(s, s->profile);
         }
         else if (s->regstate == SIPCON_STATE_UNREGISTERING)
         {
            TRACE("SIP connection unregistered OK");
            s->status &= ~SIPCON_STATUS_REGISTERED;
            s->regstate = SIPCON_STATE_UNREGISTERED;

            if (s->events->on_unregister != NULL)
               s->events->on_unregister(s, s->profile);
         }

      break;

      case 401:      // Unauthorized
      case 407:

            s->reg_sndTryCnt--;

            if (!s->reg_sndTryCnt)
            {
               close_register(s);
               sip_error(s, 0, SIP_ERR_REGISTER_FORBIDDEN_ACCESS);
            }
            else
            {
                // create authorization digest
                sip_auth_digest(
                    s->profile->username,
                    s->profile->passwd,
                    s->msg.method,
                    SIPDOMAIN(s->profile),
                  &s->msg.auth);

                // set authorization for new registration
                memcpy(&s->auth, &s->msg.auth, sizeof(sip_auth_t));

                // reply send authorized message
                if (s->regstate == SIPCON_STATE_REGISTERING)
                {
                    s->regstate = SIPCON_STATE_REGISTER;
                }
                else if (s->regstate == SIPCON_STATE_UNREGISTERING)
                {
                    s->regstate = SIPCON_STATE_UNREGISTER;
                }
            }
      break;

      case 403:	// forbidden access
         TRACE_ERROR("register forbiden access");
         close_register(s);
         sip_error(s, s->msg.messageCode, SIP_ERR_REGISTER_FORBIDDEN_ACCESS);
			break;

      default:
         if (s->msg.messageCode >= 400 && s->msg.messageCode <= 600)
         {
            // some error
            s->errRegCount++;
            close_register(s);
            sip_error(s, s->msg.messageCode, SIP_ERR_REGISTER);
         }
   }
}
