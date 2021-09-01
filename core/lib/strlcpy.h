
#ifndef __STRLCPY_H
#define __STRLCPY_H

/* 
 * Copies src to the dest buffer. The copy will never overflow the dest buffer
 * and dest will always be null terminated, len is the size of the dest buffer.
 *
 * Returns the length of the src buffer.
 */ 
size_t strlcpy(char *dest, const char *src, size_t len);


#endif // __STRLCPY_H