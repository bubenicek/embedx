/**
 * \file voip_video.c           \brief VOIP RTP video callback impl.
 */

#include "system.h"
#include "voip_video.h"

#define TRACE_TAG "voip-video"
#if !ENABLE_TRACE_VOIP_VIDEO
#undef TRACE
#define TRACE(...)
#endif

// Enable video recv rtp stream loopack
#define CFG_VIDEO_ENABLE_LOOPBACK       0


// Prototypes:
static void rtp_poll_callback(voip_udp_socket_t *socket, void *user_param);
static void rtp_recv_callback(voip_udp_socket_t *socket, voip_ipaddr_t *ipaddr, uint16_t port, void *buf, uint16_t buflen, void *user_param);


/** Open audio RTP stream */
int voip_video_open_stream(voip_video_t *video, const char *remote_addr, uint16_t remote_port, uint16_t local_port, const sip_sdp_media_t *sdp_media)
{
    memset(video, 0, sizeof(voip_video_t));
    video->sdp_media = sdp_media;

    // Make IP address from string
    if (voip_ipaddr_aton(remote_addr, &video->remote_addr) != 0)
    {
        TRACE_ERROR("Not valid remote ipaddr: %s", remote_addr);
        goto fail_aton;
    }
    video->remote_port = remote_port;

    // Create UDP connection
    if (voip_create_udp_socket(&video->socket, &video->local_addr, local_port, rtp_recv_callback, rtp_poll_callback, CFG_VOIP_VIDEO_POLL_MS, video) != 0)
    {
        TRACE_ERROR("Create UDP socket");
        throw_exception(fail_socket);
    }

    video->active = true;

    TRACE("Video stream open to %s:%d  local-port: %d", remote_addr, remote_port, local_port);

    return 0;

fail_socket:
fail_aton:
    return -1;
}

/** Open audio RTP stream */
int voip_video_open_transcoding_stream(voip_video_t *video, uint32_t sip_uid, const char *transcoding_addr, uint16_t transcoding_port, const char *remote_addr, uint16_t remote_port, uint16_t local_port, const sip_sdp_media_t *sdp_media)
{
    voip_video_driver_config_t cfg = {
        .resolution = sdp_media->video_resolution,
        .jpeg_quality = sdp_media->video_jpeg_quality,
    };

    memset(video, 0, sizeof(voip_video_t));
    video->sdp_media = sdp_media;

    // Open video driver
    if ((video->video_drv = video_driver_open(CFG_VOIP_VIDEO_DRIVER_NAME, &cfg)) == NULL)
    {
        TRACE_ERROR("Open video driver failed");
        throw_exception(fail_open_drv);
    }

    // Make Remote IP address from string
    if (voip_ipaddr_aton(remote_addr, &video->remote_addr) != 0)
    {
        TRACE_ERROR("Not valid remote ipaddr: %s", remote_addr);
        goto fail_aton;
    }
    video->remote_port = remote_port;

    // Make transcoding server IP address from string
    if (voip_ipaddr_aton(transcoding_addr, &video->transcoding.server_addr) != 0)
    {
        TRACE_ERROR("Not valid remote ipaddr: %s", transcoding_addr);
        goto fail_aton;
    }
    video->transcoding.server_port = transcoding_port;

    // Initializer transcoding packet
    video->transcoding.packet = (video_transcoding_packet_t *)video->transcoding.buffer;
    video->transcoding.packet->seqn = 0;
    video->transcoding.packet->sip_uid = sip_uid;
    video->transcoding.packet->flags = 0;
    voip_ipaddr_set((voip_ipaddr_t *)&video->transcoding.packet->dst_ip, &video->remote_addr);
    video->transcoding.packet->dst_port = video->remote_port;

    // Create UDP connection
    if (voip_create_udp_socket(&video->socket, &video->local_addr, local_port, rtp_recv_callback, rtp_poll_callback, CFG_VOIP_VIDEO_POLL_MS, video) != 0)
    {
        TRACE_ERROR("Create UDP socket");
        throw_exception(fail_socket);
    }

    video->active = true;

    TRACE("Video transcoding stream opened to %s:%d  local-port: %d", remote_addr, remote_port, local_port);

    return 0;

fail_socket:
fail_aton:
    video_driver_close(video->video_drv);
fail_open_drv:
    return -1;
}


/** Close audio RTP stream */
int voip_video_close_stream(voip_video_t *video)
{
    if (video->active)
    {
        voip_close_udp_socket(&video->socket);
        video_driver_close(video->video_drv);
        video->active = false;

        TRACE("Video stream closed");
    }

    return 0;
}

/** RTP periodical timer handler */
static void rtp_poll_callback(voip_udp_socket_t *socket, void *user_param)
{
#if defined (CFG_VIDEO_ENABLE_LOOPBACK) && (CFG_VIDEO_ENABLE_LOOPBACK == 0)
    int len, ip;
    voip_video_t *video = user_param;
    bool start_frame;
    static uint32_t relax_time = 0;

    if (relax_time > 0)
    {
        if (hal_time_ms() >= relax_time)
        {
            relax_time = 0;
        }
        else
        {
            return;
        }
    }

    for (ip = 0; ip < CFG_VOIP_VIDEO_NUM_PACKETS_PER_PERIOD; ip++)
    {
        len = video_driver_read(video->video_drv, video->transcoding.packet->payload, CFG_VOIP_VIDEO_TRANSCODING_BUFSIZE - sizeof(video_transcoding_packet_t), &start_frame);
        if (len > 0)
        {
            video->transcoding.packet->flags = start_frame ? VIDEO_TRANSCODING_FLAG_START_FRAME : 0;
            video->transcoding.packet->seqn++;
            video->transcoding.packet->payload_length = len;

            // Send transcoding packet
            if (voip_send_udp_packet(socket, &video->transcoding.server_addr, video->transcoding.server_port, video->transcoding.packet, sizeof(video_transcoding_packet_t) + len) != 0)
            {
                relax_time = hal_time_ms() + 1000;
                TRACE_ERROR("Send transcoding UDP packet failed");
                break;
            }
        }
        else if (len < 0)
        {
            TRACE_ERROR("voip video driver read failed");
        }
        else
        {
            // No data to send
            break;
        }
    }

#endif
}

/** Event invoked when is received UDP packet */
static void rtp_recv_callback(voip_udp_socket_t *socket, voip_ipaddr_t *ipaddr, uint16_t port, void *buf, uint16_t buflen, void *user_param)
{
#if defined (CFG_VIDEO_ENABLE_LOOPBACK) && (CFG_VIDEO_ENABLE_LOOPBACK == 1)
    voip_video_t *video = user_param;

    TRACE("Recv %d bytes", buflen);

    // Send RTP loopback packet
    if (voip_send_udp_packet(socket, &video->remote_addr, video->remote_port, buf, buflen) != 0)
    {
        TRACE_ERROR("Send loop UDP packet failed");
    }
#endif
}
