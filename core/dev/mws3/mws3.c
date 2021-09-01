
#include "system.h"

#ifdef MWS3_UART

TRACE_TAG(mws3);
#if !ENABLE_TRACE_MWS3
#include "trace_undef.h"
#endif

#define CFG_MSW3_LINEBUFSIZE          256

// Prototypes:
static void mws3_uart_recv_callback(hal_uart_t uart, uint8_t c);

// locals:
static char linebuf[CFG_MSW3_LINEBUFSIZE] = "\0";
static char linebuf_out[CFG_MSW3_LINEBUFSIZE] = "\0";
static volatile int linebuf_index = 0;


int mws3_init(void)
{
    if (hal_uart_init(MWS3_UART) != 0)
    {
        TRACE_ERROR("Uart init failed");
        return -1;
    }

    hal_uart_recv(MWS3_UART, mws3_uart_recv_callback);

    TRACE("Init");

    return 0;
}

int mws3_read_line(char *buf, int bufsize)
{
    int len;

    len = strlen(linebuf_out);

    if (bufsize < len + 1)
    {
        TRACE_ERROR("bufsize is small");
        return -1;
    }

    DISABLE_INTERRUPTS();

    if (len > 0)
    {
        strcpy(buf, linebuf_out);
        linebuf_out[0] = '\0';
    }

    ENABLE_INTERRUPTS();

    return len;
}

static void mws3_uart_recv_callback(hal_uart_t uart, uint8_t c)
{
    if (c == '\n')
    {
        linebuf[linebuf_index] = '\0';
        linebuf_index = 0;
        strcpy(linebuf_out, linebuf);
    }
    else if (c != '\r')
    {
        linebuf[linebuf_index++] = c;
        if (linebuf_index >= CFG_MSW3_LINEBUFSIZE)
        {
            TRACE_ERROR("Out of linebuf size");
            linebuf_index = 0;
        }
    }
}


#endif   // MWS3_UART
