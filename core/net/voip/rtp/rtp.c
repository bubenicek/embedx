/**
 * \file rtp.c				\brief RTP protocol implementation
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "rtp.h"

#define TRACE_TAG "rtp"
#if !ENABLE_TRACE_RTP
#undef TRACE
#define TRACE(...)
#endif

// Prototypes:
static void rtp_poll_callback(voip_udp_socket_t *socket, void *user_param);
static void rtp_recv_callback(voip_udp_socket_t *socket, voip_ipaddr_t *ipaddr, uint16_t port, void *buf, uint16_t buflen, void *user_param);


/** create new RTP connection */
int rtp_create_connection(
    rtp_connection_t *rc,
    voip_ipaddr_t *remoteAddr,
    uint16_t remotePort,
    uint16_t localPort,
    uint16_t payloadType,
    uint16_t bytesPerFrame,
    uint16_t clockRate,
    uint8_t telephone_event,
    uint32_t ssrc,
    const rtp_connection_events_t *events,
    void *userData)
{
    uint16_t poll_interval;

    memset(rc, 0, sizeof(rtp_connection_t));

    // send rtp is enabled only when is remote addr
    if (remoteAddr != NULL)
    {
        // alloc rtp send buffer
        rc->sndbuf_audio = voip_malloc(bytesPerFrame + RTP_HEADER_SIZE);
        if (rc->sndbuf_audio == NULL)
        {
            TRACE_ERROR("alloc rtp send buffer");
            goto fail_alloc_audio;
        }

        // alloc rtp send buffer
        rc->sndbuf_dtmf = voip_malloc(RTP_HEADER_SIZE + RTP_DTMF_EVENT_SIZE);
        if (rc->sndbuf_dtmf == NULL)
        {
            TRACE_ERROR("alloc rtp send buffer");
            goto fail_alloc_dtmf;
        }

        voip_ipaddr_set(&rc->remoteAddr, remoteAddr);
        rc->remotePort = remotePort;

        // Count pool timer interval
        poll_interval = (1000.0 / (double)clockRate) * bytesPerFrame;

        TRACE("RTP send frame interval = %d ms, clockrate = %d  bytesPerFrame = %d", poll_interval, clockRate, bytesPerFrame);
    }
    else
    {
        poll_interval = 0;
        TRACE("RTP clockrate = %d  bytesPerFrame = %d", clockRate, bytesPerFrame);
    }

    // Initialize new RTP connection
    rc->localPort = localPort;
    rc->userData = userData;
    rc->events = events;
    rc->ssrc = ssrc;
    rc->payloadType = payloadType;
    rc->bytesPerFrame = bytesPerFrame;
    rc->telephone_event = telephone_event;

    // initialize RTP session
    pjmedia_rtp_session_init(&rc->rtpSes, payloadType, rc->ssrc);

    // initialize RTPC session
    pjmedia_rtcp_init(&rc->rtpcSes, "uip", clockRate, bytesPerFrame, rc->ssrc);

    // Create UDP connection
    if (voip_create_udp_socket(&rc->socket, &rc->localAddr, rc->localPort, rtp_recv_callback, rtp_poll_callback, poll_interval, rc) != 0)
    {
        TRACE_ERROR("Create UDP socket");
        goto fail_create_socket;
    }

    rc->socket_is_open = true;

    TRACE("RTP connection to %s:%d  *.%d created, telephone_event=%d ",
               remoteAddr != NULL ? voip_ipaddr_ntoa(remoteAddr) : "*", remotePort, localPort, rc->telephone_event);

    return 0;

fail_create_socket:
    voip_free(rc->sndbuf_dtmf);
fail_alloc_dtmf:
    voip_free(rc->sndbuf_audio);
fail_alloc_audio:
    return -1;
}

/** destroy RTP connection */
void rtp_destroy_connection(rtp_connection_t *rc)
{
    if (rc->socket_is_open)
    {
        voip_close_udp_socket(&rc->socket);

        if (rc->sndbuf_dtmf != NULL)
        {
            voip_free(rc->sndbuf_dtmf);
            rc->sndbuf_dtmf = NULL;
        }

        if (rc->sndbuf_audio != NULL)
        {
            voip_free(rc->sndbuf_audio);
            rc->sndbuf_audio = NULL;
        }

        rc->socket_is_open = false;

        TRACE("RTP port destroyed");
    }
}

void rtp_send_dtmf(rtp_connection_t *rc, rtp_dtmf_code_t dtmf_code, uint16_t duration)
{
    pjmedia_rtp_session_setting settings;

    rc->send_dtmf_event.duration = RTP_SEND_DTMF_STEP;
    rc->send_dtmf_event.cnt = duration / 20;
    rc->send_dtmf_event.code = dtmf_code;
    rc->send_dtmf_event.state = RTP_SEND_DTMF_EVENT_STATE_PENDING_MARK;

    // Configure DTMF RTP session (copy from audio RTP session, TS must be constant)
    settings.flags = 0xF;
    settings.default_pt = rc->rtpSes.out_pt;
    settings.sender_ssrc = pj_ntohl(rc->rtpSes.out_hdr.ssrc);
    settings.seq = pj_ntohs(rc->rtpSes.out_hdr.seq);
    settings.ts = pj_ntohl(rc->rtpSes.out_hdr.ts);

    pjmedia_rtp_session_init2(&rc->rtpSes_dtmf, settings);

    TRACE("Send DTMF code = %d  ('%c')", dtmf_code, rtp_dtmfcode_to_ascii(dtmf_code));
}


/** convert dtmf code to ascii */
char rtp_dtmfcode_to_ascii(rtp_dtmf_code_t code)
{
    if (code >= RTP_DTMF_CODE_0 && code <= RTP_DTMF_CODE_9)
        return code + 48;
    else if (code == RTP_DTMF_CODE_ASTERISK)
        return '*';
    else if (code == RTP_DTMF_CODE_HASH)
        return '#';
    else if (code == RTP_DTMF_CODE_A)
        return 'A';
    else if (code == RTP_DTMF_CODE_B)
        return 'B';
    else if (code == RTP_DTMF_CODE_C)
        return 'C';
    else if (code == RTP_DTMF_CODE_D)
        return 'D';
    else
        return 0;
}

/** Convert ascii to code */
rtp_dtmf_code_t rtp_ascii_to_dtmfcode(char c)
{
    switch(c)
    {
    case '0':
        return RTP_DTMF_CODE_0;
    case '1':
        return RTP_DTMF_CODE_1;
    case '2':
        return RTP_DTMF_CODE_2;
    case '3':
        return RTP_DTMF_CODE_3;
    case '4':
        return RTP_DTMF_CODE_4;
    case '5':
        return RTP_DTMF_CODE_5;
    case '6':
        return RTP_DTMF_CODE_6;
    case '7':
        return RTP_DTMF_CODE_7;
    case '8':
        return RTP_DTMF_CODE_8;
    case '9':
        return RTP_DTMF_CODE_9;
    case '*':
        return RTP_DTMF_CODE_ASTERISK;
    case '#':
        return RTP_DTMF_CODE_HASH;
    case 'A':
        return RTP_DTMF_CODE_A;
    case 'B':
        return RTP_DTMF_CODE_B;
    case 'C':
        return RTP_DTMF_CODE_C;
    case 'D':
        return RTP_DTMF_CODE_D;
    default:
        return RTP_DTMF_CODE_UNKNOWN;
    }
}

/** RTP periodical timer handler */
static void rtp_poll_callback(voip_udp_socket_t *socket, void *user_param)
{
    rtp_connection_t *s = user_param;

    if (s->events->on_send_frame_data != NULL)
    {
        uint16_t datalen;

/** TODO: odeslani RTCP packetu shazuje asterisk server !!
        if (!s->rtpc_counter)
        {
            void *rtcp_pkt;
            int len;

            // receiver report
            pjmedia_rtcp_build_rtcp(&s->rtpcSes, &rtcp_pkt, &len);
            pjmedia_rtcp_tx_rtp(&s->rtpcSes, len);
            memcpy(s->sndbuf_audio, rtcp_pkt, len);
            datalen = len;

            // sender report
            pjmedia_rtcp_build_rtcp(&s->rtpcSes, &rtcp_pkt, &len);
            pjmedia_rtcp_tx_rtp(&s->rtpcSes, len);
            memcpy(s->sndbuf_audio + datalen, rtcp_pkt, len);
            datalen += len;

            // send RTPC packet
            voip_send_udp_packet(socket, &s->remoteAddr, RTCP_PORT(s->remotePort), s->sndbuf_audio, datalen);
        }
*/
        s->rtpc_counter++;

        // get data to send
        datalen = s->events->on_send_frame_data(s, s->sndbuf_audio + RTP_HEADER_SIZE, (s->bytesPerFrame + RTP_HEADER_SIZE) - sizeof(pjmedia_rtp_hdr), s->userData);
        if (datalen > 0)
        {
            const pjmedia_rtp_hdr *hdr;
            int hdrlen;

            switch(s->send_dtmf_event.state)
            {
            case RTP_SEND_DTMF_EVENT_STATE_PENDING_MARK:
            case RTP_SEND_DTMF_EVENT_STATE_PENDING:
            {
                rtp_dtmf_event_t *pevt = (rtp_dtmf_event_t *)(s->sndbuf_dtmf + RTP_HEADER_SIZE);

                pevt->id = s->send_dtmf_event.code;
                pevt->volume = RTP_SEND_DTMF_VOLUME;
                pevt->r = 0;
                pevt->e = 0;
                pevt->duration = pj_htons(s->send_dtmf_event.duration);

                pjmedia_rtp_encode_rtp(
                    &s->rtpSes_dtmf,
                    s->telephone_event,
                    (s->send_dtmf_event.state == RTP_SEND_DTMF_EVENT_STATE_PENDING_MARK),   // marker bit
                    sizeof(rtp_dtmf_event_t), // bytes per frame
                    0,                        // Time stamp inc
                    (const void **)&hdr, &hdrlen);

                if (--s->send_dtmf_event.cnt == 0)
                {
                    s->send_dtmf_event.cnt = RTP_SEND_DTMF_END_COUNT;
                    s->send_dtmf_event.state = RTP_SEND_DTMF_EVENT_STATE_END;
                }
                else
                {
                    s->send_dtmf_event.duration +=  RTP_SEND_DTMF_STEP * 2;
                    s->send_dtmf_event.state = RTP_SEND_DTMF_EVENT_STATE_PENDING;
                }

                // copy RTP header to packet
                memcpy(s->sndbuf_dtmf, hdr, hdrlen);

                // send RTP packet
                voip_send_udp_packet(socket, &s->remoteAddr, s->remotePort, s->sndbuf_dtmf, RTP_HEADER_SIZE + RTP_DTMF_EVENT_SIZE);
            }
            break;

            case RTP_SEND_DTMF_EVENT_STATE_END:
            {
                rtp_dtmf_event_t *pevt = (rtp_dtmf_event_t *)(s->sndbuf_dtmf + RTP_HEADER_SIZE);

                pevt->id = s->send_dtmf_event.code;
                pevt->volume = RTP_SEND_DTMF_VOLUME;
                pevt->r = 0;
                pevt->e = 1;
                pevt->duration = pj_htons(s->send_dtmf_event.duration);

                pjmedia_rtp_encode_rtp(
                    &s->rtpSes_dtmf,
                    s->telephone_event,
                    0,                          // marker bit
                    sizeof(rtp_dtmf_event_t),   // bytes per frame
                    0,                          // Time stamp inc
                    (const void **)&hdr, &hdrlen);

                if (--s->send_dtmf_event.cnt == 0)
                {
                    s->send_dtmf_event.state = RTP_SEND_DTMF_EVENT_STATE_NONE;
                    if (s->events->on_sent_dtmf != NULL)
                        s->events->on_sent_dtmf(s, s->send_dtmf_event.code, s->userData);
                }

                // copy RTP header to packet
                memcpy(s->sndbuf_dtmf, hdr, hdrlen);

                // send RTP packet
                voip_send_udp_packet(socket, &s->remoteAddr, s->remotePort, s->sndbuf_dtmf, RTP_HEADER_SIZE + RTP_DTMF_EVENT_SIZE);
            }
            break;

            default:
                break;
            }

            // Send audio data
            pjmedia_rtp_encode_rtp(
                &s->rtpSes,
                s->payloadType,
                0,                       // marker bit
                datalen,                 // bytes per frame
                s->bytesPerFrame,        // number of bytes per frame
                (const void **)&hdr, &hdrlen);

            // copy RTP header to packet
            memcpy(s->sndbuf_audio, hdr, hdrlen);

            // send RTP packet
            voip_send_udp_packet(socket, &s->remoteAddr, s->remotePort, s->sndbuf_audio, s->bytesPerFrame + RTP_HEADER_SIZE);
        }
    }
}

/** Event invoked when is received UDP packet */

static void rtp_recv_callback(voip_udp_socket_t *socket, voip_ipaddr_t *ipaddr, uint16_t port, void *buf, uint16_t buflen, void *user_param)
{
    rtp_connection_t *rc = user_param;

    rc->numReceivedPackets++;

    if (rc->events->on_receive_frame != NULL)
    {
        // add to event handler all received packet
        rc->events->on_receive_frame(rc, buf, buflen, rc->userData);
    }
    else if (rc->events->on_receive_frame_data != NULL)
    {
        const pjmedia_rtp_hdr *hdr;
        const void *pdata;
        unsigned datalen;
        pj_status_t status;

        status = pjmedia_rtp_decode_rtp(
                     &rc->rtpSes,
                     buf,
                     buflen,
                     &hdr,
                     &pdata,
                     &datalen);

        if (status == PJ_SUCCESS)
        {
            if (hdr->pt == rc->payloadType)
            {
                if (rc->events->on_receive_frame_data != NULL)
                    rc->events->on_receive_frame_data(rc, hdr, (void *)pdata, datalen, rc->userData);
            }
            else if (hdr->pt == RTP_COMFORT_NOISE_PAYLOADTYPE)
            {
                if (rc->events->on_receive_comfort_noise != NULL)
                    rc->events->on_receive_comfort_noise(rc, rc->userData);
            }
            else if (hdr->pt == rc->telephone_event ||
                     hdr->pt == RTP_EVENT_PAYLOADTYPE ||
                     hdr->pt == RTP_EVENT2_PAYLOADTYPE ||
                     hdr->pt == RTP_EVENT3_PAYLOADTYPE)
            {
                const rtp_dtmf_event_t *event = pdata;

                if (!event->e)
                {
                    // when is not last event
                    rc->dtmf_event.id = event->id;
                    rc->dtmf_event.e = 1;
                    rc->dtmf_event.volume = event->volume;
                    rc->dtmf_event.duration = event->duration;
                }
                else if (rc->dtmf_event.e)
                {
                    // last event and previous was not last event

                    if (rc->events->on_receive_dtmf != NULL)
                        rc->events->on_receive_dtmf(rc, rc->dtmf_event.id, rc->dtmf_event.volume, rc->dtmf_event.duration, rc->userData);

                    rc->dtmf_event.e = 0;
                }
            }
        }
        else
        {
            TRACE_ERROR("pjmedia_rtp_decode_rtp");
        }
    }
}
