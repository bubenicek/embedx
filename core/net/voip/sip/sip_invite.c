/**
 * \file sip_invite.c        \brief SIP invite dialog processor
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sip.h"

#define TRACE_TAG "sip-invite"
#if !ENABLE_TRACE_SIP_INVITE
#undef TRACE
#define TRACE(...)
#endif


// Prototypes:
static void close_invite(sip_con_t *s);


void sip_invite_timer(sip_con_t *s)
{
   int res;

   switch(s->invstate)
   {
      case SIPCON_STATE_INVITE:              // start invite
         // build and send invite message
         res = sip_build_invite_msg(s, s->buffer, s->invRemoteUri);
         voip_send_udp_packet(&s->udpcon, &s->inviteAddr.ipaddr, s->inviteAddr.port, s->buffer, res);
         TRACE("Send invite msg len = %d to %s", res, s->invRemoteUri);
         voip_timer_restart(&s->inv_rcvtimer);
         s->invstate = SIPCON_STATE_INVITENING;
         break;

      case SIPCON_STATE_INVITENING:          // invite in progress
         if (voip_timer_expired(&s->inv_rcvtimer))
         {
            s->errInvCount++;
            s->inv_sndTryCnt--;

            if (!s->inv_sndTryCnt)
            {
               sip_error(s, 0, SIP_ERR_INVITE_RCVTIMEOUT);
               close_invite(s);
            }
            else
            {
               TRACE_ERROR("SIP invite response timeout, try it again, tryCnt = %d", s->inv_sndTryCnt);
               s->invstate = SIPCON_STATE_INVITE;
            }
         }
         break;

      case SIPCON_STATE_DECLINE:          // decline outgoing invite
         res = sip_build_cancel_msg(s, s->buffer, s->invRemoteUri);
         voip_send_udp_packet(&s->udpcon, &s->inviteAddr.ipaddr, s->inviteAddr.port, s->buffer, res);
         TRACE("Send cancel msg len = %d", res);
         voip_timer_restart(&s->inv_rcvtimer);
         s->invstate = SIPCON_STATE_DECLINING;
         break;

      case SIPCON_STATE_DECLINING:
         if (voip_timer_expired(&s->inv_rcvtimer))
         {
            s->errInvCount++;
            s->inv_sndTryCnt--;

            if (!s->inv_sndTryCnt)
            {
               sip_error(s, 0, SIP_ERR_INVITE_RCVTIMEOUT);
               close_invite(s);
            }
            else
            {
               TRACE_ERROR("SIP cancel response timeout, close invite");
               close_invite(s);
            }
         }
         break;

      case SIPCON_STATE_HANGUP:              // hangup connection
      {
         res = sip_build_bye_msg(s, s->buffer, s->invRemoteUri);
         voip_send_udp_packet(&s->udpcon, &s->inviteAddr.ipaddr, s->inviteAddr.port, s->buffer, res);
         TRACE("Send bye msg len = %d to %s", res, s->invRemoteUri);
         voip_timer_restart(&s->inv_rcvtimer);
         s->invstate = SIPCON_STATE_HANGUPING;
      }
      break;

      case SIPCON_STATE_HANGUPING:          // hangup in progress
         if (voip_timer_expired(&s->inv_rcvtimer))
         {
            s->errInvCount++;
            s->inv_sndTryCnt--;

            if (!s->inv_sndTryCnt)
            {
               sip_error(s, 0, SIP_ERR_INVITE_RCVTIMEOUT);
               close_invite(s);
            }
            else
            {
               TRACE_ERROR("SIP bye response timeout, try it again, tryCnt = %d", s->inv_sndTryCnt);
               s->invstate = SIPCON_STATE_HANGUP;
            }
         }
         break;

      case SIPCON_STATE_INCOMMING_ACCEPT:
         // send 200 OK with SDP
         res = sip_build_ok_sdp_msg(s, s->buffer, &s->msg);
         voip_send_udp_packet(&s->udpcon, &s->inviteAddr.ipaddr, s->inviteAddr.port, s->buffer, res);
         TRACE("Send 200 OK with SDP len = %d", res);
         voip_timer_restart(&s->inv_rcvtimer);
         s->invstate = SIPCON_STATE_INCOMMING_ACCEPTING;
         break;

      case SIPCON_STATE_INCOMMING_ACCEPTING:
         if (voip_timer_expired(&s->inv_rcvtimer))
         {
            s->errInvCount++;
            s->inv_sndTryCnt--;

            if (!s->inv_sndTryCnt)
            {
               sip_error(s, 0, SIP_ERR_INVITE_RCVTIMEOUT);
               close_invite(s);
            }
            else
            {
               TRACE_ERROR("SIP ACK response timeout, try it again, trycnt = %d", s->inv_sndTryCnt);
               s->invstate = SIPCON_STATE_INCOMMING_ACCEPT;
            }
         }
         break;

       case SIPCON_STATE_INCOMMING_DECLINE:
       {
            res = sip_build_decline_msg(s, s->buffer, &s->msg);
            voip_send_udp_packet(&s->udpcon, &s->inviteAddr.ipaddr, s->inviteAddr.port, s->buffer, res);
            TRACE("Send sip msg 603 Decline len = %d", res);

            TRACE("== DECLINE/CANCEL connection OK");
            close_invite(s);
            if (s->events->on_hangup != NULL)
                s->events->on_hangup(s, s->invRemoteUri);
       }
       break;

      default:
         break;
   }
}

void sip_invite_request_handler(sip_con_t *s, voip_ipaddr_t *addr, uint16_t port)
{
   int res;

   if (s->msg.methodType != SIP_METHOD_OPTIONS &&
       s->msg.methodType != SIP_METHOD_INVITE)
   {
      // check if invite call ID is the same
      if (strcmp(s->invCallID, s->msg.callId))
         return;
   }

   TRACE("INVITE handle request type = %d", s->msg.methodType);
   switch(s->msg.methodType)
   {
      case SIP_METHOD_OPTIONS:
         res = sip_build_ok_sdp_msg(s, s->buffer, &s->msg);
         voip_send_udp_packet(&s->udpcon, addr, port, s->buffer, res);
         TRACE("Send sip OK len = %d", res);
         break;

      case SIP_METHOD_BYE:                    // hangup connection
      case SIP_METHOD_CANCEL:
      {
         // send OK message
         res = sip_build_ok_msg(s, s->buffer, &s->msg);
         voip_send_udp_packet(&s->udpcon, addr, port, s->buffer, res);
         TRACE("Send sip OK len = %d", res);
         if (sip_con_invited(s))
         {
            close_invite(s);
            if (s->events->on_hangup != NULL)
               s->events->on_hangup(s, s->invRemoteUri);
         }
         else if (s->invstate == SIPCON_STATE_INCOMMING_INVITE)
         {
            // cancel incomming invite
            close_invite(s);
            if (s->events->on_invite_declined != NULL)
               s->events->on_invite_declined(s, s->invRemoteUri);
         }
      }
      break;

      case SIP_METHOD_INVITE:
         if (sip_con_invited(s))
         {
            if (!strcmp(s->invCallID, s->msg.callId))
            {
                // The same invite call ID - ReINVITE

                memcpy(&s->invRecordRoute, &s->msg.record_route, sizeof(s->invRecordRoute));
                strlcpy(s->invFrom, s->msg.from, sizeof(s->invFrom));

                // Copy media info from invite message
                if (s->msg.has_sdp_content)
                    memcpy(&s->invSdpMedia, &s->msg.sdpmedia, sizeof(sip_sdp_media_t));

                s->invRemoteUri = s->msg.reqUri;
                strlcpy(s->invCallID, s->msg.callId, sizeof(s->invCallID));
                strlcpy(s->invContact, s->msg.contact, sizeof(s->invContact));

                // Set incomming invite addr
                voip_ipaddr_set(&s->inviteAddr.ipaddr, addr);
                s->inviteAddr.port = port;

                // RB. disabled - Increment SDP media local port
                //++s->sdpMediaLocalPort;

                res = sip_build_ok_sdp_msg(s, s->buffer, &s->msg);
                voip_send_udp_packet(&s->udpcon, addr, port, s->buffer, res);
                TRACE("Send 200 OK with SDP len = %d", res);
            }
            else
            {
               // already exist another invited connector
               // decline invite, send 480 Temporarily unavailable
               res = sip_build_unavailable_msg(s, s->buffer, &s->msg);
               voip_send_udp_packet(&s->udpcon, addr, port, s->buffer, res);
               TRACE("Send sip msg 480 Unavailable len = %d", res);
            }
         }
         else if (s->invstate == SIPCON_STATE_NOINVITED)
         {
            memcpy(&s->invRecordRoute, &s->msg.record_route, sizeof(s->invRecordRoute));
            strlcpy(s->invFrom, s->msg.from, sizeof(s->invFrom));
            s->invTag = rand();

            // Copy media info from invite message
            if (s->msg.has_sdp_content)
                memcpy(&s->invSdpMedia, &s->msg.sdpmedia, sizeof(sip_sdp_media_t));

            s->invRemoteUri = s->msg.reqUri;
            strlcpy(s->invCallID, s->msg.callId, sizeof(s->invCallID));
            strlcpy(s->invContact, s->msg.contact, sizeof(s->invContact));

            // set incomming invite addr
            voip_ipaddr_set(&s->inviteAddr.ipaddr, addr);
            s->inviteAddr.port = port;

            // send ringing message 180
            res = sip_build_ringing_msg(s, s->buffer, &s->msg);
            voip_send_udp_packet(&s->udpcon, addr, port, s->buffer, res);
            TRACE("Send sip msg 180 Ringing len = %d", res);

            s->invstate = SIPCON_STATE_INCOMMING_INVITE;
            if (s->events->on_incomming_invite_ringing != NULL)
               s->events->on_incomming_invite_ringing(s, s->msg.reqUri);
         }
         else if (s->invstate == SIPCON_STATE_INCOMMING_INVITE)
         {
            // send ringing message 180
            res = sip_build_ringing_msg(s, s->buffer, &s->msg);
            voip_send_udp_packet(&s->udpcon, addr, port, s->buffer, res);
            TRACE("Send sip msg 180 Ringing len = %d", res);
         }
         break;

      case SIP_METHOD_ACK:
         if (s->invstate == SIPCON_STATE_INCOMMING_ACCEPTING ||
             s->invstate == SIPCON_STATE_INVITED)
         {
            // Copy media info from invite message
            if (s->msg.has_sdp_content)
                memcpy(&s->invSdpMedia, &s->msg.sdpmedia, sizeof(sip_sdp_media_t));

            if (s->invstate != SIPCON_STATE_INVITED)
                s->lastInvStartTime = s->lastInvEndTime = voip_clock_time();

            s->status |= SIPCON_STATUS_INVITED;
            s->invstate = SIPCON_STATE_INVITED;

            if (s->events->on_invite != NULL)
            {
               s->events->on_invite(
                     s,
                     s->invRemoteUri,
                     s->invSdpMedia.addr,
                     s->invSdpMedia.audio_port,
                     s->sdpMediaLocalPort,
                     &s->invSdpMedia);
            }
         }
         break;

        default:
            break;
   }
}

void sip_invite_response_handler(sip_con_t *s, voip_ipaddr_t *addr, uint16_t port)
{
   // check if invite call ID is the same
   if (strcmp(s->invCallID, s->msg.callId))
      return;

   TRACE("Rcv SIP invite response code = %d", s->msg.messageCode);

   if (s->msg.messageCode >= 200)
   {
      int res;

      if (s->invstate != SIPCON_STATE_HANGUPING)
      {
         // send ACK message
         res = sip_build_ack_msg(s, s->buffer, &s->msg, s->invRemoteUri);
         voip_send_udp_packet(&s->udpcon, addr, port, s->buffer, res);
         TRACE("Send sip ACK len = %d   cseq=%d", res, s->msg.cseq);
      }
   }

   switch(s->msg.messageCode)
   {
      case 100:         // Trying
      case 183:         // Progress
         voip_timer_restart(&s->inv_rcvtimer);
         s->invstate = SIPCON_STATE_INVPENDING;
         if (s->events->on_invite_trying != NULL)
            s->events->on_invite_trying(s, s->invRemoteUri);
         break;

      case 180:         // ringing
         s->invstate = SIPCON_STATE_RINGING;
         if (s->events->on_invite_ringing != NULL)
            s->events->on_invite_ringing(s, s->invRemoteUri);
         break;

      case 200:         // Success
      {
         TRACE("SIP 200: state = %d", s->invstate);
         if (s->invstate == SIPCON_STATE_HANGUPING)
         {
            TRACE("== HANGUP connection OK ==");
            close_invite(s);
            if (s->events->on_hangup != NULL)
               s->events->on_hangup(s, s->invRemoteUri);
         }
         else if (s->invstate == SIPCON_STATE_INVITENING ||
                  s->invstate == SIPCON_STATE_RINGING ||
                  s->invstate == SIPCON_STATE_INVPENDING)
         {
            TRACE("== INVITE connection OK ==");

            memcpy(&s->invRecordRoute, &s->msg.record_route, sizeof(s->invRecordRoute));
            strlcpy(s->invFrom, s->msg.to, sizeof(s->invFrom));

            s->lastInvStartTime = s->lastInvEndTime = voip_clock_time();
            memcpy(&s->invSdpMedia, &s->msg.sdpmedia, sizeof(sip_sdp_media_t));
            strlcpy(s->invContact, s->msg.contact, sizeof(s->invContact));

            s->status |= SIPCON_STATUS_INVITED;
            s->invstate = SIPCON_STATE_INVITED;

            if (s->events->on_invite != NULL)
            {
               s->events->on_invite(
                     s,
                     s->invRemoteUri,
                     s->invSdpMedia.addr,
                     s->invSdpMedia.audio_port,
                     s->sdpMediaLocalPort,
                     &s->invSdpMedia);
            }
         }
         else if (s->invstate == SIPCON_STATE_DECLINING)
         {
            close_invite(s);

            if (s->events->on_invite_declined != NULL)
               s->events->on_invite_declined(s, s->invRemoteUri);
         }
      }
      break;

      case 401:      // Unauthorized
      case 407:
      {
            s->inv_sndTryCnt--;

            if (!s->inv_sndTryCnt)
            {
               sip_error(s, 0, SIP_ERR_INVITE_FORBIDDEN_ACCESS);
               close_invite(s);
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


                if (s->invstate == SIPCON_STATE_HANGUPING)
                {
                    // reply send BYE message
                    s->invstate = SIPCON_STATE_HANGUP;
                }
                else
                {
                    // reply send invite message
                    s->invstate = SIPCON_STATE_INVITE;
                }
            }
      }
      break;

      case 487:		// request terminated
          TRACE("== DECLINE/CANCEL connection OK");
          close_invite(s);
          if (s->events->on_hangup != NULL)
             s->events->on_hangup(s, s->invRemoteUri);
      break;

      case 491:     // Request Pending
         voip_timer_restart(&s->inv_rcvtimer);
         s->invstate = SIPCON_STATE_INVPENDING;
         break;

      case 403:	// forbidden access
         TRACE_ERROR("SIP: forbiden access");
         close_invite(s);
         sip_error(s, s->msg.messageCode, SIP_ERR_INVITE_FORBIDDEN_ACCESS);
			break;

      case 404:	// user not found
         TRACE_ERROR("SIP: user not found");
         close_invite(s);
         sip_error(s, s->msg.messageCode, SIP_ERR_INVITE_USER_NOTFOUND);
			break;

      case 486:	// Busy Here
      case 502:	// bad gateway, send proxy when host declined invite
      case 603:	// declined
			TRACE("SIP: invite declined");
			close_invite(s);
			if (s->events->on_invite_declined != NULL)
            s->events->on_invite_declined(s, s->invRemoteUri);
			break;

      default:
         if (s->msg.messageCode >= 400 && s->msg.messageCode <= 600)
         {
            TRACE_ERROR("Invite sip error code = %d", s->msg.messageCode);
            close_invite(s);
            sip_error(s, s->msg.messageCode, SIP_ERR_INVITE);
         }
   }
}


static void close_invite(sip_con_t *s)
{
   s->status &= ~SIPCON_STATUS_INVITED;
   s->invstate = SIPCON_STATE_NOINVITED;
   s->lastInvEndTime = voip_clock_time();

   // Set start RTP media port
   s->sdpMediaLocalPort = s->sdpMedia->audio_port;
}
