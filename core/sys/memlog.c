/**
 * \file memlog.c     \brief Memory log
 */

#include "system.h"

#if defined (CFG_MEMLOG_ENABLED) && (CFG_MEMLOG_ENABLED == 1)

//TRACE_TAG(memlog);
//#if !ENABLE_TRACE_MEMLOG
//#include "trace_undef.h"
//#endif

// Locals:
static char membuf[CFG_MEMLOG_BUFSIZE];
static volatile uint32_t membuf_head = 0;
static volatile uint32_t membuf_tail = 0;


/**
 * Open memory system log file
 * @return 0 if ok else -1 if any error
 */
int memlog_init(void)
{
   membuf_head = 0;
   membuf_tail = 0;

   return 0;
}

/** Put char to memory log */
void memlog_putchar(char c)
{
   uint32_t nxthead;

   nxthead = (membuf_head + 1) & (CFG_MEMLOG_BUFSIZE - 1);
   if (nxthead == membuf_tail)
   {
      // Remove last char
      membuf_tail = (membuf_tail + 1) & (CFG_MEMLOG_BUFSIZE - 1);
   }      

   membuf[membuf_head] = c;
   membuf_head = nxthead;
}

/** Get data from memory log */
int memlog_read(char *buf, int bufsize)
{
   int total = 0;

   while (membuf_head != membuf_tail && total < bufsize)
   {
      *buf = membuf[membuf_tail];
      membuf_tail = (membuf_tail + 1) & (CFG_MEMLOG_BUFSIZE - 1);
      buf++;
      total++;
   }

   return total;
}

#endif   // CFG_MEMLOG_ENABLED
