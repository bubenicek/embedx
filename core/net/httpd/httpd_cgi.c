
#include "httpd.h"

//TRACE_TAG(httpd_cgi);
//#if !ENABLE_TRACE_HTTPD_CGI
//#undef TRACE
//#define TRACE(...)
//#endif


// Prototypes:
static char httpd_from_hex(char c);

// external vars
extern const httpd_cgi_call_t httpd_cgicalls[];


/**
 * ziskani CGI funkce podle nazvu
 * @return pokud existuje tak ptr na CGI funkce jinak NULL
 */
httpd_cgifunction_t httpd_get_cgi_function(httpd_connection_t *s, char *name)
{
  const httpd_cgi_call_t *cgicall = httpd_cgicalls;

  while(cgicall->name)
  {
    if (!strcmp(cgicall->name, name))
      return cgicall->function;

    cgicall++;
  }

  // funcion not exist
  return NULL;
}

void httpd_prepare_input_data(httpd_connection_t *s, char *data)
{
  char *item;

  // free previous allocated input buffer
  if (s->input_data != NULL && s->input_data != data)
  {
    os_free(s->input_data);
  }

  // url encoding
  httpd_unescape(data);

  // make string array
  item = s->input_data = data;
  while ((item = strchr(item, '&')) != NULL)
  {
    *item++ = 0;
  }
}

char *httpd_get_param_value(httpd_connection_t *s, const char *name)
{
  char *item = s->input_data;
  char *pval;

  while (item && *item)
  {
    if (strncmp(item, name, strlen(name)) == 0)
    {
      if ((pval = strchr(item, '=')) != NULL)
        item = ++pval;

      return item;
    }

    // next item
    item += strlen(item) + 1;
  }

  return NULL;
}

int httpd_decode(char *bufcoded, char *bufplain, int outbufsize)
{
  static char six2pr[64] =
  {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
  };

  static unsigned char pr2six[256];

  /* single character decode */
#	define DEC(c) pr2six[(int)c]
#	define _DECODE_MAXVAL 63

  static int first = 1;

  int nbytesdecoded, j;
  char *bufin = bufcoded;
  unsigned char *bufout = (uint8_t *)bufplain;
  int nprbytes;

  /*
  ** If this is the first call, initialize the mapping table.
  ** This code should work even on non-ASCII machines.
  */
  if(first)
  {
    first = 0;
    for(j=0; j<256; j++) pr2six[j] = _DECODE_MAXVAL+1;
    for(j=0; j<64; j++) pr2six[(int)six2pr[j]] = (unsigned char)j;
  }

  /* Strip leading whitespace. */

  while(*bufcoded==' ' || *bufcoded == '\t') bufcoded++;

  /*
  ** Figure out how many characters are in the input buffer.
  ** If this would decode into more bytes than would fit into
  ** the output buffer, adjust the number of input bytes downwards.
  */
  bufin = bufcoded;
  while(pr2six[(int)*(bufin++)] <= _DECODE_MAXVAL);
  nprbytes = bufin - bufcoded - 1;
  nbytesdecoded = ((nprbytes+3)/4) * 3;
  if(nbytesdecoded > outbufsize)
  {
    nprbytes = (outbufsize*4)/3;
  }
  bufin = bufcoded;

  while (nprbytes > 0)
  {
    *(bufout++)=(unsigned char)(DEC(*bufin)<<2|DEC(bufin[1])>>4);
    *(bufout++)=(unsigned char)(DEC(bufin[1])<<4|DEC(bufin[2])>>2);
    *(bufout++)=(unsigned char)(DEC(bufin[2])<<6|DEC(bufin[3]));
    bufin += 4;
    nprbytes -= 4;
  }
  if(nprbytes & 03)
  {
    if(pr2six[(int)bufin[-2]] > _DECODE_MAXVAL)
    {
      nbytesdecoded -= 2;
    }
    else
    {
      nbytesdecoded -= 1;
    }
  }
  bufplain[nbytesdecoded] = 0;
  return(nbytesdecoded);
}

static char httpd_from_hex (char c)
{
  return  c >= '0' && c <= '9' ?  c - '0'
          : c >= 'A' && c <= 'F'? c - 'A' + 10
          : c - 'a' + 10;     /* accept small letters just in case */
}

char * httpd_unescape(char *str)
{
  char * p = str;
  char * q = str;
  static char blank[] = "";

  if (!str)
    return(blank);
  while(*p)
  {
    if (*p == '%')
    {
      p++;
      if (*p) *q = httpd_from_hex(*p++) * 16;
      if (*p) *q = (*q + httpd_from_hex(*p++));
      q++;
    }
    else
    {
      if (*p == '+')
      {
        *q++ = ' ';
        p++;
      }
      else
      {
        *q++ = *p++;
      }
    }
  }

  *q++ = 0;
  return str;
}

void httpd_str_remove_char(char *str, char c)
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
