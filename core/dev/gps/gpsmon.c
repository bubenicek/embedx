
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "system.h"
#include "minmea.h"
#include "gps.h"
#include "gps_driver.h"

TRACE_TAG(gpsmon);
#if !ENABLE_TRACE_GPS
#undef TRACE
#define TRACE(...)
#endif

#ifndef CFG_GPSMON_PORT
#define CFG_GPSMON_PORT 2021
#endif

#ifndef CFG_GPSMON_THREAD_STACK_SIZE
#define CFG_GPSMON_THREAD_STACK_SIZE 4096
#endif

#ifndef CFG_GPSMON_THREAD_PRIORITY
#define CFG_GPSMON_THREAD_PRIORITY osPriorityNormal
#endif

// Prototypes:
static void gpsmon_thread(void *arg);

// Locals:
static const osThreadDef(GPSMON, gpsmon_thread, CFG_GPSMON_THREAD_PRIORITY, 0, CFG_GPSMON_THREAD_STACK_SIZE);
static int spi_fd;
static int server_socket;
static int client_socket;
static bool connected = false;

/** Initialize GPS socket monitor */
int gpsmon_init(void)
{
    // Enable power
 	hal_gpio_set(GPS_PWR_EN, 0);

    spi_fd = open("/dev/spidev0.0", O_RDWR);
    if (spi_fd < 0)
    {
        TRACE_ERROR("open() SPI error");
        return -1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0)
    {
        TRACE_ERROR("Create socket failed");
        return -1;
    }

    int yes = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
    {
        TRACE_ERROR("setsockopt failed");
        return -1;
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(CFG_GPSMON_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        TRACE_ERROR("Bind socket failed");
        return -1;
    }

    if (listen(server_socket, 5) < 0)
    {
        TRACE_ERROR("Listen socket failed");
        return -1;
    }

    // Start monitor thread
    if (osThreadCreate(osThread(GPSMON), NULL) == 0)
    {
        TRACE_ERROR("Start GPSMON thread failed");
        return -1;
    }

    TRACE("Init");

    return 0;
}

static void gpsmon_thread(void *arg)
{
    int res;
    struct sockaddr_in client_address;
    socklen_t address_size = sizeof(client_address);

    uint8_t txdata;
    uint8_t rxdata;

    struct spi_ioc_transfer tr = {0};

    tr.tx_buf = (unsigned long)&txdata;
    tr.rx_buf = (unsigned long)&rxdata;
    tr.len = 1;
    tr.speed_hz = 1000000;
    tr.bits_per_word = 8;

    enum State
    {
        Message_Start,

        UBX_Sync2,
        UBX_Class,
        UBX_ID,
        UBX_Length_LSB,
        UBX_Length_MSB,
        UBX_Payload,
        UBX_CK_A,
        UBX_CK_B,

        NMEA_Message
    };

    int state = Message_Start;
    unsigned char message[1024];
    unsigned int position = 0;
    unsigned short ubx_payload_length;

    TRACE("Running ...");

    while (1)
    {
        TRACE("Waiting for connection ...");

        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &address_size)) < 0)
        {
            TRACE_ERROR("Accept connection failed");
            continue;
        }

        TRACE("New connection accepted");
        connected = true;

        while (1)
        {
            // Reading data from a TCP client
            if ((res = recv(client_socket, &txdata, 1, MSG_DONTWAIT)) < 0)
            {
                txdata = 0x00;
            }
            else if (res == 0)
            {
                TRACE("Client disconnected");
                break;
            }

            // Performing SPI transfer
            ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);

            // Scanning incoming SPI data for messages
            message[position++] = rxdata;

            switch (state)
            {
            case Message_Start:
                if (rxdata == 0xb5)
                {
                    state = UBX_Sync2;
                }
                else if (rxdata == '$')
                {
                    state = NMEA_Message;
                }
                else
                {
                    state = Message_Start;
                    position = 0;
                }
                break;

            case UBX_Sync2:
                if (rxdata == 0x62)
                    state = UBX_Class;
                else if (rxdata == 0xb5)
                {
                    state = Message_Start;
                    position = 1;
                }
                else
                {
                    state = Message_Start;
                    position = 0;
                }
                break;

            case UBX_Class:
                state = UBX_ID;
                break;

            case UBX_ID:
                state = UBX_Length_LSB;
                break;

            case UBX_Length_LSB:
                ubx_payload_length = rxdata;
                state = UBX_Length_MSB;
                break;

            case UBX_Length_MSB:
                ubx_payload_length += rxdata << 8;
                state = UBX_Payload;
                break;

            case UBX_Payload:
                if (position == ubx_payload_length + 6)
                    state = UBX_CK_A;
                if (position >= 1022)
                {
                    state = Message_Start;
                    position = 0;
                }
                break;

            case UBX_CK_A:
                state = UBX_CK_B;
                break;

            case UBX_CK_B:
                /* Sending the received message after getting the last symbol */
                send(client_socket, message, 6 + ubx_payload_length + 2, 0);
                state = Message_Start;
                position = 0;
                break;

            case NMEA_Message:
                /* Sending the message after receiveng the end symbol (LF)  */
                if (rxdata == '\n')
                    send(client_socket, message, position, 0);

                if (rxdata == '\n' || position >= 80)
                {
                    state = Message_Start;
                    position = 0;
                }
                break;
            }
        }

        connected = false;
    }
}
