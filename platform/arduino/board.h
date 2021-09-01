
#ifndef __ARDUINO_BOARD_H
#define __ARDUINO_BOARD_H

#include <stdio.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>


#define ENABLE_INTERRUPTS()         sei()
#define DISABLE_INTERRUPTS()        cli()

//
// UART settings flags
//
#define  USART_SET_8_1_N   0x006
#define  USART_SET_7_1_N   0x004
#define  USART_SET_6_1_N   0x002
#define  USART_SET_5_1_N   0x000

#define  USART_SET_8_1_E   0x026
#define  USART_SET_7_1_E   0x024
#define  USART_SET_6_1_E   0x022
#define  USART_SET_5_1_E   0x020

#define  USART_SET_8_1_O   0x036
#define  USART_SET_7_1_O   0x034
#define  USART_SET_6_1_O   0x032
#define  USART_SET_5_1_O   0x030

#define  USART_SET_8_2_N   0x00e
#define  USART_SET_7_2_N   0x00c
#define  USART_SET_6_2_N   0x00a
#define  USART_SET_5_2_N   0x008

#define  USART_SET_8_2_E   0x02e
#define  USART_SET_7_2_E   0x02c
#define  USART_SET_6_2_E   0x02a
#define  USART_SET_5_2_E   0x028

#define  USART_SET_8_2_O   0x03e
#define  USART_SET_7_2_O   0x03c
#define  USART_SET_6_2_O   0x03a
#define  USART_SET_5_2_O   0x038


//
// GPIO configuration
//
#define GPIO_PORTA                  0
#define GPIO_PORTB                  1
#define GPIO_PORTC                  2
#define GPIO_PORTD                  3

#define GPIO_MODE_OUTPUT            0
#define GPIO_MODE_INPUT             1


#define CFG_HAL_GPIO_DEF  \
   {GPIO_PORTD, PIN2, GPIO_MODE_OUTPUT, 1},         /* Relay1 */   \
   {GPIO_PORTC, PIN4, GPIO_MODE_OUTPUT, 1},         /* Relay2 */  \
   {GPIO_PORTD, PIN3, GPIO_MODE_OUTPUT, 1},         /* Relay3 */   \
   {GPIO_PORTC, PIN3, GPIO_MODE_OUTPUT, 1},         /* Relay4 */  \
   {GPIO_PORTD, PIN4, GPIO_MODE_OUTPUT, 1},         /* Relay5 */  \
   {GPIO_PORTC, PIN2, GPIO_MODE_OUTPUT, 1},         /* Relay6 */  \
   {GPIO_PORTD, PIN5, GPIO_MODE_OUTPUT, 1},         /* Relay7 */  \
   {GPIO_PORTC, PIN1, GPIO_MODE_OUTPUT, 1},         /* Relay8 */  \
   {GPIO_PORTD, PIN6, GPIO_MODE_OUTPUT, 1},         /* Relay9 */  \
   {GPIO_PORTC, PIN0, GPIO_MODE_OUTPUT, 1},         /* Relay10 */  \
   {GPIO_PORTD, PIN7, GPIO_MODE_OUTPUT, 1},         /* Relay11 */  \
   {GPIO_PORTB, PIN4, GPIO_MODE_OUTPUT, 1},         /* Relay12 */  \
   {GPIO_PORTB, PIN0, GPIO_MODE_OUTPUT, 1},         /* Relay13 */  \
   {GPIO_PORTB, PIN3, GPIO_MODE_OUTPUT, 1},         /* Relay14 */  \
   {GPIO_PORTB, PIN1, GPIO_MODE_OUTPUT, 1},         /* Relay15 */  \
   {GPIO_PORTB, PIN2, GPIO_MODE_OUTPUT, 1},         /* Relay16 */  \
   {GPIO_PORTB, PIN5, GPIO_MODE_OUTPUT, 0},         /* LED system */ \
   {GPIO_PORTC, PIN5, GPIO_MODE_OUTPUT, 0},         /* RS485 DIR */ \



//
// LED configuration
//
#define CFG_HAL_LED_DEF  { \
   {HAL_GPIO16, 1},                                 /* LED system */    \
}

//
// Board configuration
//

#define CFG_HAL_CONSOLE_SWUART               1           // Use software console uart 9600,N,8,1
#define CFG_HAL_NUM_UARTS                    1           // Number of uarts
#define CFG_HAL_UART_RS485                   1           // Use rs485 direction switching
#define CFG_HAL_UART_RS485_AFTER_TX_DELAY    5

#define LED_SYSTEM                           HAL_LED0
#define LED_ERROR                            HAL_LED0
#define GPIO_RS485_DIR                       HAL_GPIO17

#endif
