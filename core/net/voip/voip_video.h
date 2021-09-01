
#ifndef __VOIP_VIDEO_H
#define __VOIP_VIDEO_H

#include "voip.h"
#include "video/video_transcoder.h"

#define CFG_VOIP_VIDEO_POLL_MS                      5
#define CFG_VOIP_VIDEO_TRANSCODING_BUFSIZE          1400
#define CFG_VOIP_VIDEO_NUM_PACKETS_PER_PERIOD       10


typedef struct
{
    bool active;

    /** Audio settings */
    const sip_sdp_media_t *sdp_media;

    /** Video driver */
    const voip_video_driver_t *video_drv;

    /** RTP UDP connection */
    voip_udp_socket_t socket, socket2;
    voip_ipaddr_t local_addr;

    voip_ipaddr_t remote_addr;
    uint16_t remote_port;

    struct
    {
        voip_ipaddr_t server_addr;
        uint16_t server_port;
        uint8_t buffer[CFG_VOIP_VIDEO_TRANSCODING_BUFSIZE];
        video_transcoding_packet_t *packet;

    } transcoding;


} voip_video_t;


/** Open video RTP stream */
int voip_video_open_stream(voip_video_t *video, const char *remote_addr, uint16_t remote_port, uint16_t local_port, const sip_sdp_media_t *sdp_media);

/** Open video transcoding RTP stream */
int voip_video_open_transcoding_stream(voip_video_t *video, uint32_t sip_uid, const char *transcoding_addr, uint16_t transcoding_port, const char *remote_addr, uint16_t remote_port, uint16_t local_port, const sip_sdp_media_t *sdp_media);

/** Close audio RTP stream */
int voip_video_close_stream(voip_video_t *video);

/** Return true if video is active */
#define voip_video_is_active(_video) (_video)->active



#endif   // __VOIP_VIDEO_H
