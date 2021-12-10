
#include "system.h"
#include "httpd_socket.h"

TRACE_TAG(http_client);

int httpclient_connect(const char *host, int port)
{
    int sd;

    if ((sd = httpd_raw_socket_connect(host, port)) < 0)
    {
        TRACE_ERROR("Connect to %s failed", host);
        return -1;
    }

    TRACE("Connected to %s:%d", host, port);

    return sd;
}

/** Close connection with host */
int httpclient_close(int sd)
{
    TRACE("Disconnected");
    return httpd_raw_socket_close(sd);
}

/** Send http GET request */
int httpclient_get(int sd, const char *url)
{
    int len;

    if (httpd_raw_socket_send(sd, "GET ", 4) < 0)
    {
        TRACE_ERROR("Send request");
        throw_exception(fail);
    }

    if (httpd_raw_socket_send(sd, url, strlen(url)) < 0)
    {
        TRACE_ERROR("Send request");
        throw_exception(fail);
    }

    if (httpd_raw_socket_send(sd, " HTTP/1.0\r\n\r\n", 13) < 0)
    {
        TRACE_ERROR("Send data end");
        throw_exception(fail);
    }

    TRACE("Send GET %s", url);

    return 0;

fail:
    return -1;
}

/** Read data from host */
int httpclient_read_data(int sd, uint8_t *buf, int bufsize)
{
    return httpd_raw_socket_recv(sd, buf, bufsize);
}

/** Get file from host and save it to local file */
int httpclient_get_file(const char *host, int port, const char *url, const char *filename)
{
    int res;
    int sd;
    FILE *fw = NULL;
    uint8_t buf[512];

    if ((fw = fopen(filename, "w")) == NULL)
    {
        TRACE_ERROR("Can't create file %s", filename);
        throw_exception(fail);
    }

    if ((sd = httpclient_connect(host, port)) < 0)
        throw_exception(fail);


    if (httpclient_get(sd, url) != 0)
    {
        TRACE_ERROR("Send GET request failed");
        throw_exception(fail);
    }

    // Read status
    if ((res = httpd_raw_socket_readto(sd, buf, sizeof(buf), '\n')) <= 0)
    {
        TRACE_ERROR("Read status failed");
        throw_exception(fail);
    }

    if (strstr(buf, "200 OK") == NULL)
    {
        TRACE_ERROR("Error resposponse: %s", buf);
        throw_exception(fail);
    }

    // Read header
    while(1)
    {
        if ((res = httpd_raw_socket_readto(sd, buf, sizeof(buf), '\n')) <= 0)
        {
            TRACE_ERROR("Read header failed");
            throw_exception(fail);
        }

        if (buf[0] == '\r' && buf[1] == '\n')
            break;
    }

    // Read payload
    while((res = httpclient_read_data(sd, buf, sizeof(buf))) > 0)
    {
        if (fwrite(buf, sizeof(char), res, fw) < 0)
        {
            TRACE_ERROR("Write to file %s failed", filename);
            throw_exception(fail);
        }
    }

    fclose(fw);
    httpclient_close(sd);

    TRACE("Get file from %s:%d  %s -> %s finished", host, port, url, filename);

    return 0;

fail:
    if (sd != -1)
        httpclient_close(sd);

    if (fw != NULL)
    {
        fclose(fw);
        unlink(filename);
    }

    return -1;
}