
#include "system.h"

#if defined(CFG_DEBUG) && (CFG_DEBUG == 1)

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
 #include <sys/time.h>
#include "lib/sprintf.h"

#ifndef CFG_TRACE_SIMPLE_PRINTF
#define CFG_TRACE_SIMPLE_PRINTF     0               // Use simple printf formating
#endif

#ifndef number_t
#define number_t long long
#endif

/* Prototypes */
static void trace_vprintf(const char *fmt, va_list args);

/* Locals */
#if defined (CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API  == 1)
static osMutexId trace_mutex_id;
#endif

/** Initialize trace */
int trace_init(void)
{
#if defined (CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API  == 1)
   if ((trace_mutex_id = osMutexCreate(NULL)) == NULL)
      return -1;
#endif

#if defined (CFG_FSLOG_ENABLED) && (CFG_FSLOG_ENABLED == 1)
   if (fslog_init() != 0)
      return -1;
#endif

   return 0;
}

inline void trace_putchar(char c)
{
   //if (c == '\n')
   //   hal_console_putchar('\r');

   hal_console_putchar(c);

#if defined (CFG_FSLOG_ENABLED) && (CFG_FSLOG_ENABLED == 1)
   fslog_putchar(c);
#endif

#if defined (CFG_MEMLOG_ENABLED) && (CFG_MEMLOG_ENABLED == 1)
	memlog_putchar(c);
#endif
}

/** Formated trace output */
#if defined (CFG_TRACE_SIMPLE_PRINTF) && (CFG_TRACE_SIMPLE_PRINTF == 1)
void trace_printf(const char *fmt, ...)
{
	va_list args;

#if defined (CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API  == 1)
	if (osKernelRunning())
   	    osMutexWait(trace_mutex_id, osWaitForever);
#endif

	va_start(args, fmt);
	TRACE_VPRINTF(fmt, args);
	va_end(args);

#if defined (CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API  == 1)
	if (osKernelRunning())
   	    osMutexRelease(trace_mutex_id);
#endif

   hal_console_flush();

#if defined (CFG_FSLOG_ENABLED) && (CFG_FSLOG_ENABLED == 1)
   if (fslog_size() >= CFG_FSLOG_ROTATE_FILESIZE)
      fslog_rotate();
#endif
}

#else

void trace_printf(const char *fmt, ...)
{
	va_list args;
    char buf[255];

#if defined (CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API  == 1)
	if (osKernelRunning())
   	    osMutexWait(trace_mutex_id, osWaitForever);
#endif

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

    for (char *pbuf = buf; *pbuf != '\0'; pbuf++)
    {
        trace_putchar(*pbuf);
    }

#if defined (CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API  == 1)
	if (osKernelRunning())
   	    osMutexRelease(trace_mutex_id);
#endif

   hal_console_flush();

#if defined (CFG_FSLOG_ENABLED) && (CFG_FSLOG_ENABLED == 1)
   if (fslog_size() >= CFG_FSLOG_ROTATE_FILESIZE)
      fslog_rotate();
#endif
}

#endif  // CFG_TRACE_SIMPLE_PRINTF


/** trace current uptime */
const char *trace_uptime(void)
{
  static char buf[48];
  return time2str(hal_time_ms(), buf, sizeof(buf));
}

/** Trace current systime */
const char *trace_systime(void)
{
  static char strtm[48];
  int msec;
  struct timeval curtime;
  struct tm *tm;

  gettimeofday(&curtime, NULL);
  tm = localtime(&curtime.tv_sec);
  msec = (hal_time_ms() % 1000);

  ee_snprintf(strtm, sizeof(strtm), "%02d/%02d/%d %02d:%02d:%02d.%03d", tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec, msec);

  return strtm;
}

/** Dump memory to trace */
void trace_dump(const void *buffer, int buff_len)
{
#define BYTES_PER_LINE  16
    int i;
    if ( buff_len == 0 ) return;
    const char *ptr_line;
    //format: field[length]
    // ADDR[10]+"   "+DATA_HEX[8*3]+" "+DATA_HEX[8*3]+"  |"+DATA_CHAR[8]+"|"
    char hd_buffer[10+3+BYTES_PER_LINE*3+3+BYTES_PER_LINE+1+1];
    char *ptr_hd;
    int bytes_cur_line;

    do {
        if ( buff_len > BYTES_PER_LINE ) {
            bytes_cur_line = BYTES_PER_LINE;
        } else {
            bytes_cur_line = buff_len;
        }

        ptr_line = buffer;
        ptr_hd = hd_buffer;

        ptr_hd += sprintf( ptr_hd, "%p ", buffer );
        for(i = 0; i < BYTES_PER_LINE; i ++ ) {
            if ( (i&7)==0 ) {
                ptr_hd += sprintf( ptr_hd, " " );
            }
            if ( i < bytes_cur_line ) {
                ptr_hd += sprintf( ptr_hd, " %02X", (uint8_t )ptr_line[i] );
            } else {
                ptr_hd += sprintf( ptr_hd, "   " );
            }
        }
        ptr_hd += sprintf( ptr_hd, "  |" );
        for(i = 0; i < bytes_cur_line; i ++ ) {
            if ( isprint((int)ptr_line[i]) ) {
                ptr_hd += sprintf( ptr_hd, "%c", ptr_line[i] );
            } else {
                ptr_hd += sprintf( ptr_hd, "." );
            }
        }
       ptr_hd += sprintf( ptr_hd, "|" );

        trace_printf("%s\n", hd_buffer);

        buffer += bytes_cur_line;
        buff_len -= bytes_cur_line;

    } while( buff_len );
}

static int do_div(number_t *n, unsigned int base)
{
   int res;
	res = *n % base;
	*n = *n / base;
	return res;
}


static int skip_atoi(const char **s)
{
	int i = 0;

	while (isdigit((int)**s)) {
		i = i*10 + *((*s)++) - '0';
	}
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */

static void number(number_t num, int base, int size, int precision, int type)
{
	char c,sign,tmp[66];
	const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
	int i;

	if (type & LARGE)
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16) {
			size -= 2;
		} else if (base == 8) {
			size--;
		}
	}
	i = 0;
	if (num == 0) {
		tmp[i++]='0';
	} else while (num != 0) {
		tmp[i++] = digits[do_div(&num, base)];
	}
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT))) {
		while(size-->0)
			trace_putchar(' ');
	}
	if (sign) {
		trace_putchar(sign);
	}

	if (type & SPECIAL) {
		if (base==8)
			trace_putchar('0');
		else if (base==16) {
			trace_putchar('0');
			trace_putchar(digits[33]);
		}
	}
	if (!(type & LEFT)) {
		while (size-- > 0)
			trace_putchar(c);
	}
	while (i < precision--)
		trace_putchar('0');
	while (i-- > 0)
		trace_putchar(tmp[i]);
	while (size-- > 0)
		trace_putchar(' ');
}

#if defined(CFG_TRACE_HAS_FLOAT) && (CFG_TRACE_HAS_FLOAT == 1)
#include <math.h>
#define CVTBUFSIZE 	255

static char *trace_cvt(double arg, int ndigits, int *decpt, int *sign, char *buf, int eflag)
{
	int r2;
	double fi, fj;
	char *p, *p1;

	if (ndigits < 0) ndigits = 0;
	if (ndigits >= CVTBUFSIZE - 1) ndigits = CVTBUFSIZE - 2;
	r2 = 0;
	*sign = 0;
	p = &buf[0];
	if (arg < 0) {
		*sign = 1;
		arg = -arg;
	}
	arg = modf(arg, &fi);
	p1 = &buf[CVTBUFSIZE];

	if (fi != 0) {
		p1 = &buf[CVTBUFSIZE];
		while (fi != 0)	{
				fj = modf(fi / 10, &fi);
				*--p1 = (int)((fj + .03) * 10) + '0';
				r2++;
		}
		while (p1 < &buf[CVTBUFSIZE]) *p++ = *p1++;
	} else if (arg > 0)	{
		while ((fj = arg * 10) < 1)	{
			arg = fj;
			r2--;
		}
	}
	p1 = &buf[ndigits];
	if (eflag == 0) p1 += r2;
	*decpt = r2;
	if (p1 < &buf[0]) {
		buf[0] = '\0';
		return buf;
	}
	while (p <= p1 && p < &buf[CVTBUFSIZE]) {
		arg *= 10;
		arg = modf(arg, &fj);
		*p++ = (int) fj + '0';
	}
	if (p1 >= &buf[CVTBUFSIZE]) {
		buf[CVTBUFSIZE - 1] = '\0';
		return buf;
	}
	p = p1;
	*p1 += 5;
	while (*p1 > '9') {
		*p1 = '0';
		if (p1 > buf) {
			++*--p1;
		} else {
			*p1 = '1';
			(*decpt)++;
			if (eflag == 0) {
				if (p > buf) *p = '0';
				p++;
			}
		}
	}
	*p = '\0';
	return buf;
}

static char *trace_ecvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf)
{
  return trace_cvt(arg, ndigits, decpt, sign, buf, 1);
}

static char *trace_fcvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf)
{
  return trace_cvt(arg, ndigits, decpt, sign, buf, 0);
}

static void ee_bufcpy(char *pd, char *ps, int count) {
	char *pe=ps+count;
	while (ps!=pe)
		*pd++=*ps++;
}

static void parse_float(double value, char *buffer, char fmt, int precision)
{
	int decpt, sign, exp, pos;
	char *fdigits = NULL;
	char cvtbuf[CVTBUFSIZE];
	int capexp = 0;
	int magnitude;

	if (fmt == 'G' || fmt == 'E') {
		capexp = 1;
		fmt += 'a' - 'A';
	}

	if (fmt == 'g') {
		fdigits = trace_ecvtbuf(value, precision, &decpt, &sign, cvtbuf);
		magnitude = decpt - 1;
		if (magnitude < -4  ||  magnitude > precision - 1) {
			fmt = 'e';
			precision -= 1;
		} else {
			fmt = 'f';
			precision -= decpt;
		}
	}

	if (fmt == 'e') {
		fdigits = trace_ecvtbuf(value, precision + 1, &decpt, &sign, cvtbuf);

		if (sign) *buffer++ = '-';
		*buffer++ = *fdigits;
		if (precision > 0) *buffer++ = '.';
		ee_bufcpy(buffer, fdigits + 1, precision);
		buffer += precision;
		*buffer++ = capexp ? 'E' : 'e';

		if (decpt == 0) {
			if (value == 0.0)
				exp = 0;
			else
				exp = -1;
		} else
			exp = decpt - 1;

		if (exp < 0) {
			*buffer++ = '-';
			exp = -exp;
		} else
			*buffer++ = '+';

		buffer[2] = (exp % 10) + '0';
		exp = exp / 10;
		buffer[1] = (exp % 10) + '0';
		exp = exp / 10;
		buffer[0] = (exp % 10) + '0';
		buffer += 3;
	} else if (fmt == 'f') {
		fdigits = trace_fcvtbuf(value, precision, &decpt, &sign, cvtbuf);
		if (sign) *buffer++ = '-';
		if (*fdigits) {
			if (decpt <= 0) {
				*buffer++ = '0';
				*buffer++ = '.';
				for (pos = 0; pos < -decpt; pos++) *buffer++ = '0';
				while (*fdigits) *buffer++ = *fdigits++;
			} else {
				pos = 0;
				while (*fdigits) {
						if (pos++ == decpt) *buffer++ = '.';
						*buffer++ = *fdigits++;
				}
			}
		} else {
			*buffer++ = '0';
			if (precision > 0) {
				*buffer++ = '.';
				for (pos = 0; pos < precision; pos++) *buffer++ = '0';
			}
		}
	}

	*buffer = '\0';
}

static void decimal_point(char *buffer)
{
	while (*buffer) {
		if (*buffer == '.') return;
		if (*buffer == 'e' || *buffer == 'E') break;
		buffer++;
	}

	if (*buffer) {
		int n = strnlen(buffer, CVTBUFSIZE);
		while (n > 0) {
			buffer[n + 1] = buffer[n];
			n--;
		}

		*buffer = '.';
	} else {
		*buffer++ = '.';
		*buffer = '\0';
	}
}

static void cropzeros(char *buffer)
{
	char *stop;

	while (*buffer && *buffer != '.') buffer++;
	if (*buffer++) {
		while (*buffer && *buffer != 'e' && *buffer != 'E') buffer++;
		stop = buffer--;
		while (*buffer == '0') buffer--;
		if (*buffer == '.') buffer--;
		while (buffer!=stop)
			*++buffer=0;
	}
}

static void flt(double num, int size, int precision, char fmt, int flags)
{
	char tmp[CVTBUFSIZE];
	char c, sign;
	int n, i;

	/* Left align means no zero padding */
	if (flags & LEFT) flags &= ~ZEROPAD;

	/* Determine padding and sign char */
	c = (flags & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (flags & SIGN) {
		if (num < 0.0) {
			sign = '-';
			num = -num;
			size--;
		} else if (flags & PLUS) {
			sign = '+';
			size--;
		} else if (flags & SPACE) {
			sign = ' ';
			size--;
		}
	}

	/* Compute the precision value */
	if (precision < 0)
		precision = 6; // Default precision: 6

	/* Convert floating point number to text */
	parse_float(num, tmp, fmt, precision);

	if ((flags & SPECIAL) && precision == 0) decimal_point(tmp);
	if (fmt == 'g' && !(flags & SPECIAL)) cropzeros(tmp);

	n = strnlen(tmp, CVTBUFSIZE);

	/* Output number with alignment and padding */
	size -= n;
	if (!(flags & (ZEROPAD | LEFT))) while (size-- > 0) trace_putchar(' ');
	if (sign) trace_putchar(sign);
	if (!(flags & LEFT)) while (size-- > 0) trace_putchar(c);
	for (i = 0; i < n; i++) trace_putchar(tmp[i]);
	while (size-- > 0) trace_putchar(' ');
}
#endif  /* CFG_TRACE_HAS_FLOAT */



/* Find the length of S, but scan at most MAXLEN characters.  If no '\0'
   terminator is found within the first MAXLEN characters, return MAXLEN. */
static int trace_strnlen(const char *s, int count)
{
  const char *sc;
  for (sc = s; *sc != '\0' && count--; ++sc);
  return sc - s;
}

static void trace_vprintf(const char *fmt, va_list args)
{
	int len;
	number_t num;
	int i, base;
	const char *s;
	int flags;		    /* flags to number() */
	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
						   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */
							/* 'z' support added 23/7/1999 S.H.    */
							/* 'z' changed to 'Z' --davidm 1/25/99 */

	for (; *fmt ; ++fmt) {
		if (*fmt != '%') {
			trace_putchar(*fmt);
			continue;
		}

		/* Process flags */
		flags = 0;
repeat:
		++fmt;		/* this also skips first '%' */
		switch (*fmt) {
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		}

		/* Get field width */
		field_width = -1;
		if (isdigit((int)*fmt)) {
			field_width = skip_atoi(&fmt);
		} else if (*fmt == '*') {
			++fmt;
			/* It's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* Get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (isdigit((int)*fmt)) {
				precision = skip_atoi(&fmt);
			} else if (*fmt == '*') {
				++fmt;
				/* It's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* Get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'H' || *fmt == 'l' || *fmt == 'L' || *fmt =='Z') {
			qualifier = *fmt++;
			if (*fmt == 'l') {
				qualifier = 'L';
				fmt++;
			}
			else if (*fmt == 'h') {
            // signed/unsigned char
				qualifier = 'H';
				fmt++;
			}
		}

		/* Default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT)) {
				while (--field_width > 0)
					trace_putchar(' ');
			}
			trace_putchar((unsigned char) va_arg(args, int));
			while (--field_width > 0)
				trace_putchar(' ');
			continue;

		case 's':
			s = va_arg(args, char *);
			if (!s)
				s = "<NULL>";

			len = trace_strnlen(s, precision);

			if (!(flags & LEFT)) {
				while (len < field_width--)
					trace_putchar(' ');
			}
			for (i = 0; i < len; ++i)
				trace_putchar(*s++);
			while (len < field_width--)
				trace_putchar(' ');
			continue;

		case 'p':
			if (field_width == -1) {
				field_width = 2*sizeof(void *);
				flags |= ZEROPAD;
			}
			number((unsigned long) va_arg(args, void *), 16, field_width, precision, flags);
			continue;

		case '%':
			trace_putchar('%');
			continue;

		/* Integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
				break;

#if defined(CFG_TRACE_HAS_FLOAT) && (CFG_TRACE_HAS_FLOAT == 1)
		case 'G':
		case 'E':
		case 'g':
		case 'e':
		case 'f':
		flt(va_arg(args, double), field_width, precision, *fmt, flags | SIGN);
      continue;
#endif

		default:
			trace_putchar('%');
			if (*fmt) {
				trace_putchar(*fmt);
			} else {
				--fmt;
			}
			continue;
		}

		if (qualifier == 'L') {
			num = va_arg(args, number_t);
		} else if (qualifier == 'l') {
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long) num;
		} else if (qualifier == 'Z') {
			num = va_arg(args, size_t);
		} else if (qualifier == 'h') {
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (signed short) num;
		} else if (qualifier == 'H') {
#ifdef __GNUC_VA_LIST
			num = (unsigned char) va_arg(args, int);
#else
			num = (unsigned char) va_arg(args, unsigned char);
#endif // __GNUC_VA_LIST
			if (flags & SIGN)
				num = (signed char) num;
		} else {
			num = va_arg(args, unsigned int);
			if (flags & SIGN)
				num = (signed int) num;
		}

		number(num, base, field_width, precision, flags);
	}
}

#endif    /* CFG_DEBUG */
