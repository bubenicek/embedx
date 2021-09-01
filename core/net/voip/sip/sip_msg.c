/**
 * \file sip_msg.h    \brie SIP message builder/parser
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sip.h"
#include "sip_msg.h"

#define TRACE_TAG "sip-invite"
#if !ENABLE_TRACE_SIP
#undef TRACE
#define TRACE(...)
#endif


#define buf_printf(format, ...)\
   do {\
      res = sprintf(buf, format, ## __VA_ARGS__);\
      buflen += res; \
      buf += res; \
   } while(0)


// Prototypes:
static void parse_request_line(char *buf, sip_msg_t *msg);
static void parse_method_type(char *buf, sip_msg_t *msg);
static void parse_via(struct sip_conn_t *s, char *buf, sip_msg_t *msg);
static void parse_record_route(char *buf, sip_msg_t *msg);
static void parse_route(char *buf, sip_msg_t *msg);
static void parse_to(char *buf, sip_msg_t *msg);
static void parse_from(char *buf, sip_msg_t *msg);
static void parse_callId(char *buf, sip_msg_t *msg);
static void parse_cseq(char *buf, sip_msg_t *msg);
static void parse_contact(char *buf, sip_msg_t *msg);
static void parse_authenticate(char *buf, sip_msg_t *msg);
static void parse_expires(char *buf, sip_msg_t *msg);
static void parse_min_expires(char *buf, sip_msg_t *msg);
static void parse_sdp_media_addr(char *buf, sip_msg_t *msg);
static void parse_sdp_media_audio_port(char *buf, sip_msg_t *msg);
static void parse_sdp_media_video_port(char *buf, sip_msg_t *msg);
static void parse_sdp_media_telephone_event(char *buf, sip_msg_t *msg);

static int sip_build_sdp_content(struct sip_conn_t *s, char *buf);



/**
 * build SIP register message
 * @param s SIP connection
 * @param buf output buffer
 * @return if success then return size of message buffer else -1 if any error
 */
int sip_build_register_msg(struct sip_conn_t *s, char *buf)
{
    int res, ix;
    int buflen = 0;

    buf_printf("REGISTER sip:%s SIP/2.0\r\n", SIPDOMAIN(s->profile));
    buf_printf("Via: SIP/2.0/UDP %s:%d;" SIPMSG_BRANCH_STR ";rport\r\n",
               voip_ipaddr_ntoa(&s->localAddr.ipaddr),
               s->localAddr.port,
               s->tag + rand());

    buf_printf("Max-Forwards: %d\r\n", SIP_MAX_FORWARDS);
    buf_printf("Contact: <sip:%s@%s:%d>\r\n",
               s->profile->username,
               voip_ipaddr_ntoa(&s->localAddr.ipaddr),
               s->localAddr.port);
    buf_printf("To: <sip:%s@%s>\r\n", s->profile->username, SIPDOMAIN(s->profile));
    buf_printf("From: <sip:%s@%s>;tag=%x\r\n", s->profile->username, SIPDOMAIN(s->profile), s->tag);

    buf_printf("Call-ID: %s\r\n", s->regCallID);
    buf_printf("CSeq: %d REGISTER\r\n",++s->lastCseq);

    // Expires value must be send when is connection registering
    if (s->expires > 0)
        buf_printf("Expires: %d\r\n", s->expires);

    buf_printf("Allow: INVITE, ACK, OPTIONS, CANCEL, BYE\r\n");
    buf_printf("User-Agent: %s\r\n", SIP_USER_AGENT);

    if (s->auth.type == AUTH_TYPE_WWWAUTH)
        buf_printf("Authorization: ");
    else if (s->auth.type == AUTH_TYPE_PROXYAUTH)
        buf_printf("Proxy-Authorization: ");

    if (s->auth.type != AUTH_TYPE_NOAUTH)
    {
        buf_printf("Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"sip:%s\",response=\"",
                   s->profile->username,
                   s->auth.realm,
                   s->auth.nonce,
                   SIPDOMAIN(s->profile));

        for (ix = 0; ix < 16; ix++)
            buf_printf("%2.2x", s->auth.digest[ix]);

        buf_printf("\"");

        if (*s->auth.opaque != 0)
            buf_printf(",opaque=\"%s\"", s->auth.opaque);

        buf_printf(",algorithm=MD5\r\n");
    }

    buf_printf("Content-Length: 0\r\n");
    buf_printf("\r\n");

    return buflen;
}

/** Build invite message */
int sip_build_invite_msg(struct sip_conn_t *s, char *buf, const char *remoteUri)
{
    int res, ix, sdplen;
    int buflen = 0;
    char sdpbuf[SDPBUF_SIZE];

    if (s->directCall)
        buf_printf("INVITE sip:%s SIP/2.0\r\n", SIPDOMAIN(s->profile));
    else
        buf_printf("INVITE sip:%s@%s SIP/2.0\r\n", remoteUri, SIPDOMAIN(s->profile));

    buf_printf("Via: SIP/2.0/UDP %s:%d;" SIPMSG_BRANCH_STR "\r\n",
               voip_ipaddr_ntoa(&s->localAddr.ipaddr),
               s->localAddr.port,
               s->tag + rand());

    if (s->directCall)
        buf_printf("From: <sip:%s>;tag=%x\r\n", voip_ipaddr_ntoa(&s->localAddr.ipaddr), s->invTag);
    else
        buf_printf("From: <sip:%s@%s>;tag=%x\r\n", s->profile->username, SIPDOMAIN(s->profile), s->invTag);

    if (s->directCall)
        buf_printf("To: <sip:%s>\r\n", SIPDOMAIN(s->profile));
    else
        buf_printf("To: <sip:%s@%s>\r\n", remoteUri, SIPDOMAIN(s->profile));

    if (s->directCall)
        buf_printf("Contact: <sip:%s:%d>\r\n",
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);
    else
        buf_printf("Contact: <sip:%s@%s:%d>\r\n",
                   s->profile->username,
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);

    buf_printf("Call-ID: %s\r\n", s->invCallID);
    buf_printf("CSeq: %d INVITE\r\n",++s->lastCseq);
    buf_printf("Allow: INVITE, ACK, OPTIONS, CANCEL, BYE\r\n");
    buf_printf("Max-Forwards: %d\r\n", SIP_MAX_FORWARDS);
    buf_printf("User-Agent: %s\r\n", SIP_USER_AGENT);

    if (s->auth.type == AUTH_TYPE_WWWAUTH)
        buf_printf("Authorization: ");
    else if (s->auth.type == AUTH_TYPE_PROXYAUTH)
        buf_printf("Proxy-Authorization: ");

    if (s->auth.type != AUTH_TYPE_NOAUTH)
    {
        buf_printf("Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"sip:%s\",response=\"",
                   s->profile->username,
                   s->auth.realm,
                   s->auth.nonce,
                   SIPDOMAIN(s->profile));

        for (ix = 0; ix < 16; ix++)
            buf_printf("%2.2x", s->auth.digest[ix]);

        buf_printf("\"");

        if (*s->auth.opaque != 0)
            buf_printf(",opaque=\"%s\"", s->auth.opaque);

        buf_printf(",algorithm=MD5\r\n");
    }

    sdplen = sip_build_sdp_content(s, sdpbuf);
    buf_printf("Content-Type: application/sdp\r\n");
    buf_printf("Content-Length: %d\r\n", sdplen);
    buf_printf("\r\n");
    strcat(buf, sdpbuf);
    buflen += sdplen;

    return buflen;
}

int sip_build_ack_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg, const char *remoteUri)
{
    int res, ix;
    int buflen = 0;

    if (*msg->contact != 0)
    {
        buf_printf("ACK %s SIP/2.0\r\n", msg->contact);
    }
    else
    {
        if (s->directCall)
            buf_printf("ACK sip:%s SIP/2.0\r\n", SIPDOMAIN(s->profile));
        else
            buf_printf("ACK sip:%s@%s SIP/2.0\r\n", remoteUri, SIPDOMAIN(s->profile));
    }

    for (ix = 0; ix < msg->via.count; ix++)
        buf_printf("Via: %s\r\n", msg->via.values[ix].text);

    for (ix = 0; ix < msg->route.count; ix++)
        buf_printf("Route: %s\r\n", msg->route.values[ix].text);

    // record-route must be send as route
    for (ix = 0; ix < msg->record_route.count; ix++)
        buf_printf("Route: %s\r\n", msg->record_route.values[ix].text);

    //for (ix = 0; ix < msg->record_route.count; ix++)
    //   buf_printf("Record-Route: %s\r\n", msg->record_route.values[ix].text);


    buf_printf("From: %s\r\n", msg->from);
    buf_printf("To: %s\r\n", msg->to);
    buf_printf("Call-ID: %s\r\n", msg->callId);
    buf_printf("CSeq: %d ACK\r\n", msg->cseq);

    if (s->directCall)
        buf_printf("Contact: <sip:%s:%d>\r\n",
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);
    else
        buf_printf("Contact: <sip:%s@%s:%d>\r\n",
                   s->profile->username,
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);

    buf_printf("Content-Length: 0\r\n");
    buf_printf("\r\n");

    return buflen;
}

int sip_build_bye_msg(struct sip_conn_t *s, char *buf, const char *remoteUri)
{
    int res, ix;
    int buflen = 0;

    if (*s->invContact != 0)
        buf_printf("BYE %s SIP/2.0\r\n", s->invContact);
    else
        buf_printf("BYE sip:%s@%s SIP/2.0\r\n", remoteUri, SIPDOMAIN(s->profile));

    buf_printf("Via: SIP/2.0/UDP %s:%d;rport;" SIPMSG_BRANCH_STR "\r\n",
               voip_ipaddr_ntoa(&s->localAddr.ipaddr),
               s->localAddr.port,
               s->tag + rand());

    // record-route must be send as route
    for (ix = 0; ix < s->invRecordRoute.count; ix++)
        buf_printf("Route: %s\r\n", s->invRecordRoute.values[ix].text);

    buf_printf("From: <sip:%s@%s>;tag=%x\r\n", s->profile->username, SIPDOMAIN(s->profile), s->invTag);
    buf_printf("To: %s\r\n", s->invFrom);

    if (s->directCall)
        buf_printf("Contact: <sip:%s:%d>\r\n",
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);
    else
        buf_printf("Contact: <sip:%s@%s:%d>\r\n",
                   s->profile->username,
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);

    buf_printf("Call-ID: %s\r\n", s->invCallID);
    buf_printf("CSeq: %d BYE\r\n", ++s->lastCseq);
    buf_printf("Max-Forwards: %d\r\n", SIP_MAX_FORWARDS);

    if (s->auth.type == AUTH_TYPE_WWWAUTH)
        buf_printf("Authorization: ");
    else if (s->auth.type == AUTH_TYPE_PROXYAUTH)
        buf_printf("Proxy-Authorization: ");

    if (s->auth.type != AUTH_TYPE_NOAUTH)
    {
        buf_printf("Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"sip:%s\",response=\"",
                   s->profile->username,
                   s->auth.realm,
                   s->auth.nonce,
                   SIPDOMAIN(s->profile));

        for (ix = 0; ix < 16; ix++)
            buf_printf("%2.2x", s->auth.digest[ix]);

        buf_printf("\"");

        if (*s->auth.opaque != 0)
            buf_printf(",opaque=\"%s\"", s->auth.opaque);

        buf_printf(",algorithm=MD5\r\n");
    }

    buf_printf("Content-Length: 0\r\n");
    buf_printf("\r\n");

    return buflen;
}

int sip_build_cancel_msg(struct sip_conn_t *s, char *buf, const char *remoteUri)
{
    int res, ix;
    int buflen = 0;

    if (*s->invContact != 0)
        buf_printf("CANCEL %s SIP/2.0\r\n", s->invContact);
    else
        buf_printf("CANCEL sip:%s@%s SIP/2.0\r\n", remoteUri, SIPDOMAIN(s->profile));

    buf_printf("Via: SIP/2.0/UDP %s:%d;" SIPMSG_BRANCH_STR "\r\n",
               voip_ipaddr_ntoa(&s->localAddr.ipaddr),
               s->localAddr.port,
               s->tag + rand());

    // record-route must be send as route
    for (ix = 0; ix < s->invRecordRoute.count; ix++)
        buf_printf("Route: %s\r\n", s->invRecordRoute.values[ix].text);

//    buf_printf("From: <sip:%s@%s>;tag=%x\r\n", s->profile->username, SIPDOMAIN(s->profile), s->invTag);
//    buf_printf("To: <sip:%s@%s>\r\n", remoteUri, SIPDOMAIN(s->profile));

    buf_printf("From: <sip:%s@%s>;tag=%x\r\n", s->profile->username, SIPDOMAIN(s->profile), s->invTag);
    buf_printf("To: %s\r\n", s->invFrom);

    if (s->directCall)
        buf_printf("Contact: <sip:%s:%d>\r\n",
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);
    else
        buf_printf("Contact: <sip:%s@%s:%d>\r\n",
                   s->profile->username,
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);

    buf_printf("Call-ID: %s\r\n", s->invCallID);
    buf_printf("CSeq: %d CANCEL\r\n", s->lastCseq);
    buf_printf("Max-Forwards: %d\r\n", SIP_MAX_FORWARDS);
    buf_printf("Content-Length: 0\r\n");
    buf_printf("\r\n");

    return buflen;
}

/** buil 200 OK message from request message */
int sip_build_ok_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg)
{
    int res, ix;
    int buflen = 0;

    buf_printf("SIP/2.0 200 OK\r\n");

    for (ix = 0; ix < msg->route.count; ix++)
        buf_printf("Route: %s\r\n", msg->route.values[ix].text);

    for (ix = 0; ix < msg->record_route.count; ix++)
        buf_printf("Record-Route: %s\r\n", msg->record_route.values[ix].text);

    for (ix = 0; ix < msg->via.count; ix++)
        buf_printf("Via: %s\r\n", msg->via.values[ix].text);

    buf_printf("From: %s\r\n", msg->from);
    buf_printf("To: %s\r\n", msg->to);
    buf_printf("Call-ID: %s\r\n", msg->callId);
    buf_printf("CSeq: %d %s\r\n", msg->cseq, msg->method);

    if (s->directCall)
        buf_printf("Contact: <sip:%s:%d>\r\n",
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);
    else
        buf_printf("Contact: <sip:%s@%s:%d>\r\n",
                   s->profile->username,
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);

    buf_printf("Content-Length: 0\r\n");
    buf_printf("\r\n");

    return buflen;
}

/** build 200 OK with SDP info */
int sip_build_ok_sdp_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg)
{
    int res, sdplen, ix;
    int buflen = 0;
    char sdpbuf[SDPBUF_SIZE];

    buf_printf("SIP/2.0 200 OK\r\n");

    for (ix = 0; ix < msg->via.count; ix++)
        buf_printf("Via: %s\r\n", msg->via.values[ix].text);

    for (ix = 0; ix < msg->route.count; ix++)
        buf_printf("Route: %s\r\n", msg->route.values[ix].text);

    for (ix = 0; ix < msg->record_route.count; ix++)
        buf_printf("Record-Route: %s\r\n", msg->record_route.values[ix].text);

    buf_printf("From: %s\r\n", msg->from);

    // Add invite tag when tag does not exist
    if (strstr(msg->to, ";tag=") == NULL)
        buf_printf("To: %s;tag=%x\r\n", msg->to, s->invTag);
    else
        buf_printf("To: %s\r\n", msg->to);

    buf_printf("Call-ID: %s\r\n", msg->callId);
    buf_printf("CSeq: %d %s\r\n", msg->cseq, msg->method);
    buf_printf("Allow: INVITE, ACK, CANCEL, OPTIONS, BYE\r\n");
//   buf_printf("Accept: application/sdp, text/plain\r\n");

    if (s->directCall)
        buf_printf("Contact: <sip:%s:%d>\r\n",
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);
    else
        buf_printf("Contact: <sip:%s@%s:%d>\r\n",
                   s->profile->username,
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);

    sdplen = sip_build_sdp_content(s, sdpbuf);
    buf_printf("Content-Type: application/sdp\r\n");
    buf_printf("Content-Length: %d\r\n", sdplen);
    buf_printf("\r\n");
    strcat(buf, sdpbuf);
    buflen += sdplen;

    return buflen;
}


/** buil 480 Temporarily unavailable message from request message */
int sip_build_unavailable_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg)
{
    int res, ix;
    int buflen = 0;

    buf_printf("SIP/2.0 480 Temporarily Unavailable\r\n");

    for (ix = 0; ix < msg->route.count; ix++)
        buf_printf("Route: %s\r\n", msg->route.values[ix].text);

    for (ix = 0; ix < msg->record_route.count; ix++)
        buf_printf("Record-Route: %s\r\n", msg->record_route.values[ix].text);

    for (ix = 0; ix < msg->via.count; ix++)
        buf_printf("Via: %s\r\n", msg->via.values[ix].text);

    buf_printf("From: %s\r\n", msg->from);
    buf_printf("To: %s\r\n", msg->to);
    buf_printf("Call-ID: %s\r\n", msg->callId);
    buf_printf("CSeq: %d %s\r\n", msg->cseq, msg->method);

    if (s->directCall)
        buf_printf("Contact: <sip:%s:%d>\r\n",
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);
    else
        buf_printf("Contact: <sip:%s@%s:%d>\r\n",
                   s->profile->username,
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);

    buf_printf("Content-Length: 0\r\n");
    buf_printf("\r\n");

    return buflen;
}

int sip_build_decline_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg)
{
    int res, ix;
    int buflen = 0;

    buf_printf("SIP/2.0 603 Decline\r\n");

    for (ix = 0; ix < msg->route.count; ix++)
        buf_printf("Route: %s\r\n", msg->route.values[ix].text);

    for (ix = 0; ix < msg->record_route.count; ix++)
        buf_printf("Record-Route: %s\r\n", msg->record_route.values[ix].text);

    for (ix = 0; ix < msg->via.count; ix++)
        buf_printf("Via: %s\r\n", msg->via.values[ix].text);

    buf_printf("From: %s\r\n", msg->from);
    buf_printf("To: %s\r\n", msg->to);
    buf_printf("Call-ID: %s\r\n", msg->callId);
    buf_printf("CSeq: %d %s\r\n", msg->cseq, msg->method);

    if (s->directCall)
        buf_printf("Contact: <sip:%s:%d>\r\n",
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);
    else
        buf_printf("Contact: <sip:%s@%s:%d>\r\n",
                   s->profile->username,
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);

    buf_printf("Content-Length: 0\r\n");
    buf_printf("\r\n");

    return buflen;
}


/** build 180 ringing message */
int sip_build_ringing_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg)
{
    int res, ix;
    int buflen = 0;

    buf_printf("SIP/2.0 180 Ringing\r\n");

    for (ix = 0; ix < msg->via.count; ix++)
        buf_printf("Via: %s\r\n", msg->via.values[ix].text);

    for (ix = 0; ix < msg->route.count; ix++)
        buf_printf("Route: %s\r\n", msg->route.values[ix].text);

    for (ix = 0; ix < msg->record_route.count; ix++)
        buf_printf("Record-Route: %s\r\n", msg->record_route.values[ix].text);

    buf_printf("From: %s\r\n", msg->from);
    buf_printf("To: %s\r\n", msg->to);
    buf_printf("Call-ID: %s\r\n", msg->callId);
    buf_printf("CSeq: %d %s\r\n", msg->cseq, msg->method);

    if (s->directCall)
        buf_printf("Contact: <sip:%s:%d>\r\n",
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);
    else
        buf_printf("Contact: <sip:%s@%s:%d>\r\n",
                   s->profile->username,
                   voip_ipaddr_ntoa(&s->localAddr.ipaddr),
                   s->localAddr.port);

    buf_printf("Allow: INVITE, ACK, OPTIONS, CANCEL, BYE\r\n");

    buf_printf("Content-Length: 0\r\n");
    buf_printf("\r\n");

    return buflen;
}

static int sip_build_sdp_content(struct sip_conn_t *s, char *buf)
{
    int res, buflen = 0;
    int origin_time = voip_clock_time() - s->lastInvStartTime;

    buf_printf("v=0\r\n");
    buf_printf("o=- %d %d IN IP4 %s\r\n", origin_time, origin_time, voip_ipaddr_ntoa(&s->localAddr.ipaddr));

    buf_printf("s=-\r\n");
    buf_printf("c=IN IP4 %s\r\n", voip_ipaddr_ntoa(&s->localAddr.ipaddr));

    buf_printf("t=0 0\r\n");
    buf_printf("m=audio %d RTP/AVP %d %d\r\n",
               s->sdpMediaLocalPort,
               s->sdpMedia->audioCodec.type,
               s->invSdpMedia.telephone_event);

    buf_printf("a=rtpmap:%d %s/%d\r\n",
               s->sdpMedia->audioCodec.type,
               s->sdpMedia->audioCodec.name,
               s->sdpMedia->audioCodec.clockRate);
    buf_printf("a=ptime:20\r\n");

    buf_printf("a=rtpmap:%d telephone-event/8000\r\n", s->invSdpMedia.telephone_event);
    buf_printf("a=sendrecv\r\n");

#if defined(CFG_VOIP_VIDEO_ENABLE) && (CFG_VOIP_VIDEO_ENABLE == 1)
    buf_printf("m=video 16200 RTP/AVP 96\r\n");
    buf_printf("a=rtpmap:96 VP8/90000\r\n");
//    buf_printf("a=rtcp-fb:* ccm fir\r\n");
/*
    buf_printf("a=rtcp-fb:* ccm tmmbr\r\n");
    buf_printf("a=rtcp-fb:96 nack pli\r\n");
    buf_printf("a=rtcp-fb:96 nack sli\r\n");
    buf_printf("a=rtcp-fb:96 ack rpsi\r\n");
    buf_printf("a=rtcp-fb:96 ccm fir\r\n");
    buf_printf("a=sendrecv\r\n");
*/
#endif

    return buflen;
}

/**
 * parse SIP message
 * @param buf input buffer
 * @param buflen input buffer length
 * @param msg output message
 * @return 0 if success else -1 if any error
 */
int sip_parse_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg)
{
    char *pl;

    memset(msg, 0, sizeof(sip_msg_t));
    parse_request_line(buf, msg);

    // read msg contect buffer lines and parse by type
    while((pl = strstr(buf, SIP_NEWLINE)) != NULL)
    {
        *pl = 0; // terminate line

        if (!strncmp(buf, "Via: ", 5))
            parse_via(s, buf+5, msg);
        else if (!strncmp(buf, "Record-Route: ", 14))
            parse_record_route(buf+14, msg);
        else if (!strncmp(buf, "Route: ", 7))
            parse_route(buf+7, msg);
        else if (!strncmp(buf, "Contact: ", 9))
            parse_contact(buf+9, msg);
        else if (!strncmp(buf, "From: ", 6))
            parse_from(buf+6, msg);
        else if (!strncmp(buf, "To: ", 4))
            parse_to(buf+4, msg);
        else if (!strncmp(buf, "Call-ID: ", 9))
            parse_callId(buf+9, msg);
        else if (!strncmp(buf, "CSeq: ", 6))
            parse_cseq(buf+6, msg);
        else if (!strncmp(buf, "Expires: ", 9))
            parse_expires(buf+9, msg);
        else if (!strncmp(buf, "Min-Expires: ", 13))
            parse_min_expires(buf+13, msg);
        else if (!strncmp(buf, "WWW-Authenticate: ", 18))
        {
            msg->auth.type = AUTH_TYPE_WWWAUTH;
            parse_authenticate(buf+18, msg);
        }
        else if (!strncmp(buf, "Proxy-Authenticate: ", 20))
        {
            msg->auth.type = AUTH_TYPE_PROXYAUTH;
            parse_authenticate(buf+20, msg);
        }
        else if (strstr(buf, "Content-Type: application/sdp"))
        {
            // Set local SDP media
            memcpy(&msg->sdpmedia, s->sdpMedia, sizeof(sip_sdp_media_t));
            msg->has_sdp_content = 1;
        }

        // parse SDP content
        if (msg->has_sdp_content)
        {
            if (strstr(buf, "c=IN IP4 "))
                parse_sdp_media_addr(buf+9, msg);
            else if (strstr(buf, "m=audio "))
                parse_sdp_media_audio_port(buf+8, msg);
            else if (strstr(buf, "m=video "))
                parse_sdp_media_video_port(buf+8, msg);
            else if (strstr(buf, "a=rtpmap:") && strstr(buf, "telephone-event"))
                parse_sdp_media_telephone_event(buf+9, msg);
        }

        pl += 2;    // skip newline '\r\n'
        buf = pl;
    }

    return 0;
}

static void parse_request_line(char *buf, sip_msg_t *msg)
{
    char *pb, *pe;
    char txt[32];

    if ((pb = strstr(buf, "SIP/2.0 ")) != NULL)
    {
        // response SIP message
        msg->messageType = SIPMSG_RESPONSE;

        pb += 8;
        if ((pe = strchr(pb, ' ')) != NULL && pe - pb < sizeof(txt)-1)
        {
            strncpy(txt, pb, pe - pb);
            txt[pe - pb] = '\0';
            msg->messageCode = atoi(txt);
        }
    }
    else if ((pb = strstr(buf, " SIP/2.0\r\n")) != NULL)
    {
        // request SIP message
        msg->messageType = SIPMSG_REQUEST;

        parse_method_type(buf, msg);

        // get request uri ' sip:22112323@100.100.120.1 '
        if ((pb = strstr(buf, " sip:")) != NULL)
        {
            pb += 5;  // skip ' sip:'
            if ((pe = strchr(pb, ' ')) != NULL && pe - pb < sizeof(msg->reqUri)-1)
            {
                strncpy(msg->reqUri, pb, pe - pb);
                msg->reqUri[pe - pb] = '\0';
            }
        }
    }
    else
    {
        TRACE_ERROR("Unknown request line");
        TRACE_DUMP(buf, strlen(buf));
    }
}

static void parse_method_type(char *buf, sip_msg_t *msg)
{
    if (strstr(buf, "REGISTER") != NULL)
        msg->methodType = SIP_METHOD_REGISTER;
    else if (strstr(buf, "OPTIONS") != NULL)
        msg->methodType = SIP_METHOD_OPTIONS;
    else if (strstr(buf, "INVITE") != NULL)
        msg->methodType = SIP_METHOD_INVITE;
    else if (strstr(buf, "BYE") != NULL)
        msg->methodType = SIP_METHOD_BYE;
    else if (strstr(buf, "CANCEL") != NULL)
        msg->methodType = SIP_METHOD_CANCEL;
    else if (strstr(buf, "ACK") != NULL)
        msg->methodType = SIP_METHOD_ACK;
    else
        msg->methodType = SIP_METHOD_UNKNOWN;
}

static void parse_via(struct sip_conn_t *s, char *buf, sip_msg_t *msg)
{
    if (msg->via.count < SIP_NMAX_VIA)
    {
        strlcpy(msg->via.values[msg->via.count].text, buf, SIP_STRING_LEN);
        msg->via.count++;
    }
    else
    {
        TRACE_ERROR("max number of SIP VIAs exceeded");
    }
}

static void parse_record_route(char *buf, sip_msg_t *msg)
{
    if (msg->record_route.count < SIP_NMAX_ROUTE)
    {
        strlcpy(msg->record_route.values[msg->record_route.count].text, buf, SIP_STRING_LEN);
        msg->record_route.count++;
    }
    else
    {
        TRACE_ERROR("max number of SIP Record-routes exceeded");
    }
}

static void parse_route(char *buf, sip_msg_t *msg)
{
    if (msg->route.count < SIP_NMAX_ROUTE)
    {
        strlcpy(msg->route.values[msg->route.count].text, buf, SIP_STRING_LEN);
        msg->route.count++;
    }
    else
    {
        TRACE_ERROR("max number of SIP Record-routes exceeded");
    }
}

static void parse_to(char *buf, sip_msg_t *msg)
{
    strlcpy(msg->to, buf, sizeof(msg->to));
}

static void parse_from(char *buf, sip_msg_t *msg)
{
    strlcpy(msg->from, buf, sizeof(msg->from));
}

static void parse_callId(char *buf, sip_msg_t *msg)
{
    strlcpy(msg->callId, buf, sizeof(msg->callId));
}

static void parse_cseq(char *buf, sip_msg_t *msg)
{
    char *pe;
    char txt[20];

    if ((pe = strchr(buf, ' ')) != NULL && pe - buf < sizeof(txt)-1)
    {
        strncpy(txt, buf, pe - buf);
        txt[pe - buf] = '\0';
        msg->cseq = atoi(txt);

        strlcpy(msg->method, pe+1, sizeof(msg->method));

        parse_method_type(msg->method, msg);
    }
}

static void parse_expires(char *buf, sip_msg_t *msg)
{
    if (msg->expires == 0)
        msg->expires = atoi(buf);
}

static void parse_min_expires(char *buf, sip_msg_t *msg)
{
    msg->expires = atoi(buf);
}

static void parse_contact(char *buf, sip_msg_t *msg)
{
    char *pb, *pe;
    char txt[32];

    if ((pb = strchr(buf, '<')) != NULL)
    {
        pb++;
        if ((pe = strchr(pb, '>')) != NULL && pe - pb < sizeof(msg->contact)-1)
        {
            strncpy(msg->contact, pb, pe - pb);
            msg->contact[pe - pb] = '\0';
        }
    }

    if ((pb = strstr(buf, ";expires=")) != NULL)
    {
        pb += 9; // skip ';expires'

        // find expires delim
        if ((pe = strchr(pb, ',')) != NULL || (pe = strchr(pb, ';')) != NULL || (pe = strchr(pb, '\0')) != NULL)
        {
            if (pe - pb < sizeof(txt)-1)
            {
                strncpy(txt, pb, pe - pb);
                txt[pe - pb] = '\0';
                msg->expires = atoi(txt);
            }
        }
    }
}

static void parse_authenticate(char *buf, sip_msg_t *msg)
{
    char *start, *end;

    // get realm
    if ((start = strstr(buf, "realm=\"")) != NULL)
    {
        start += 7;	// realm="
        if ((end = strchr(start, '\"')) != NULL && end - start < sizeof(msg->auth.realm)-1)
        {
            strncpy(msg->auth.realm, start, end - start);
            msg->auth.realm[end - start] = '\0';
        }
    }

    // get nonce
    if ((start = strstr(buf, "nonce=\"")) != NULL)
    {
        start += 7;		// nonce="
        if ((end = strchr(start, '\"')) != NULL && end - start < sizeof(msg->auth.nonce)-1)
        {
            strncpy(msg->auth.nonce, start, end - start);
            msg->auth.nonce[end - start] = '\0';
        }
    }

    // get opaque
    if ((start = strstr(buf, "opaque=\"")) != NULL)
    {
        start += 8;		// opaque="
        if ((end = strchr(start, '\"')) != NULL && end - start < sizeof(msg->auth.opaque)-1)
        {
            strncpy(msg->auth.opaque, start, end - start);
            msg->auth.opaque[end - start] = '\0';
        }
    }
}

static void parse_sdp_media_addr(char *buf, sip_msg_t *msg)
{
    strlcpy(msg->sdpmedia.addr, buf, sizeof(msg->sdpmedia.addr));
}

static void parse_sdp_media_audio_port(char *buf, sip_msg_t *msg)
{
    char *pe;
    char txt[20];
    if ((pe = strchr(buf, ' ')) != NULL && pe - buf < sizeof(txt)-1)
    {
        strncpy(txt, buf, pe - buf);
        txt[pe - buf] = '\0';
        msg->sdpmedia.audio_port = atoi(txt);
    }
}

static void parse_sdp_media_video_port(char *buf, sip_msg_t *msg)
{
    char *pe;
    char txt[20];
    if ((pe = strchr(buf, ' ')) != NULL && pe - buf < sizeof(txt)-1)
    {
        strncpy(txt, buf, pe - buf);
        txt[pe - buf] = '\0';
        msg->sdpmedia.video_port = atoi(txt);
    }
}

static void parse_sdp_media_telephone_event(char *buf, sip_msg_t *msg)
{
    char *pe;
    char txt[20];
    if ((pe = strchr(buf, ' ')) != NULL && pe - buf < sizeof(txt)-1)
    {
        strncpy(txt, buf, pe - buf);
        txt[pe - buf] = '\0';
        msg->sdpmedia.telephone_event = atoi(txt);
    }
}
