/**
 * \file sipauth.c			\brief SIP authorization
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "sip_md5.h"
#include "sip.h"

#define TRACE_TAG "sip-auth"
#if !ENABLE_TRACE_SIP_AUTH
#undef TRACE
#define TRACE(...)
#endif

static void digest_to_str(unsigned char digest[16], char *str)
{
   int ix;
   char *pstr = str;

   for (ix = 0; ix < 16; ix++)
      pstr += sprintf(pstr, "%2.2x", digest[ix]);

   pstr++;
   *pstr = 0;
}

void sip_auth_digest(const char *username, const char *passwd, const char *method, const char *uri, sip_auth_t *auth)
{
   switch(auth->type)
   {
      case AUTH_TYPE_WWWAUTH:
      case AUTH_TYPE_PROXYAUTH:
      {
         SIP_MD5Context h;
         char sha1[40];
         char sha2[40];

         TRACE("AUTH: username=%s  passwd=%s  method=%s  uri=%s  realm=%s nonce=%s",
               username, passwd, method, uri, auth->realm, auth->nonce);

         // create MD5 hash1 (username:realm:passwd)
         sip_md5_init(&h);
         sip_md5_update(&h, username, strlen(username));
         sip_md5_update(&h, ":", 1);
         sip_md5_update(&h, auth->realm, strlen(auth->realm));
         sip_md5_update(&h, ":", 1);
         sip_md5_update(&h, passwd, strlen(passwd));
         sip_md5_final(&h, auth->digest);
         digest_to_str(auth->digest, sha1);
         TRACE("SHA1 = [%s]", sha1);

         // create MD5 hash2 (method:uri)
         sip_md5_init(&h);
         sip_md5_update(&h, method, strlen(method));
         sip_md5_update(&h, ":", 1);
         sip_md5_update(&h, "sip:", 4);
         sip_md5_update(&h, uri, strlen(uri));
         sip_md5_final(&h, auth->digest);
         digest_to_str(auth->digest, sha2);
         TRACE("SHA2 = [%s]", sha2);

         // create MD5 response digest (HA1:nonce:HA2)
         sip_md5_init(&h);
         sip_md5_update(&h, sha1, strlen(sha1));
         sip_md5_update(&h, ":", 1);
         sip_md5_update(&h, auth->nonce, strlen(auth->nonce));
         sip_md5_update(&h, ":", 1);
         sip_md5_update(&h, sha2, strlen(sha2));
         sip_md5_final(&h, auth->digest);

         digest_to_str(auth->digest, sha2);
         TRACE("DIGEST = [%s]", sha2);
      }
      break;

   }
}
