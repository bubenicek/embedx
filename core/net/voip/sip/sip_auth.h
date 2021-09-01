/**
 * \file sipauth.h			\brief SIP authorization
 */

#ifndef __sip_auth_h
#define __sip_auth_h


/** \brief type of authorization */
#define AUTH_TYPE_NOAUTH       0     ///< no authorization required
#define AUTH_TYPE_WWWAUTH      1	 ///< WWW authorization required
#define AUTH_TYPE_PROXYAUTH    2	 ///< proxy authorization required

/**
 * SIP authorization data
 */
typedef struct
{
   uint8_t type;					 ///< athorization type of message
   char opaque[64];
   char realm[64];					 ///< authorization realm
   char nonce[64];					 ///< authorization nonce
   uint8_t digest[64];              ///< MD5 auth digest

} sip_auth_t;


void sip_auth_digest(const char *username, const char *passwd, const char *method, const char *uri, sip_auth_t *auth);


#endif  // sipauth.h
