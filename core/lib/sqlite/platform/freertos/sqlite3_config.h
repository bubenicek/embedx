
#ifndef __SQLITE_CONFIG_H
#define __SQLITE_CONFIG_H

#define SQLITE_OS_OTHER                   1
#define SQLITE_THREADSAFE                 0
#define THREADSAFE                        0

#define SQLITE_ENABLE_MEMORY_MANAGEMENT
#define SQLITE_ENABLE_MEMSYS5
#define SQLITE_DEFAULT_CACHE_SIZE         10
#define SQLITE_DEFAULT_TEMP_CACHE_SIZE    10
#define SQLITE_DEFAULT_PAGE_SIZE          1024

#define SQLITE_TEMP_STORE                 3        //(0 = Always use temporary files, 3 = memory)

#endif // __SQLITE_CONFIG_H
