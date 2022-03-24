/**
 * \file httpd.c   \brief Embedded HTTP server
 */

#include "httpd.h"

TRACE_TAG(httpd);
#if !ENABLE_TRACE_HTTPD
#undef TRACE
#define TRACE(...)
#endif

// Prototypes:
static void httpd_thread(void *arg);
static void httpd_handle_connection(void *arg);
static int httpd_handle_input(httpd_connection_t *s);
static int httpd_handle_output(httpd_connection_t *s);
static int httpd_handle_script(httpd_connection_t *s, httpd_file_t *fr);
static int httpd_handle_file(httpd_connection_t *con, const char *filename);


// Locals:
static const osThreadDef(HTTPD, httpd_thread, CFG_HTTPD_THREAD_PRIORITY, 0, CFG_HTTPD_LISTENER_THREAD_STACK_SIZE);
static const osThreadDef(HTTPD_CON, httpd_handle_connection, CFG_HTTPD_THREAD_PRIORITY, 0, CFG_HTTPD_THREAD_STACK_SIZE);


int httpd_init(httpd_t *s, uint16_t port)
{
    memset(s, 0, sizeof(httpd_t));
    s->port = port;

    if ((s->consem = osSemaphoreCreate(NULL, CFG_HTTPD_MAXNUM_CONNECTIONS)) == NULL)
    {
        TRACE_ERROR("Create max connections semaphore");
        goto fail_sem;
    }

    // Create listening socket
    if ((s->sd = httpd_socket_create(s->port)) < 0)
    {
        TRACE_ERROR("Create HTTP listening socket");
        goto fail_socket;
    }

    // Create listening thread
    if ((s->thread = osThreadCreate(osThread(HTTPD), s)) == 0)
    {
        TRACE_ERROR("Create httpd thread");
        goto fail_thread;
    }

    TRACE("HTTP server initialized, port: %d  max_connections: %d", s->port, CFG_HTTPD_MAXNUM_CONNECTIONS);

    return 0;

fail_thread:
    httpd_socket_close(s->sd);
fail_socket:
fail_sem:
    return -1;
}

int httpd_deinit(httpd_t *s)
{
    return httpd_socket_close(s->sd);
}

static void httpd_thread(void *arg)
{
    int newsd;
    struct sockaddr_in remote_addr;
    httpd_connection_t *con;
    httpd_t *s = arg;

    while (1)
    {
        // Accept connection only on raw socket
        if ((newsd = httpd_socket_accept(s->sd, &remote_addr)) < 0)
        {
            TRACE_ERROR("accept error, errno: %d", errno);
            continue;
        }

        // Wait for active connection
        if (osSemaphoreWait(s->consem, osWaitForever) != osOK)
        {
            TRACE_ERROR("Wait for maxcon sem failed");
            httpd_socket_close(newsd);
            continue;
        }
       
        // Alloc connection context
        if ((con = os_malloc(sizeof(httpd_connection_t))) != NULL)
        {
            memset(con, 0, sizeof(httpd_connection_t));
            con->start_tm = hal_time_ms();
            con->httpd = s;
            con->sd = newsd;
            con->remote_addr = remote_addr;

            // Create connection thread
            if ((con->thread = osThreadCreate(osThread(HTTPD_CON), con)) == 0)
            {
                TRACE_ERROR("Create httpd thread");
                httpd_socket_close(con->sd);
                osSemaphoreRelease(s->consem);
                os_free(con);
            }
        }
        else 
        {
            TRACE_ERROR("Alloc new httpd connection");
            httpd_socket_close(newsd);
            osSemaphoreRelease(s->consem);
        }
    }
}

static void httpd_handle_connection(void *arg)
{
    httpd_connection_t *s = (httpd_connection_t *)arg;

    TRACE("Start connection[%d] - %s", s->sd, inet_ntoa(s->remote_addr.sin_addr));

    // Handle input and output
    if (httpd_handle_input(s) == 0)
        httpd_handle_output(s);

    // Free alocated connection memory
    if (s->input_data != NULL)
        os_free(s->input_data);

    TRACE("Finished connection[%d] - %s  tmlen: %d (ms)   min_stack: %d", 
        s->sd, inet_ntoa(s->remote_addr.sin_addr), (int)(hal_time_ms() - s->start_tm), osMemGetStackHighWatterMark());

    // Close socket
    httpd_socket_close(s->sd);

    osSemaphoreRelease(s->httpd->consem);
    os_free(s);

    osThreadTerminate(osThreadGetId());
}


/** Handle HTTP input */
static int httpd_handle_input(httpd_connection_t *s)
{
    int res, count = 0;
    char *pbuf;
    char *params = NULL;

    s->content_type = HTTPD_CONTENT_TYPE_PLAIN;
    s->content_length = 0;
    s->input_data = NULL;

    // Read first request line
    if (httpd_readto(s, s->buffer, sizeof(s->buffer), ISO_SPACE) <= 0)
        return -1;

    // Get request method
    if (strncmp(s->buffer, HTTP_GET, 4) == 0)
    {
        s->reqtype = HTTPD_REQ_TYPE_GET;
    }
    else if (strncmp(s->buffer, HTTP_POST, 5) == 0)
    {
        s->reqtype = HTTPD_REQ_TYPE_POST;
    }
    else if (strncmp(s->buffer, HTTP_PUT, 4) == 0)
    {
        s->reqtype = HTTPD_REQ_TYPE_PUT;
    }
    else if (strncmp(s->buffer, HTTP_DELETE, 7) == 0)
    {
        s->reqtype = HTTPD_REQ_TYPE_DELETE;
    }
    else
    {
        // Not supported request
        TRACE_ERROR("Not supported request '%s'", s->buffer);
        return -1;
    }

    // Read tail of command line
    if ((res = httpd_readto(s, s->buffer, sizeof(s->buffer), ISO_SPACE)) <= 0)
        return -1;

    s->buffer[res-1] = '\0';

    // Test first slash
    if (s->buffer[0] != ISO_SLASH)
        return -1;

    // Read request params
    if ((params = strchr(s->buffer, '?')) != NULL)
    {
        *params++ = '\0';

        if ((s->input_data = os_malloc(strlen(params) + 1)) == NULL)
        {
            TRACE_ERROR("httpd alloc param");
            return -1;
        }

        strcpy(s->input_data, params);
        httpd_prepare_input_data(s, s->input_data);
    }

    // Make filename path
    if (!strcmp(s->buffer, "/"))
        snprintf(s->filename, sizeof(s->filename), "%s/%s", CFG_HTTPD_WWW_ROOT_DIR, HTTP_INDEX_HTML);
    else if (!strcmp(s->buffer, "//"))
        snprintf(s->filename, sizeof(s->filename), "%s", CFG_HTTPD_WWW_ROOT_DIR);
    else
        snprintf(s->filename, sizeof(s->filename), "%s%s", CFG_HTTPD_WWW_ROOT_DIR, s->buffer);

   // Replace char '-' -> '_'
   for (pbuf = s->filename; *pbuf != '\0'; pbuf++)
   {
      if (*pbuf == '-')
         *pbuf = '_';
   }

    TRACE("Request type: %d  url: %s", s->reqtype, s->filename);

    // Parse HTTP header
    while(1)
    {
        if ((res = httpd_readto(s, s->buffer, sizeof(s->buffer), ISO_NL)) <= 0)
            return -1;

        if (s->buffer[0] == '\r' && s->buffer[1] == '\n')
            break;

        if (strncmp(s->buffer, "Authorization: Basic ", 21) == 0)
        {
            // decode and save authorization string
            s->buffer[res - 2] = '\0';
            httpd_decode(&s->buffer[21], s->authorization, sizeof(s->authorization));
        }
        else if (strncasecmp(s->buffer, HTTP_CONTENT_TYPE_WWW_FORM, 47) == 0)
        {
            s->content_type = HTTPD_CONTENT_TYPE_WWWFORM_ENCODED;
        }
        else if (strncasecmp(s->buffer, HTTP_CONTENT_LENGTH, 16) == 0)
        {
            s->content_length = count = atoi(&s->buffer[16]);
            TRACE("Content_length: %d", count);
        }
        else if (strncasecmp(s->buffer, "Cookie: ", 8) == 0)
        {
            s->buffer[res - 2] = '\0';
            strncpy(s->cookies, &s->buffer[8], sizeof(s->cookies));
            httpd_prepare_cookie_data(s);
        }
        else if (strncasecmp(s->buffer, "Connection: Keep-Alive", 22) == 0)
        {
            s->keep_connection = 1;
        }
        else if (strncasecmp(s->buffer, "X-Atmosphere-Transport: long-polling", 36) == 0)
        {
            s->longpolling = 1;
            TRACE("Long-pooling enabled");
        }
    }

/* RBU disabled
    if (s->content_type == HTTPD_CONTENT_TYPE_WWWFORM)
    {
        /////////////////////////
        // WWW form content type

        TRACE("POST WWW-FORM");

        if (count > 0)
        {
            if (count > CFG_HTTPD_MAX_POST_WWFORM_SIZE)
            {
                TRACE_ERROR("POST WWWFORM content-length exceeded");
                return -1;
            }

            if ((s->input_data = os_malloc(count + 1)) == NULL)
            {
                TRACE_ERROR("Alloc post data size: %d", count+1);
                return -1;
            }
            pbuf = s->input_data;

            // Read post data
            while (count > 0)
            {
                int len = count > sizeof(s->buffer) ? sizeof(s->buffer) : count;

                if ((res = httpd_socket_recv(s->sd, pbuf, len)) <= 0)
                    return -1;

                pbuf += res;
                *pbuf = '\0';
                count -= res;

                TRACE("Read %d bytes, count: %d", res, count);
            }

            httpd_prepare_input_data(s, s->input_data);
        }
    }
*/

    return 0;
}

/** Handle HTTP output */
static int httpd_handle_output(httpd_connection_t *s)
{
    int res;
    char *ptr;
    httpd_file_t fr;
    httpd_cgifunction_t cgi_func;
    httpd_rest_function_t rest_func;
    const httpd_rest_call_t *rest_call;

#if defined(CFG_HTTPD_USE_BASIC_AUTHORIZATION) && (CFG_HTTPD_USE_BASIC_AUTHORIZATION == 1)
    if (*s->authorization == 0)
    {
        httpd_send_headers(s, HTTP_HEADER_401_BASIC);
        return 0;
    }
    else
    {
        // authorize user
        char *passwd = strchr(s->authorization, ':');
        if (passwd == NULL)
        {
            // authorization string is not valid format
            TRACE_ERROR("Not valid authorization string '%s'", s->authorization);
            httpd_send_headers(s, HTTP_HEADER_401_BASIC);
            return -1;
        }
        *passwd++ = 0;

        // Proved autorizaci
        if (!httpd_basic_authorize(s->authorization, passwd))
        {
            TRACE_ERROR("Authorization failed, user='%s'  passwd='%s'", s->authorization, passwd);
            httpd_send_headers(s, HTTP_HEADER_401_BASIC);
            return -1;
        }

        TRACE("Authorization success, user='%s' password='%s'", s->authorization, passwd);
    }
#endif

    if (httpd_file_open(&fr, s->filename) != 0)
    {
        int argc = 0;
        const char *argv[CFG_HTTPD_REST_MAX_PARAMS];

        // File not found, when exists cgi or rest function then invoke it
        if ((cgi_func = httpd_get_cgi_function(s, s->filename[0] == '/' ? s->filename+1: s->filename)) != NULL)
        {
            cgi_func(s, s->filename);
        }
        else if ((rest_call = httpd_get_rest_call(s, s->filename, argv, &argc)) != NULL)
        {
            switch(s->reqtype)
            {
            case HTTPD_REQ_TYPE_GET:
                rest_func = rest_call->get_func;
                break;
            case HTTPD_REQ_TYPE_POST:
                rest_func = rest_call->insert_func;
                break;
            case HTTPD_REQ_TYPE_PUT:
                rest_func = rest_call->update_func;
                break;
            case HTTPD_REQ_TYPE_DELETE:
                rest_func = rest_call->delete_func;
                break;
            default:
                rest_func = NULL;
            }

            if (rest_func != NULL)
            {
#if CFG_HTTPD_USE_RESTAPI_AUTHORIZATION
                if (httpd_rest_authorize(s, rest_call) == 0)
#else
                if (1)
#endif
                {
                    if ((res = rest_func(s, rest_call, argv, argc)) == 0)
                    {
                        switch(s->reqtype)
                        {
                        case HTTPD_REQ_TYPE_POST:
                        case HTTPD_REQ_TYPE_PUT:
                        case HTTPD_REQ_TYPE_DELETE:
                            httpd_send_headers(s, HTTP_HEADER_200);
                            break;
                        default:
                            break;
                        }
                    }
                    else
                    {
                        TRACE_ERROR("Exec REST function '%s' type: %d", s->filename, s->reqtype);
                        if (res == REST_API_ERR_NOTFOUND)
                            httpd_send_headers(s, HTTP_HEADER_404);
                        else if (res == REST_API_ERR_FORMAT)
                            httpd_send_headers(s, HTTP_HEADER_400);
                        else
                            httpd_send_headers(s, HTTP_HEADER_500);
                    }
                }
                else
                {
                    TRACE_ERROR("Authorize REST function '%s'", s->filename);
                    httpd_send_headers(s, HTTP_HEADER_401);
                }
            }
            else
            {
                TRACE_ERROR("Method: %d not alowed for REST function '%s'", s->reqtype, s->filename);
                httpd_send_headers(s, HTTP_HEADER_405);
            }
        }
        else if (httpd_handle_file(s, s->filename) != 0)
        {
            httpd_send_headers(s, HTTP_HEADER_404);
        }
    }
    else
    {
#if CFG_HTTPD_GZIP_FILE_CONTENT
         httpd_send_headers(s, HTTP_HEADER_200_GZIP);
#else
         httpd_send_headers(s, HTTP_HEADER_200);
#endif
        ptr = strchr(s->filename, ISO_PERIOD);
        if (ptr != NULL && !strncmp(ptr, HTTP_HTML, 6))
        {
            httpd_handle_script(s, &fr);
        }
        else
        {
            // Send file
            while ((res = httpd_file_read(&fr, s->buffer, sizeof(s->buffer))) > 0)
            {
                httpd_socket_send(s->sd, s->buffer, res);
            }
        }

        httpd_file_close(&fr);
    }

    return 0;
}

/** Handle script in the file */
static int httpd_handle_script(httpd_connection_t *s, httpd_file_t *fr)
{
    int len;
    char *pline;
    char *pscript;
    char *scriptName;
    char *pvarValue;
    httpd_file_t fr2;
    httpd_cgifunction_t cgiFunction;
    char line[HTTPD_LINE_SIZE];

    while ((len = httpd_file_getline(fr, line, sizeof(line))) > 0)
    {
        pline = line;

        // Pokud je v radlu tag <% tak ho zpracuj, jinak uloz do bufferu pro odeslani
        if ((pscript = strstr(pline, "<%")) != NULL)
        {
            do
            {
                // Pokud nezacina <% od zacatku radku, odesli data pred <%
                len = pscript - pline;
                if (len > 0)
                {
                    httpd_socket_send(s->sd, pline, len);
                }

                scriptName = pscript + 2;  // skip '<%'

                // Find end of script TAG %>
                if ((pscript = strstr(pscript, "%>")) != NULL)
                {
                    pline = pscript + 2;                      // skip %>
                    *pscript = 0;                             // terminate scriptame (end of %>)
                    httpd_str_remove_char(scriptName, ' ');   // Remove spaces
                }
                else
                {
                    TRACE_ERROR("Not found end of TAG");
                    continue;
                }

                if (*scriptName == ':')
                {
                    //////////////////
                    // included file

                    scriptName++;   // skip ':'

                    TRACE("Include html '%s'", scriptName);

                    if (httpd_file_open(&fr2, scriptName) == 0)
                    {
                        httpd_handle_script(s, &fr2);
                        httpd_file_close(&fr2);
                    }
                    else
                    {
                        TRACE_ERROR("Can't open file '%s'", scriptName);
                        httpd_send_headers(s, HTTP_HEADER_404);
                        return -1;
                    }
                }
                else if (*scriptName == '!')
                {
                    //////////////
                    // CGI script

                    scriptName++;     // skip '!'

                    TRACE("CGI script '%s'", scriptName);

                    // get CGI function by name when exists
                    cgiFunction = httpd_get_cgi_function(s, scriptName);
                    if (cgiFunction != NULL)
                    {
                        cgiFunction(s, scriptName);
                    }
                }
                else
                {
                    /////////////////////////////
                    // insert variable content

                    TRACE("CGI variable '%s'", scriptName);

                    pvarValue = httpd_get_param_value(s, scriptName);
                    len = strlen(pvarValue);
                    if (len > 0)
                    {
                        httpd_socket_send(s->sd, pvarValue, len);
                    }
                }
            }
            while ((pscript = strstr(pline, "<%")) != NULL);

            // odesli zbytek do konce radku
            len = strlen(pline);
            if (len > 0)
            {
                httpd_socket_send(s->sd, pline, len);
            }
        }
        else
        {
            // send read line
            httpd_socket_send(s->sd, line, len);
        }
    }

    return 0;
}

static int httpd_handle_file(httpd_connection_t *con, const char *filename)
{
    int fd = -1;
    int len;

    switch(con->reqtype)
    {
    case HTTPD_REQ_TYPE_GET:
    {
        struct stat st;

        if (stat(filename, &st) != 0)
        {
            TRACE_ERROR("Open file/dir %s", filename);
            goto fail;
        }

        if (S_ISDIR(st.st_mode))
        {
            int res;
            DIR *dir;
            struct dirent *dirent;

            if ((dir = opendir(filename)) == NULL)
            {
                TRACE_ERROR("Open dir %s", filename);
                goto fail;
            }

            httpd_send_headers(con, HTTP_HEADER_200);

            while ((dirent = readdir(dir)) != NULL)
            {
                if (dirent->d_type == 1)
                {
                    if (filename[strlen(filename)-1] == '/')
                        snprintf(con->buffer, sizeof(con->buffer), "%s%s", filename, dirent->d_name);
                    else
                        snprintf(con->buffer, sizeof(con->buffer), "%s/%s", filename, dirent->d_name);

                    if (stat(con->buffer, &st) == 0)
                    {
                        res = snprintf(con->buffer, sizeof(con->buffer), "%-30s %lu\n", dirent->d_name, st.st_size);
                        httpd_send(con, con->buffer, res);
                    }
                }
                else
                {
                    res = snprintf(con->buffer, sizeof(con->buffer), "[%s]\n", dirent->d_name);
                    httpd_send(con, con->buffer, res);
                }
            }

            closedir(dir);
        }
        else
        {
            fd = open(filename, O_RDONLY, 0);
            if (fd < 0)
            {
                TRACE_ERROR("Open file %s", filename);
                goto fail;
            }

#if CFG_HTTPD_GZIP_FILE_CONTENT
            httpd_send_headers(con, HTTP_HEADER_200_GZIP);
#else
            httpd_send_headers(con, HTTP_HEADER_200);
#endif
            while((len = read(fd, con->buffer, sizeof(con->buffer))) > 0)
            {
                httpd_send(con, con->buffer, len);
            }

            close(fd);
        }
    }
    break;

    case HTTPD_REQ_TYPE_POST:
    case HTTPD_REQ_TYPE_PUT:
    {
        if (con->reqtype == HTTPD_REQ_TYPE_POST && con->content_length  == 0)
        {
            if (mkdir(filename, 0755) != 0)
            {
                TRACE_ERROR("Create dir %s", filename);
                goto fail;
            }

            TRACE("Create dir %s", filename);
        }
        else
        {
            if ((fd = creat(filename, 0644)) < 0)
            {
                TRACE_ERROR("Create file %s", filename);
                goto fail;
            }

            // Write file
            while(con->content_length > 0 && (len = httpd_socket_recv(con->sd, con->buffer, sizeof(con->buffer))) > 0)
            {
                if (write(fd, con->buffer, len) != len)
                {
                    TRACE_ERROR("Write file");
                    goto fail;
                }

                con->content_length -= len;
            }

            close(fd);
            TRACE("Create file %s", filename);
        }

        httpd_send_headers(con, HTTP_HEADER_200);
    }
    break;

    case HTTPD_REQ_TYPE_DELETE:
    {
        struct stat st;

        if (stat(filename, &st) != 0)
        {
            TRACE_ERROR("Delete file/dir %s", filename);
            goto fail;
        }

        if (S_ISDIR(st.st_mode))
        {
            if (rmdir(filename) != 0)
            {
                TRACE_ERROR("Delete dir %s", filename);
                goto fail;
            }
        }
        else
        {
            if (remove(filename) != 0)
            {
                TRACE_ERROR("Delete file %s", filename);
                goto fail;
            }
        }

        TRACE("Delete file %s", filename);

        httpd_send_headers(con, HTTP_HEADER_200);
    }
    break;

    default:
        goto fail;
    }

    return 0;

fail:
    if (fd != -1)
        close(fd);

    return -1;
}

/** Send HTTP header */
void httpd_send_headers(httpd_connection_t *s, const char *statushdr)
{
    int len;
    char *ptr;

    if (!s->header_send)
    {
        httpd_send(s, statushdr, strlen(statushdr));

        if (s->output_cookies_length > 0)
        {
            strcat(s->cookies, HTTP_CRNL);
            httpd_send(s, s->cookies, strlen(s->cookies));
        }

        if (*s->filename != '\0')
        {
            len = snprintf(s->buffer, sizeof(s->buffer), HTTP_CONTENT_TYPE_DISPOSITION_FILENAME, s->filename);
            httpd_send(s, s->buffer, len);
        }

        len = snprintf(s->buffer, sizeof(s->buffer), "Access-Control-Allow-Origin: *\r\n");
        httpd_send(s, s->buffer, len);

        ptr = strrchr(s->filename, ISO_PERIOD);
        if (ptr == NULL)
        {
            httpd_send(s, HTTP_CONTENT_TYPE_BINARY, sizeof(HTTP_CONTENT_TYPE_BINARY)-1);
        }
        else if (strncmp(HTTP_HTML, ptr, 5) == 0 ||
                 strncmp(HTTP_SHTML, ptr, 6) == 0)
        {
            httpd_send(s, HTTP_CONTENT_TYPE_HTML, sizeof(HTTP_CONTENT_TYPE_HTML)-1);
        }
        else if (strncmp(HTTP_CSS, ptr, 4) == 0)
        {
            httpd_send(s, HTTP_CONTENT_TYPE_CSS, sizeof(HTTP_CONTENT_TYPE_CSS)-1);
        }
        else if (strncmp(HTTP_PNG, ptr, 4) == 0)
        {
            httpd_send(s, HTTP_CONTENT_TYPE_PNG, sizeof(HTTP_CONTENT_TYPE_PNG)-1);
        }
        else if (strncmp(HTTP_GIF, ptr, 4) == 0)
        {
            httpd_send(s, HTTP_CONTENT_TYPE_GIF, sizeof(HTTP_CONTENT_TYPE_GIF)-1);
        }
        else if (strncmp(HTTP_JPG, ptr, 4) == 0)
        {
            httpd_send(s, HTTP_CONTENT_TYPE_JPG, sizeof(HTTP_CONTENT_TYPE_JPG)-1);
        }
        else if (strncmp(HTTP_JSON, ptr, 5) == 0)
        {
            httpd_send(s, HTTP_CONTENT_TYPE_JSON, sizeof(HTTP_CONTENT_TYPE_JSON)-1);
        }
        else if (strncmp(HTTP_TXT, ptr, 5) == 0)
        {
            httpd_send(s, HTTP_CONTENT_TYPE_TEXT, sizeof(HTTP_CONTENT_TYPE_TEXT)-1);
        }
        else
        {
            httpd_send(s, HTTP_CONTENT_TYPE_PLAIN, sizeof(HTTP_CONTENT_TYPE_PLAIN)-1);
        }

        s->header_send = 1;
    }
}

/** Receive data to file */
int httpd_recv_file(httpd_connection_t *con, const char *filename)
{
    int fd, len = 0;

    if ((fd = creat(filename, 0644)) < 0)
    {
        TRACE_ERROR("Can't create file %s", filename);
        throw_exception(fail);
    }

    // Receive data and write to file
    while(con->content_length > 0 && (len = httpd_socket_recv(con->sd, con->buffer, sizeof(con->buffer))) > 0)
    {
        if (write(fd, con->buffer, len) != len)
        {
            TRACE_ERROR("Write to file %s failed", filename);
            throw_exception(fail);
        }

        con->content_length -= len;
    }

    if (len < 0)
    {
        TRACE_ERROR("Recv file '%s'", filename);
        throw_exception(fail);
    }

    close(fd);

    return 0;

fail:
    if (fd != -1)
        close(fd);

    return -1;
}

/** Send data from file */
int httpd_send_file(httpd_connection_t *con, const char *filename)
{
    int fd, len;
    const char *pp;

    if ((fd = open(filename, O_RDONLY, 0)) < 0)
    {
        TRACE_ERROR("Can't open file %s", filename);
        return -1;
    }

    if ((pp = strchr(filename, '/')) != NULL)
        pp++;
    else
        pp = filename;

    httpd_set_content_filename(con, pp);
    httpd_send_headers(con, HTTP_HEADER_200);

    while((len = read(fd, con->buffer, sizeof(con->buffer))) > 0)
    {
        httpd_send(con, con->buffer, len);
    }

    close(fd);

    return 0;
}
