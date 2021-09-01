/**
 * \file sipmsg.h    \brie SIP message builder / parser
 */

#ifndef __sipmsg_h
#define __sipmsg_h

// forward decl.
struct sip_conn_t;


#define SDPBUF_SIZE                             512                   //! buffer size for SDP content
#define SIP20                                   "SIP/2.0"
#define SIP_NEWLINE                             "\r\n"
#define SDP_CONTENT_TYPE                        "application/sdp"
#define SIP_MAX_FORWARDS                        70
#define SIP_EXPIRES                             300
#define SIP_USER_AGENT                          "uSIP"
#define SIP_CALLID_LEN                          128
#define SIP_STRING_LEN                          128                   //! max. length of sip string in header field
#define SIP_METHOD_LEN                          20
#define SIP_REQ_URILEN                          48
#define SIP_SDP_CODEC_NAME_LENGTH               20
#define SIP_SDP_MEDIA_ADDR_LENGTH               20


/** \brief type of SIP message */
typedef enum
{
    SIPMSG_UNKNOWN,
    SIPMSG_REQUEST,             ///< SIP request message type
    SIPMSG_RESPONSE,            ///< SIP response

} sip_msg_type_e;


/** \brief type of SIP method */
typedef enum
{
    SIP_METHOD_UNKNOWN,
    SIP_METHOD_REGISTER,
    SIP_METHOD_OPTIONS,
    SIP_METHOD_INVITE,
    SIP_METHOD_BYE,
    SIP_METHOD_CANCEL,
    SIP_METHOD_ACK

} sip_method_type_e;


/** begin of SIP msg branch format string */
#define SIPMSG_BRANCH_STR  "branch=z9hG4bK-%x"

/** if domain is not known then we are using server ip address */
#define SIPDOMAIN(_p) (*(_p)->domain == 0 ? (_p)->srv_addr : (_p)->domain)

/** max number for VIA records in message */
#define SIP_NMAX_VIA           3

/** max number of route records in message */
#define SIP_NMAX_ROUTE          3


typedef struct
{
    uint8_t type;
    char name[SIP_SDP_CODEC_NAME_LENGTH];
    uint16_t clockRate;
    uint16_t bytesPerFrame;

} sip_sdp_audiocodec_t;

typedef struct
{
    uint8_t type;
    char name[SIP_SDP_CODEC_NAME_LENGTH];
    uint16_t bytesPerFrame;

} sip_sdp_videocodec_t;


/**
 * SDP media information
 */
typedef struct
{
    char addr[SIP_SDP_MEDIA_ADDR_LENGTH];   ///< SDP media address
    uint8_t telephone_event;                ///< DTMF RTP type

    uint16_t audio_port;                    ///< SDP media RTP audio port
    sip_sdp_audiocodec_t audioCodec;        ///< Audiocodec info

    uint16_t video_port;                    ///< SDP media RTP video port
    sip_sdp_videocodec_t videoCodec;        ///< Videocodec info
    char video_transcoder_addr[32];         ///< Video transcoder server address
    uint32_t video_resolution;
    int32_t video_jpeg_quality;

} sip_sdp_media_t;


/** SIP text string */
typedef struct
{
    char text[SIP_STRING_LEN];

} sip_text_value_t;


typedef struct
{
    sip_text_value_t values[SIP_NMAX_VIA];
    int count;

} sip_via_t;


typedef struct
{
    sip_text_value_t values[SIP_NMAX_ROUTE];
    int count;

} sip_route_t;


typedef struct
{
    sip_text_value_t values[SIP_NMAX_ROUTE];
    int count;

} sip_record_route_t;


/**
 * SIP message
 */
typedef struct
{
    sip_msg_type_e messageType;			///< message type
    sip_method_type_e methodType;		///< type of method
    uint16_t messageCode;			    ///< message code when message is response

    char method[SIP_METHOD_LEN];        ///! message method string name
    char reqUri[SIP_REQ_URILEN];

    sip_via_t via;                      ///! VIA list of messages
    sip_route_t route;                  ///! route record list
    sip_record_route_t record_route;    ///! record-route record list

    char from[SIP_STRING_LEN];	        ///< from nickname
    char contact[SIP_STRING_LEN];	    ///< contact name
    char to[SIP_STRING_LEN];	      	///< to nickname
    char callId[SIP_STRING_LEN];		///< message Call ID
    int cseq;							///< CSeq number of message
    uint16_t expires;                   ///! SIP message expires

    sip_auth_t auth;                     ///! SIP authorization
    sip_sdp_media_t sdpmedia;            ///! SDP media information
    uint8_t has_sdp_content;             ///! message has SDP content

} sip_msg_t;


/**
 * build SIP register message
 * @param s SIP connection
 * @param buf output buffer
 * @return if success then return size of message buffer else -1 if any error
 */
int sip_build_register_msg(struct sip_conn_t *s, char *buf);
#define sip_build_unregister_msg sip_build_register_msg
int sip_build_invite_msg(struct sip_conn_t *s, char *buf, const char *remoteUri);
int sip_build_ack_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg, const char *remoteUri);
int sip_build_ok_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg);
int sip_build_ok_sdp_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg);
int sip_build_bye_msg(struct sip_conn_t *s, char *buf, const char *remoteUri);
int sip_build_cancel_msg(struct sip_conn_t *s, char *buf, const char *remoteUri);
int sip_build_unavailable_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg);
int sip_build_decline_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg);
int sip_build_ringing_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg);



/**
 * parse SIP message
 * @param buf input buffer
 * @param buflen input buffer length
 * @param msg output message
 * @return 0 if success else -1 if any error
 */
int sip_parse_msg(struct sip_conn_t *s, char *buf, sip_msg_t *msg);


#endif   // sipmsg.h
