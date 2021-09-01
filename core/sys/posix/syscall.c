
#include "system.h"

TRACE_TAG(syscall);

void abort(void)
{
   TRACE("abort");
   ASSERT(0);
}

void _exit(int __status )
{
   TRACE("_exit");
   ASSERT(0);
}

void _fini(void)
{
   TRACE("_fini");
}
