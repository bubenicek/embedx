
#include "httpd.h"

//TRACE_TAG(httpd_cookie);
//#if !ENABLE_TRACE_HTTPD
//#undef TRACE
//#define TRACE(...)
//#endif


void httpd_prepare_cookie_data(httpd_connection_t *s)
{
  char *item;

  // make string array
  item = s->cookies;
  while ((item = strchr(item, ';')) != NULL)
  {
    *item++ = 0;
  }
}

char *httpd_get_cookie_value(httpd_connection_t *s, const char *name)
{
  char *item = s->cookies;
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

/** Set cookie value */
int httpd_set_cookie_value(struct httpd_connection *s, const char *name, const char *value)
{
   if (!s->output_cookies_length)
   {
      s->output_cookies_length = snprintf(s->cookies, sizeof(s->cookies), "Set-Cookie: %s=%s", name, value);
   }
   else
   {
      s->output_cookies_length += snprintf(&s->cookies[s->output_cookies_length], sizeof(s->cookies - s->output_cookies_length), "; %s=%s", name, value);
   }
   
   return 0;
}

