
#include "system.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>


/** Convert string to double */
double str_todouble(const char *str)
{
    char *eptr;
    return strtod(str, &eptr);
}

/** Remove all occurence of char from string */
void str_remove(char *str, char c)
{
    char *src, *dest;

    src = dest = str;

    while(*src != '\0')
    {
        if (*src != c)
        {
            *dest = *src;
            dest++;
        }
        src++;
    }

    *dest = '\0';
}

/** Function to remove white spaces on both sides of a string i.e trim */
void str_trim(char *s)
{
    char *ptr;
    if (!s)
        return;   // handle NULL string
    if (!*s)
        return;      // handle empty string
    for (ptr = s + strlen(s) - 1; (ptr >= s) && isspace((int)*ptr); --ptr);
    ptr[1] = '\0';
}

/** Replace char in string */
void str_replace(char *str, char c1, char c2)
{
   while(*str != '\0')
   {
      if (*str == c1)
         *str = c2;

      str++;
    }
}

int str_replace_str(char *line, const char *search, const char *replace)
{
   int count;
   char *sp; // start of pattern

   //printf("replacestr(%s, %s, %s)\n", line, search, replace);
   if ((sp = strstr(line, search)) == NULL) {
      return(0);
   }
   count = 1;
   int sLen = strlen(search);
   int rLen = strlen(replace);
   if (sLen > rLen) {
      // move from right to left
      char *src = sp + sLen;
      char *dst = sp + rLen;
      while((*dst = *src) != '\0') { dst++; src++; }
   } else if (sLen < rLen) {
      // move from left to right
      int tLen = strlen(sp) - sLen;
      char *stop = sp + rLen;
      char *src = sp + sLen + tLen;
      char *dst = sp + rLen + tLen;
      while(dst >= stop) { *dst = *src; dst--; src--; }
   }
   memcpy(sp, replace, rLen);

   count += str_replace_str(sp + rLen, search, replace);

   return(count);
}

void str_tolower(char *str)
{
   for ( ; *str; ++str)
      *str = tolower((int)*str);
}

/** Check for string is number */
int str_is_number(char *str)
{
   for(; *str != '\0'; str++)
   {
      if (*str == '.')
         continue;

      if (!isdigit((int)*str))
         return -1;
   }

   return 0;
}

/**
 * Split string buffer to array
 * @param buf input string buffer
 * @param delim char delimiter
 * @param argv output argumets array
 * @param argv_size max number of args
 * @return number of arguments
 */
int split_line(char *buf, char delim, char **argv, int argv_size)
{
    int argc = 0;
    char *pp;

    if (buf != NULL)
    {
       // Remove '\n'
       if ((pp = strchr(buf, '\n')) != NULL)
           *pp = '\0';

       pp = buf;
       argv[argc++] = pp;

       while(argc < argv_size && (pp = strchr(pp, delim)) != NULL)
       {
           *pp++ = '\0';
           argv[argc++] = pp;
       }
    }

    return argc;
}

/** Read line from file */
int readline(int fd, char *buf, int bufsz)
{
   int res;
   int total = 0;

   while(bufsz > 0)
   {
      res = read(fd, buf, 1);
      if (!res)
         break;      // EOF
      else if (res < 0)
         return res; // error

      bufsz--;
      total++;
      if (*buf == '\n')
         break;

      buf++;
   }

   *buf = 0;

   return total;
}

// Converts a hexadecimal string to integer
int xtoi(const char* xs, unsigned int* result)
{
   size_t szlen = strlen(xs);
   int i, xv, fact;

   if (szlen > 0)
   {
      // Converting more than 32bit hexadecimal value?
      if (szlen>8) return 2; // exit

      // Begin conversion here
      *result = 0;
      fact = 1;

      // Run until no more character to convert
      for(i=szlen-1; i>=0 ; i--)
      {
         if (isxdigit((int)*(xs+i)))
         {
            if (*(xs+i)>=97)
            {
               xv = ( *(xs+i) - 97) + 10;
            }
            else if ( *(xs+i) >= 65)
            {
               xv = (*(xs+i) - 65) + 10;
            }
            else
            {
               xv = *(xs+i) - 48;
            }
            *result += (xv * fact);
            fact *= 16;
         }
         else
         {
            // Conversion was abnormally terminated
            // by non hexadecimal digit, hence
            // returning only the converted with
            // an error value 4 (illegal hex character)
            return 4;
         }
      }
   }

// Nothing to convert
   return 1;
}


const char *time2str(hal_time_t duration, char *buf, int bufsize)
{
   uint32_t msec = duration % 1000;
   uint32_t seconds = duration / 1000;
   uint32_t minutes = seconds / 60;
   uint32_t hours = minutes / 60;
   uint32_t days = hours / 24;

  if (days > 0)
    snprintf(buf, bufsize, "%dd %02d:%02d:%02d.%03d", days, hours % 24, minutes % 60, seconds % 60, msec);
   else
    snprintf(buf,  bufsize, "%02d:%02d:%02d.%03d", hours % 24, minutes % 60, seconds % 60, msec);

  return buf;
}

const char *unixtime2str(time_t duration, char *buf, int bufsize)
{
   uint32_t seconds = duration;
   uint32_t minutes = seconds / 60;
   uint32_t hours = minutes / 60;

   snprintf(buf,  bufsize, "%02d:%02d:%02d", hours % 24, minutes % 60, seconds % 60);

   return buf;
}



const char *buf2hex(uint8_t *inbuf, int inbufsize, char *outbuf, int outbufsize)
{
   char *pout = outbuf;

   while (inbufsize > 0 && outbufsize > 0)
   {
      byte2hex(*inbuf, pout);
      inbuf++;
      inbufsize--;
      outbufsize -= 2;
      pout += 2;
   }

   *pout = '\0';

   return outbuf;
}