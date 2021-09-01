/**
 * \file sip.h				\brief Simple SIP protocol imlementation
 */

#ifndef __sip_h
#define __sip_h

#include <stdint.h>

#include "voip_platform.h"
#include "sip_auth.h"
#include "sip_msg.h"


/** SIP register states */
typedef enum
{
    SIPCON_STATE_UNREGISTERED = 0,
    SIPCON_STATE_REGISTER,
    SIPCON_STATE_REGISTERING,
    SIPCON_STATE_REGISTERED,
    SIPCON_STATE_UNREGISTER,
    SIPCON_STATE_UNREGISTERING

} sip_register_state_t;


/** SIP invite states */
typedef enum
{
    SIPCON_STATE_NOINVITED = 100,
    SIPCON_STATE_INVITE,
    SIPCON_STATE_INVITENING,
    SIPCON_STATE_INVPENDING,
    SIPCON_STATE_RINGING,
    SIPCON_STATE_INVITED,
    SIPCON_STATE_HANGUP,
    SIPCON_STATE_HANGUPING,
    SIPCON_STATE_DECLINE,
    SIPCON_STATE_DECLINING,
    SIPCON_STATE_INCOMMING_INVITE,
    SIPCON_STATE_INCOMMING_ACCEPT,
    SIPCON_STATE_INCOMMING_ACCEPTING,
    SIPCON_STATE_INCOMMING_DECLINE

} sip_invite_state_t;


//
// SIP connection status
//
typedef enum
{
    SIPCON_STATUS_NONE              = 0x00,
    SIPCON_STATUS_REGISTERED        = 0x01,     //! connectection is registered
    SIPCON_STATUS_INVITED           = 0x02,     //! connection is invited

} sip_status_t;


//
// SIP error codes
//
typedef enum
{
    SIP_ERR_REGISTER_SERVER_ADDR_RESOLVE = 10,
    SIP_ERR_REGISTER,
    SIP_ERR_REGISTER_FORBIDDEN_ACCESS,
    SIP_ERR_REGISTER_RCVTIMEOUT,

    SIP_ERR_INVITE = 20,
    SIP_ERR_INVITE_USER_NOTFOUND,
    SIP_ERR_INVITE_FORBIDDEN_ACCESS,
    SIP_ERR_INVITE_RCVTIMEOUT

} sip_err_t;

#define IS_SIP_REGISTER_ERROR(errCode)  (errCode >= 10 && errCode < 20)
#define IS_SIP_INVITE_ERROR(errCode)    (errCode >= 20 && errCode < 30)


// forward decl.
struct sip_con_events_t;


/**
 * SIP connection profile
 */
typedef struct
{
    char srv_addr[33];	    //! server IP address
    char domain[32];        //! SIP server domain
    char username[65];		//! user name
    char passwd[33];	    //! password

} sip_profile_t;


/** SIP socket address */
typedef struct
{
    voip_ipaddr_t ipaddr;
    uint16_t port;

} sip_addr_t;


/**
 * SIP connection
 */
typedef struct sip_conn_t
{
    /** SIP conn status */
    sip_status_t status;

    /** UDP connection */
    voip_udp_socket_t udpcon;

    /** Local SIP address */
    sip_addr_t localAddr;

    /** Active SIP profile */
    const sip_profile_t *profile;

    /** SIP connection events */
    const struct sip_con_events_t *events;

    /** SIP authorization */
    sip_auth_t auth;

    /** Connection tag */
    int tag;

    /** Shared SIP message buffer */
    sip_msg_t msg;


    //
    // Registration transaction
    //

    /** SIP register state */
    sip_register_state_t regstate;

    /** Register SIP address */
    sip_addr_t registerAddr;

    /** Registraton receive response timer */
    voip_timer_t reg_rcvtimer;

    /** Registratin counter of try to send messages */
    uint8_t reg_sndTryCnt;

    /** Register timer */
    voip_timer_t regtimer;

    /** Registration expires time in sec */
    uint16_t expires;

    voip_clock_time_t lastRegTime;        //! Last register time
    uint16_t regCount;               //! Number of registers
    uint16_t errRegCount;            //! Number of register errors

    char regCallID[SIP_CALLID_LEN];  //! Register tag


    //
    // Invite transaction
    //

    /** SIP register and invite state */
    sip_invite_state_t invstate;

    /** Incomming/outgoing invite addr */
    sip_addr_t inviteAddr;

    /** Invite receive response timer */
    voip_timer_t inv_rcvtimer;
    /** Invite counter of try to send messages */
    uint8_t inv_sndTryCnt;

    voip_clock_time_t lastInvStartTime;   //! Last invite call time
    uint16_t lastInvEndTime;         //! Last invite call length
    uint16_t errInvCount;            //! Number of invite errors

    const char *invRemoteUri;        //! Remote invited URI
    const sip_sdp_media_t *sdpMedia; //! Available SDP media info
    uint16_t sdpMediaLocalPort;      //! RTP media local port
    sip_sdp_media_t invSdpMedia;     //! Incomming invite SDP media info
    char invContact[SIP_STRING_LEN]; //! Invite contact

    uint16_t lastCseq;               //! cseq message counter
    char invCallID[SIP_CALLID_LEN];  //! Invite tag call ID
    int invTag;                      //! Invite tag
    char invFrom[SIP_STRING_LEN];	 //! Invite from nickname
    sip_record_route_t invRecordRoute; //! Record route list

    uint8_t directCall;              //! Direct call without SIP server

    uint16_t lastMsgCode;            //! Last SIP msg code
    uint16_t lastErrorCode;          //! Last SIP error code

    void *userData;                  //! User data pointer

    char buffer[SIP_BUFFER_SIZE];    //! Shared buffer for read/write operation

} sip_con_t;


/**
 * SIP connections events
 */
typedef struct sip_con_events_t
{
    /** calling event when connection success registered */
    void (*on_register)(sip_con_t *sc, const sip_profile_t *prof);

    /** calling when connection success unregistered */
    void (*on_unregister)(sip_con_t *sc, const sip_profile_t *prof);

    /** calling when is success invite */
    void (*on_invite)(sip_con_t *sc, const char *remoteUri, const char *remote_addr, uint16_t remote_port, uint16_t local_port, const sip_sdp_media_t *sdp_media);

    /** on invite trying */
    void (*on_invite_trying)(sip_con_t *sc, const char *remoteUri);

    /** on invite ringing */
    void (*on_invite_ringing)(sip_con_t *sc, const char *remoteUri);

    /** calling when is connection hangup */
    void (*on_hangup)(sip_con_t *sc, const char *remoteUri);

    /** Invoked on decline outgoing invite request */
    void (*on_invite_declined)(sip_con_t *sc, const char *remoteUri);

    /** calling when incomming invite call is comming */
    void (*on_incomming_invite_ringing)(sip_con_t *sc, const char *remoteUri);

    /** calling on some connection error */
    void (*on_error)(sip_con_t *sc, uint16_t sipErrCode, sip_err_t errcode);

} sip_con_events_t;

//--------------------------------------------------------------------------

/** SIP connection process */
void sip_task(sip_con_t *sc);

/** create new SIP connection */
int sip_create_con(
    sip_con_t *sc,
    const char *srvaddr,
    uint16_t remotePort,
    uint16_t localPort,
    const sip_sdp_media_t *sdpMedia,
    const sip_con_events_t *e,
    void *userData);

/** create new SIP direct connection */
int sip_create_direct_con(
    sip_con_t *sc,
    const sip_profile_t *profile,
    uint16_t remotePort,
    uint16_t localPort,
    const sip_sdp_media_t *sdpMedia,
    const sip_con_events_t *e,
    void *userData);

/** destroy SIP connection */
int sip_destroy_con(sip_con_t *sc);

/** register SIP connection */
int sip_register(sip_con_t *sc, const sip_profile_t *prof, uint16_t reg_expires);

/** unregister SIP connection */
int sip_unregister(sip_con_t *sc);

/** invite call */
int sip_invite(sip_con_t *sc, const char *remoteUri);

/** hangup incomming/invited call */
int sip_hangup(sip_con_t *sc);

/** accept incomming call */
int sip_accept_invite(sip_con_t *sc);

/** get user data pointer */
#define sip_con_userData(sc) sc->userData

/** return true when is SIP stack registered */
#define sip_con_registered(sc) ((*sc).status & SIPCON_STATUS_REGISTERED)
#define sip_con_invited(sc) ((*sc).status & SIPCON_STATUS_INVITED)

#define sip_error(_s, _msgCode, _errCode)\
  _s->lastMsgCode = _msgCode;\
  _s->lastErrorCode = _errCode;\
  if(_s->events->on_error != NULL)\
   _s->events->on_error(_s, _msgCode, _errCode);


#endif   // sip.h
