
#ifndef _OSAPI_H_
#define _OSAPI_H_

#include "strlcpy.h"

//
// Memory functions
//
#define os_memcmp    memcmp
#define os_memcpy    memcpy
#define os_memmove   memmove
#define os_memset    memset


//
// String functions
//
#define os_strcat    strcat
#define os_strchr    strchr
#define os_strcmp    strcmp
#define os_strcpy    strcpy
#define os_strlen    strlen
#define os_strncmp   strncmp
#define os_strncpy   strncpy
#define os_strstr    strstr
#define os_strdup    strdup
#define os_sprintf   sprintf
#define os_snprintf  snprintf
#define os_strlcpy   strlcpy


//
// Memory management functions
//
#define os_malloc    osMemAlloc
#define os_realloc   osMemRealloc
#define os_free      osMemFree


#endif

