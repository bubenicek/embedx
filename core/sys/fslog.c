/**
 * \file fslog.c     \broef Filesystem log
 */

#include "system.h"
#if defined (CFG_FSLOG_ENABLED) && (CFG_FSLOG_ENABLED == 1)
#include "fslog.h"

TRACE_TAG(fslog);
#if !ENABLE_TRACE_FSLOG
#include "trace_undef.h"
#endif

// Prototypes:
static int open_rotate_file(int index);
static int close_rotate_file(void);

// Locals:
static osMutexId mutex;
static int fd = -1;
static char buf[CFG_FSLOG_BUFSIZE];
static volatile int buf_head = 0;
static volatile int buf_tail = 0;


/**
 * Open filesystem log file
 * @param filenema log filename
 * @param num_rotate_files NUmber of rotate files
 * @param rotate_file_size Filesize when logfile is rotated
 * @return 0 if ok else -1 if any error
 */
int fslog_init(void)
{
   buf_head = 0;
   buf_tail = 0;

   if ((mutex = osMutexCreate(NULL)) == NULL)
      return -1;

   return open_rotate_file(0);
}

/** Close filesystem log file */
int fslog_deinit(void)
{
   osMutexDelete(mutex);
   return close_rotate_file();
}

/** Lock access to fslog */
void fslog_lock(void)
{
   osMutexWait(mutex, osWaitForever);
}

/** Unlock access to fslog */
void fslog_unlock(void)
{
   osMutexRelease(mutex);
}

/** Write buffer to fslog file */
void fslog_putchar(char c)
{
   int nxthead;

   nxthead = (buf_head + 1) & (CFG_FSLOG_BUFSIZE - 1);
   if (nxthead == buf_tail)
      fslog_flush();

   buf[buf_head] = c;
   buf_head = nxthead;

   if (c == '\n')
      fslog_flush();
}

void fslog_flush(void)
{
   if (fd != -1)
   {
      osMutexWait(mutex, osWaitForever);

      // Write buffer to file
      if (buf_head > buf_tail)
      {
         // Write buffer before head
         VERIFY(write(fd, &buf[buf_tail], buf_head - buf_tail) == buf_head - buf_tail);
         buf_tail += buf_head - buf_tail;
      }
      else
      {
         // Write buffer before end
         VERIFY(write(fd, &buf[buf_tail], CFG_FSLOG_BUFSIZE - buf_tail) == CFG_FSLOG_BUFSIZE - buf_tail);

         // Write buffer from begin before head
         VERIFY(write(fd, buf, buf_head) == buf_head);
         buf_tail = buf_head;
      }

      osMutexRelease(mutex);
   }
}

/** Get fslog size */
uint32_t fslog_size(void)
{
   int res = 0;
   struct stat buf;

   if (fd != -1)
   {
      if ((res = fstat(fd, &buf) == 0))
      {
         res = buf.st_size;   
      }
   }
   
   return res;
}

/** Rotate files */
int fslog_rotate(void)
{
   int ix, res;
   char path1[64];
   char path2[64];

   osMutexWait(mutex, osWaitForever);

   close_rotate_file();

   TRACE("Rotate files");

   // Remove last file
   snprintf(path1, sizeof(path1), "%s%d.%s", CFG_FSLOG_NAME, CFG_FSLOG_NUM_ROTATE_FILES-1, CFG_FSLOG_EXT);
   unlink(path1);
   TRACE("  Remove %s", path1);

   // Rename files
   for (ix = CFG_FSLOG_NUM_ROTATE_FILES-2; ix >= 0; ix--)
   {
      snprintf(path1, sizeof(path1), "%s%d.%s", CFG_FSLOG_NAME, ix, CFG_FSLOG_EXT);
      snprintf(path2, sizeof(path2), "%s%d.%s", CFG_FSLOG_NAME, ix+1, CFG_FSLOG_EXT);
      rename(path1, path2);
      TRACE("  Rename %s -> %s", path1, path2);
   }

   // Open primary log file
   res = open_rotate_file(0);

   osMutexRelease(mutex);

   return res;
}

static int open_rotate_file(int index)
{
   char path[64];

   snprintf(path, sizeof(path), "%s%d.%s", CFG_FSLOG_NAME, index, CFG_FSLOG_EXT);
   fd = open(path, O_CREAT| O_APPEND | O_WRONLY, 0644);
   if (fd < 0)
   {
      TRACE_ERROR("Open fslog file %s", path);
      return -1;
   }

   return 0;
}

static int close_rotate_file(void)
{
   int res;

   res = close(fd);
   fd = -1;

   return res;
}

#endif   // CFG_FSLOG_ENABLED
