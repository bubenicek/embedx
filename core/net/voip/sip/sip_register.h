
#ifndef __sip_register_h
#define __sip_register_h


/** Register periodically timer function */
void sip_register_timer(sip_con_t *s);

/** Incomming register response handler */
void sip_register_response_handler(sip_con_t *s, voip_ipaddr_t *addr, uint16_t port);


#endif

