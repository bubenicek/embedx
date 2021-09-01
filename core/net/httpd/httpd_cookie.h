
#ifndef __HTTPD_COOKIE_H
#define __HTTPD_COOKIE_H

/** Prepare cookie data */
void httpd_prepare_cookie_data(struct httpd_connection *s);

/** Get cookie value */
char *httpd_get_cookie_value(struct httpd_connection *s, const char *name);

/** Set cookie value */
int httpd_set_cookie_value(struct httpd_connection *s, const char *name, const char *value);


#endif   