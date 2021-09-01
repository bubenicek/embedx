/**
 * \file memlog.h     \broef Memory log
 */

#ifndef __MEMLOG_H
#define __MEMLOG_H

/** Output buffer size */
#ifndef CFG_MEMLOG_BUFSIZE
#define CFG_MEMLOG_BUFSIZE              1024
#endif


/** Put char to memory log */
void memlog_putchar(char c);

/** Get data from memory log */
int memlog_read(char *buf, int bufsize);


#endif // __MEMLOG_H