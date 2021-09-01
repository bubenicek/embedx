
#include "system.h"
#include <stdlib.h>
#include "sqlite/sqlite3.h"

#define TRACE_TAG    "sqlite_os"

#if !ENABLE_TRACE_SQLITE_OS
#undef TRACE
#define TRACE(...)
#endif

extern const sqlite3_vfs demovfs;

int sqlite3_os_init(void)
{
   if (sqlite3_vfs_register((sqlite3_vfs *)&demovfs, 1) != SQLITE_OK)
   {
      TRACE_ERROR("VFS register");
      return -1;
   }

   TRACE("os init");

   return 0;
}

int sqlite3_os_end(void)
{
   return sqlite3_vfs_unregister((sqlite3_vfs *)&demovfs);
}
