
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/httpd

PROJECT_SOURCEFILES += httpd.c
PROJECT_SOURCEFILES += httpd_cgi.c
PROJECT_SOURCEFILES += httpd_cookie.c
PROJECT_SOURCEFILES += httpd_file.c
PROJECT_SOURCEFILES += httpd_rest.c
PROJECT_SOURCEFILES += httpd_raw_socket.c
PROJECT_SOURCEFILES += httpd_ssl_socket.c

## JSON sax parser
include $(EMBEDX_ROOT)/core/lib/jsmn/Makefile.inc

