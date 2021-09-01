/**
    output bus params:
        9600 baud
        even parity
        one start bit, one stop bit
        security pause before transmition start

    module uses timer and circular buffer for software implementation of serial bus

    init_bus .. initialize timer interrupt and port pin
    bus_out_putch .. putch function for filling output buffer
*/

#include "system.h"

#ifndef SW_UART_BUFSIZE
#define SW_UART_BUFSIZE     8
#endif

#define BUFF_OUT_LENGTH             SW_UART_BUFSIZE
#define BUFF_OUT_LENGTH_MASK        (BUFF_OUT_LENGTH - 1)

static uint8_t buff_out[BUFF_OUT_LENGTH];
static volatile uint8_t buff_out_wptr = 0;
static volatile uint8_t buff_out_rptr = 0;

#define B2TXP_LOW() PORTB &= ~(1 << PIN5)
#define B2TXP_HIGH() PORTB |= (1 << PIN5)


void sw_uart_init(void)
{
    // output pin 
    PORTB |= (1 << PIN5);
    DDRB |= (1 << PIN5);

    // init timer
    TCCR2A=0x02;
    TCCR2B=0x05;
    TCNT2=0x00;
    OCR2A=12;
    OCR2B=0x00;
}

void sw_uart_putchar(uint8_t c)
{
    // Enable IRQ
    TIMSK2 = 0x02;

    // Wait for space in buffer
    while(((buff_out_wptr + 1) & BUFF_OUT_LENGTH_MASK) == buff_out_rptr);

    buff_out[buff_out_wptr] = c; // write c into buffer
    buff_out_wptr = (buff_out_wptr+1) & BUFF_OUT_LENGTH_MASK; //increase pointer
}

/** output compare (CTC) interrupt handler (occurs every 9600^-1 s) */
ISR(TIMER2_COMPA_vect)
{
    static uint8_t txstat = 0;
    static char txchar;
    static uint8_t txmask;
    static uint8_t parity;

    switch (txstat)
    {
        case 0: // wait incomming char (buffer)
            if (buff_out_wptr != buff_out_rptr)
            {
                txchar = buff_out[buff_out_rptr];
                buff_out_rptr = (buff_out_rptr+1) & BUFF_OUT_LENGTH_MASK;
                txmask = 0x01;

                B2TXP_HIGH(); // pause
                parity = 0;
                txstat++;
            }
            else
            {
                // Disable IRQ
                TIMSK2 &= ~0x02;
            }
            break;
        case 1: // start bit
            B2TXP_LOW();
            txstat++;
            break;
        case 2: // tx bits (message body)
            if (txmask!=0)
            {
                if ((txchar & txmask) != 0)
                {
                    B2TXP_HIGH();
                    parity ^= 0x01;
                }
                else B2TXP_LOW();
                txmask <<= 1;
            }
            else
            {
                if (parity != 0)
                    B2TXP_HIGH();
                else
                    B2TXP_LOW();
                txstat++;
            }
            break;
        case 3: // parity
            B2TXP_HIGH();
            txstat++;
            break;
        case 4: // stop bit
            txstat++;
            break;
        case 5: // pause
            txstat=0;
            break;
        default:
            txstat=0;
            break;
    }
}
