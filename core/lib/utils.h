
#ifndef __UTILS_H
#define __UTILS_H


/** Convert string to double */
double str_todouble(const char *str);

/** Remove all occurence of char from string */
void str_remove(char *str, char c);

/** Function to remove white spaces on both sides of a string i.e trim */
void str_trim(char *s);

/** Replace char in string */
void str_replace(char *str, char c1, char c2);

/** Replace string s1 in string str with new string s2 */
int str_replace_str(char *line, const char *search, const char *replace);

/** Convert string to lower */
void str_tolower(char *str);

/** Check for string is number */
int str_is_number(char *str);

/**
 * Split string buffer to array
 * @param buf input string buffer
 * @param delim char delimiter
 * @param argv output argumets array
 * @param argv_size max number of args
 * @return number of arguments
 */
int split_line(char *buf, char delim, char **argv, int argv_size);

/** Read line from file */
int readline(int fd, char *buf, int bufsz);

// Converts a hexadecimal string to integer
int xtoi(const char* xs, unsigned int* result);

const char *time2str(hal_time_t duration, char *buf, int bufsize);
const char *unixtime2str(time_t duration, char *buf, int bufsize);


#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i)

/** Convert byte to hex string */
static inline void byte2hex(uint8_t c, char *buf)
{
   buf[0] = TO_HEX(((c & 0x00F0) >> 4));
   buf[1] = TO_HEX((c & 0x000F));
}

const char *buf2hex(uint8_t *inbuf, int inbufsize, char *outbuf, int outbufsize);

#endif // __UTILS_H
