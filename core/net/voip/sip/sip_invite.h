/**
 * \file sip_invite.h       \brief SIP invite dialog processor
 */

#ifndef __sip_invite_h
#define __sip_invite_h

/** Periodicaly timer calling */
void sip_invite_timer(sip_con_t *s);

/** Handle invite response */
void sip_invite_response_handler(sip_con_t *s, voip_ipaddr_t *addr, uint16_t port);

/** Handle invite request */
void sip_invite_request_handler(sip_con_t *s, voip_ipaddr_t *addr, uint16_t port);


#endif   // sip_invite.h

