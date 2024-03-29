/*
 * Copyright � 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#include <string.h>

/* 
 * Copies src to the dest buffer. The copy will never overflow the dest buffer
 * and dest will always be null terminated, len is the size of the dest buffer.
 *
 * Returns the length of the src buffer.
 */ 
size_t strlcpy(char *dest, const char *src, size_t len) 
{ 
	size_t src_len = strlen(src); 
	size_t new_len; 

	if (len == 0) {
		return (src_len);
	}

        if (src_len >= len) {
		new_len = len - 1;
	} else {
                new_len = src_len;
	}

        memcpy(dest, src, new_len); 
	dest[new_len] = '\0'; 
	return (src_len); 
}
