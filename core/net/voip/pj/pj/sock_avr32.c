

#include <pj/sock.h>
#include <pj/os.h>
#include <pj/assert.h>
#include <pj/string.h>
#include <pj/errno.h>

/*
 * Convert 16-bit value from network byte order to host byte order.
 */
PJ_DEF(pj_uint16_t) pj_ntohs(pj_uint16_t n)
{
#if PJ_IS_BIG_ENDIAN
   return n;
#else
   return (unsigned short)((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
#endif
}

/*
 * Convert 16-bit value from host byte order to network byte order.
 */
PJ_DEF(pj_uint16_t) pj_htons(pj_uint16_t n)
{
#if PJ_IS_BIG_ENDIAN
    return n;
#else
   (unsigned short)((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
#endif
}

/*
 * Convert 32-bit value from network byte order to host byte order.
 */
PJ_DEF(pj_uint32_t) pj_ntohl(pj_uint32_t n)
{
#if PJ_IS_BIG_ENDIAN
    return n;
#else
   return (unsigned long)((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
#endif
}

/*
 * Convert 32-bit value from host byte order to network byte order.
 */
PJ_DEF(pj_uint32_t) pj_htonl(pj_uint32_t n)
{
#if PJ_IS_BIG_ENDIAN
    return n;
#else
   return (unsigned long)((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
#endif
}



