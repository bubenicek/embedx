/**
 * \file fslog.h     \broef Filesystem log
 */

#ifndef __FSLOG_H
#define __FSLOG_H

/** Output buffer size */
#ifndef CFG_FSLOG_BUFSIZE
#define CFG_FSLOG_BUFSIZE              1024
#endif

/** Log filename prefix */
#ifndef CFG_FSLOG_NAME
#define CFG_FSLOG_NAME                 "/system"
#endif

/** Log file name extension */
#ifndef CFG_FSLOG_EXT
#define CFG_FSLOG_EXT                  "log"
#endif
  
/** Max number of rotate files */
#ifndef CFG_FSLOG_NUM_ROTATE_FILES
#define CFG_FSLOG_NUM_ROTATE_FILES     4
#endif

/** Max size of rotated file */
#ifndef CFG_FSLOG_ROTATE_FILESIZE
#define CFG_FSLOG_ROTATE_FILESIZE      (256 * 1024)
#endif

/** Current fslog filename */
#define FSLOG_CURRENT_NAME             CFG_FSLOG_NAME "0." CFG_FSLOG_EXT


/**
 * Open filesystem log file
 * @param filenema log filename
 * @param num_rotate_files NUmber of rotate files
 * @param rotate_file_size Filesize when logfile is rotated
 * @return 0 if ok else -1 if any error
 */
int fslog_init(void);

/** Close filesystem log file */
int fslog_deinit(void);

/** Lock access to fslog */
void fslog_lock(void);

/** Unlock access to fslog */
void fslog_unlock(void);

/** Write char to fslog */
void fslog_putchar(char c);

/** Flush buffer to disk */
void fslog_flush(void);

/** Get fslog file size */
uint32_t fslog_size(void);

/** Rotate files */
int fslog_rotate(void);

#endif // __FSLOG_H