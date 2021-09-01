/**
 * \file testsip.c      \brief Test SIP and RTP protocol
 */

#ifndef __testsip_h
#define __testsip_h

#include <uip/pt.h>
#include <sip/sip.h>
#include <rtp/rtp.h>

// time for accepting incomming call
#define ACCEPT_TIMER_INTERVAL          3000

//
// Test SIP commands
//
#define TESTSIP_CMD_NONE                  0
#define TESTSIP_CMD_CALL                  1
#define TESTSIP_CMD_HANGUP                2
#define TESTSIP_CMD_ACCEPT                3


typedef struct
{
   struct pt pt;
   struct timer timer, acptimer;
   uint8_t cmd;                     //! test command
   sip_con_t *scon;                 //! SIP connection
   rtp_connection_t *rtpcon;        //! RTP connection

   char *callcontact;
   uint8_t acceptCall;              //! accept call flag

} testsip_state_t;


void testsip_init(testsip_state_t *s);
void testsip_call(testsip_state_t *s, char *contact);
void testsip_hangup(testsip_state_t *s);
void testsip_accept(testsip_state_t *s);

#endif   // testsip.h
