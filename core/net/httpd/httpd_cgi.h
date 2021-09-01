
#ifndef __HTTPD_CGI_H__
#define __HTTPD_CGI_H__

// forwarde decl.
struct httpd_connection;

/** CGI function type */
typedef void(*httpd_cgifunction_t)(struct httpd_connection *, char *);

/** CGI function call */
typedef struct httpd_cgi_call
{
  const char *name;
  const httpd_cgifunction_t function;

} httpd_cgi_call_t;


/** Get function handler */
httpd_cgifunction_t httpd_get_cgi_function(struct httpd_connection *s, char *name);

/** Prepare input data, URL escape encoding, make string array */
void httpd_prepare_input_data(struct httpd_connection *s, char *data);

/** Prepare output data */
#define httpd_prepare_output_data httpd_prepare_input_data

/** Get input parameter value by variable name */
char *httpd_get_param_value(struct httpd_connection *s, const char *name);

/** Decode from http url encoding */
int httpd_decode(char *bufcoded, char *bufplain, int outbufsize);

char *httpd_unescape(char *str);

/** Remove chars from string */
void httpd_str_remove_char(char *str, char c);

#endif      // __HTTPD_CGI_H__

