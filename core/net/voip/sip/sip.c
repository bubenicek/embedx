/**
 * \file sip.c				\brief Simple SIP protocol imlementation
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sip.h"
#include "sip_register.h"
#include "sip_invite.h"

#define TRACE_TAG "sip"
#if !ENABLE_TRACE_SIP
#undef TRACE
#define TRACE(...)
#endif


// Prototypes:
static void sip_poll_callback(voip_udp_socket_t *socket, void *user_param);
static void sip_recv_callback(voip_udp_socket_t *socket, voip_ipaddr_t *ipaddr, uint16_t port, void *buf, uint16_t buflen, void *user_param);


/** Create new SIP connection - open UDP socket */
int sip_create_con(
    sip_con_t *sc,
    const char *srvaddr,
    uint16_t remotePort,
    uint16_t localPort,
    const sip_sdp_media_t *sdpMedia,
    const sip_con_events_t *e,
    void *userData)
{
   memset(sc, 0, sizeof(sip_con_t));

   // Initialize random generator
   srand(voip_clock_time());

   // Initialize register server port
   sc->registerAddr.port = remotePort;

   // Resolve register IP address
   if (voip_dns_resolve_ipaddr(srvaddr, &sc->registerAddr.ipaddr) != 0)
   {
      TRACE_ERROR("Resolve sip server hostname: %s", srvaddr);
      return -1;
   }

   // Initialize local port
   sc->localAddr.port = localPort;

   // Intialize sip connection
   sc->sdpMedia = sdpMedia;
   sc->events = e;
   sc->userData = userData;
   sc->status = SIPCON_STATUS_NONE;
   sc->regstate = SIPCON_STATE_UNREGISTERED;
   sc->invstate = SIPCON_STATE_NOINVITED;
   sc->tag = rand();

   // Set start RTP media port
   sc->sdpMediaLocalPort = sc->sdpMedia->audio_port;

   voip_timer_set(&sc->reg_rcvtimer, SIP_RCV_TMO);
   voip_timer_set(&sc->inv_rcvtimer, SIP_RCV_TMO);

   // Create UDP connection
   if (voip_create_udp_socket(&sc->udpcon, &sc->localAddr.ipaddr, sc->localAddr.port, sip_recv_callback, sip_poll_callback, SIP_POLL_INTERVAL, sc) != 0)
   {
      TRACE_ERROR("Create UDP socket");
      return -1;
   }

   TRACE("SIP conn created, srv_addr = %s, TAG=%6x", voip_ipaddr_ntoa(&sc->registerAddr.ipaddr), sc->tag);

   return 0;
}

/** Create new SIP direct connection */
int sip_create_direct_con(
    sip_con_t *sc,
    const sip_profile_t *profile,
    uint16_t remotePort,
    uint16_t localPort,
    const sip_sdp_media_t *sdpMedia,
    const sip_con_events_t *e,
    void *userData)
{
   if (sip_create_con(sc, profile->srv_addr, remotePort, localPort, sdpMedia, e, userData) != 0)
   {
      return -1;
   }

   sc->profile = profile;
   sc->directCall = 1;

   return 0;
}

/** destroy SIP connection - close UDP socket */
int sip_destroy_con(sip_con_t *sc)
{
   voip_close_udp_socket(&sc->udpcon);

   sc->regstate = SIPCON_STATE_UNREGISTERED;
   sc->status = 0;

   TRACE("SIP conn destroyed");

   return 0;
}

/** Register SIP connection */
int sip_register(sip_con_t *sc, const sip_profile_t *prof, uint16_t reg_expires)
{
    // when is not already registered
    if (sip_con_registered(sc))
    {
        TRACE_ERROR("SIP is already registered");
        return -1;
    }

    sc->expires = reg_expires;

    // generate unique register call ID
    sprintf(sc->regCallID, "%x", rand());

    // set register profile
    sc->profile = prof;

    // set number of register trying
    sc->reg_sndTryCnt = SIP_SEND_TRYCNT;

    // clear current authorization
    memset(&sc->auth, 0, sizeof(sip_auth_t));

    // start registration
    sc->regstate = SIPCON_STATE_REGISTER;

    return 0;
}

/** Unregister SIP connection */
int sip_unregister(sip_con_t *sc)
{
    // when is connection registered then unregister it
    if (!sip_con_registered(sc))
        return -1;

    // set number of register trying
    sc->reg_sndTryCnt = SIP_SEND_TRYCNT;
    sc->expires = 0;
    sc->regstate = SIPCON_STATE_UNREGISTER;

    return 0;
}

/** Invite call */
int sip_invite(sip_con_t *sc, const char *remoteUri)
{
    if (sip_con_invited(sc))
    {
        TRACE_ERROR("SIP connection is already invited");
        return -1;
    }

    // set invite address from register address
    voip_ipaddr_set(&sc->inviteAddr.ipaddr, &sc->registerAddr.ipaddr);
    sc->inviteAddr.port = sc->registerAddr.port;

    // set remote call uri
    sc->invRemoteUri = remoteUri;

    // set default SDP media
    memcpy(&sc->invSdpMedia, sc->sdpMedia, sizeof(sip_sdp_media_t));

    // generate uniqueue call ID
    sprintf(sc->invCallID, "%x", rand());

    // set number of invite trying
    sc->inv_sndTryCnt = SIP_SEND_TRYCNT;

    // set inivite state
    sc->invstate = SIPCON_STATE_INVITE;

    // uniqueue invite tag
    sc->invTag = rand();

    // we don't known contact
    *sc->invContact = 0;

    // without authorization
    sc->auth.type = AUTH_TYPE_NOAUTH;

    TRACE("sip invite started, uri = '%s'", remoteUri);

    return 0;
}

/** Hangup incomming/invited call */
int sip_hangup(sip_con_t *sc)
{
    int retval = 0;

    TRACE("%s", __FUNCTION__);

    switch(sc->invstate)
    {
        // when is invite connection established then hangup (send BYE message)
        case SIPCON_STATE_INVITED:
        {
            sc->inv_sndTryCnt = SIP_SEND_TRYCNT;
            sc->invstate = SIPCON_STATE_HANGUP;
            sc->auth.type = AUTH_TYPE_NOAUTH;
            TRACE("sip hangup started");
        }
        break;

        // decline invite, send 480 Temporarily unavailable
        case SIPCON_STATE_INCOMMING_INVITE:
        {
            sc->invstate = SIPCON_STATE_INCOMMING_DECLINE;
            TRACE("sip decline incomming invite");
        }
        break;

        case SIPCON_STATE_INVITE:
        case SIPCON_STATE_INVITENING:
        case SIPCON_STATE_INVPENDING:
        case SIPCON_STATE_RINGING:
        {
            // if connection is inviting then send message CANCEL
            sc->inv_sndTryCnt = SIP_SEND_TRYCNT;
            sc->invstate = SIPCON_STATE_DECLINE;
            TRACE("sip decline outgoing invite");
        }
        break;

        case SIPCON_STATE_NOINVITED:
        case SIPCON_STATE_HANGUP:
        case SIPCON_STATE_HANGUPING:
            break;

        default:
            TRACE_ERROR("%s: unknown sip state = %d", __FUNCTION__, sc->invstate);
            retval = -1;
    }

    return retval;
}

int sip_accept_invite(sip_con_t *sc)
{
    if (sc->invstate == SIPCON_STATE_INCOMMING_INVITE)
    {
        TRACE("Accept incomming invite");
        sc->inv_sndTryCnt = SIP_SEND_TRYCNT;
        sc->invstate = SIPCON_STATE_INCOMMING_ACCEPT;

        return 0;
    }
    else
    {
        TRACE_ERROR("%s not incomming, state = %d", __FUNCTION__, sc->invstate);
        return -1;
    }
}


static void sip_poll_callback(voip_udp_socket_t *socket, void *user_param)
{
   sip_con_t *sc = user_param;

   sip_register_timer(sc);
   sip_invite_timer(sc);
}

/** event invoked when is receive UDP packet */
static void sip_recv_callback(voip_udp_socket_t *socket, voip_ipaddr_t *ipaddr, uint16_t port, void *buf, uint16_t buflen, void *user_param)
{
   sip_con_t *sc = user_param;

   if (buflen > SIP_BUFFER_SIZE)
   {
      TRACE_ERROR("SIP buffer overflow");
      buflen = SIP_BUFFER_SIZE;
   }

   memcpy(sc->buffer, buf, buflen);

   // Terminate received string
   sc->buffer[buflen-1] = '\0';

#if defined(ENABLE_TRACE_SIP_RXTX) && (ENABLE_TRACE_SIP_RXTX == 1)
    TRACE("*** Recv %d bytes from: %s ***\n\n%s", buflen, voip_ipaddr_ntoa(ipaddr), sc->buffer);
#endif

   // Parse SIP message
   sip_parse_msg(sc, sc->buffer, &sc->msg);

   switch(sc->msg.messageType)
   {
      case SIPMSG_REQUEST:
      {
         sip_invite_request_handler(sc, ipaddr, port);
      }
      break;

      case SIPMSG_RESPONSE:
      {
         if (sc->msg.methodType == SIP_METHOD_REGISTER)
         {
            sip_register_response_handler(sc, ipaddr, port);
         }
         else
         {
            sip_invite_response_handler(sc, ipaddr, port);
         }
      }
      break;

      default:
         TRACE_ERROR("rcv uknown SIP msg type = %d", sc->msg.messageType);
    }
}
