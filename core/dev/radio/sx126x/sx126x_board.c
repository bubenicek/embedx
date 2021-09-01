/*!
 * \file      sx1262dvb6.c
 *
 * \brief     Target board SX1262_DVB6 shield driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 * \modified  Jiri Bartos
 */

#include "system.h"
#include "sx126x_utils.h"
#include "sx126x_timer.h"
#include "sx126x_radio.h"
#include "sx126x.h"
#include "sx126x_board.h"

TRACE_TAG(sx126_board);


#define BOARD_TCXO_WAKEUP_TIME                      10

#define SpiInOut(_c)  hal_spi_transmit(RADIO_SPI, _c)


void (*DioIrqCallBack)(void);

void  ( *SetTxContinuousWave )( uint32_t freq, int8_t power, uint16_t time );

void SX1262_CS_Low(void)
{
    hal_gpio_set(RADIO_SPI_CS, 0);
}

void SX1262_CS_High(void)
{
    hal_gpio_set(RADIO_SPI_CS, 1);
}

uint8_t SX1262ReadBusy(void)
{
    return 0; //GPIO_PinInGet(SX_GPIO_BUSY_PORT, SX_GPIO_BUSY_PIN);
}

uint8_t SX1262ReadDio1( void )
{
    return 0; //GPIO_PinInGet(SX_GPIO_DIO_1_PORT, SX_GPIO_DIO_1_PIN);
}

/**************************************************************************//**
 * @brief GPIO Even IRQ for pushbuttons on even-numbered pins
 *****************************************************************************/

void GPIO_EVEN_IRQHandler(void) 
{
  // Clear all even pin interrupt flags
  //GPIO_IntClear(0x5555);
  DioIrqCallBack();

}

/**************************************************************************//**
 * @brief GPIO Odd IRQ for pushbuttons on odd-numbered pins
 *****************************************************************************/
/*
void GPIO_ODD_IRQHandler(void) 
{
  // Clear all odd pin interrupt flags
  GPIO_IntClear(0xAAAA);
}
*/

/**************************************************************************//**
 * @brief GPIO initialization
 *****************************************************************************/
void initSX_GPIO_Interrupt(void *dioIrq) 
{
/*    
  // Enable GPIO clock
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(SX_GPIO_DIO_1_PORT, SX_GPIO_DIO_1_PIN, gpioModeInputPull, 0);
  // Enable IRQ for even numbered GPIO pins
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  DioIrqCallBack = dioIrq;
  // Enable rising-edge interrupts for PB pins
  GPIO_ExtIntConfig(SX_GPIO_DIO_1_PORT, SX_GPIO_DIO_1_PIN, SX_GPIO_DIO_1_PIN, true, false, true);
*/
}

void SX126xIoIrqInit( DioIrqHandler dioIrq )
{
    initSX_GPIO_Interrupt( dioIrq );
}

int SX126xIoInit(void)
{   
    // Enable power
    hal_gpio_set(RADIO_PWR_EN, 0);
    hal_delay_ms(20);

    // Reset
    hal_gpio_set(RADIO_NRESET, 0);
    hal_delay_ms(20);
    hal_gpio_set(RADIO_NRESET, 1);
    hal_delay_ms(20);

    // Initialize SPI
    if (hal_spi_init(RADIO_SPI) != 0)
    {
        TRACE_ERROR("Init GPS spi[%d] failed", GPS_SPI);
        return -1;
    }

    TRACE("(2) Testing SPI ...");
    while(1)
    {
        TRACE("Init, chip_ver: 0x%X", SX126xReadRegister(0x42));
        osDelay(500);
    }

    TRACE("Init, chip_ver: 0x%X", SX126xReadRegister(0x42));

    TRACE("TEST SX126");

    return 0;
}

void SX126xIoDeInit( void )
{
}

uint32_t SX126xGetBoardTcxoWakeupTime( void )
{
    return BOARD_TCXO_WAKEUP_TIME;
}

void SX126xReset( void )
{
    hal_gpio_set(RADIO_NRESET, 0);
    hal_delay_ms(20);
    hal_gpio_set(RADIO_NRESET, 1);
}

void SX126xWaitOnBusy( void )
{
    while( SX1262ReadBusy() == 1 );
}

void SX126xWakeup( void )
{
    //CRITICAL_SECTION_BEGIN( );

    SX1262_CS_Low();
    SpiInOut(RADIO_GET_STATUS);
    SpiInOut(0x00 );
    SX1262_CS_High();

    // Wait for chip to be ready.
    SX126xWaitOnBusy( );

    //CRITICAL_SECTION_END( );
}

void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    SX1262_CS_Low();
    SpiInOut(( uint8_t )command );

    for( uint16_t i = 0; i < size; i++ )
    {
        SpiInOut(buffer[i]);
    }
    SX1262_CS_High();

    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}

void SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    SX1262_CS_Low();
    SpiInOut(( uint8_t )command );
    SpiInOut( 0x00 );
    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }
    SX1262_CS_High();

    SX126xWaitOnBusy( );
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    SX1262_CS_Low();
    SpiInOut(RADIO_WRITE_REGISTER );    
    SpiInOut(( address & 0xFF00 ) >> 8 );
    SpiInOut(address & 0x00FF );
    for( uint16_t i = 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }
    SX1262_CS_High();

    SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    //SX126xCheckDeviceReady( );

    SX1262_CS_Low();
    SpiInOut( RADIO_READ_REGISTER );
    SpiInOut( ( address & 0xFF00 ) >> 8 );
    SpiInOut( address & 0x00FF );
    SpiInOut( 0 );
    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut(0);
    }
    SX1262_CS_High();

    //SX126xWaitOnBusy( );
}

uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t data;
    SX126xReadRegisters( address, &data, 1 );
    return data;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );

    SX1262_CS_Low();
    SpiInOut(  RADIO_WRITE_BUFFER );
    SpiInOut(  offset );
    for( uint16_t i = 0; i < size; i++ )
    {
        SpiInOut(  buffer[i] );
    }
    SX1262_CS_High();

    SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );

    SX1262_CS_Low();

    SpiInOut(  RADIO_READ_BUFFER );
    SpiInOut(  offset );
    SpiInOut(  0 );
    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut(  0 );
    }
    SX1262_CS_High();

    SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_40_US );
}

uint8_t SX126xGetDeviceId( void )
{
        return SX1262;
}

void SX126xAntSwOn( void )
{
//    GpioInit( &AntPow, RADIO_ANT_SWITCH_POWER, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
}

void SX126xAntSwOff( void )
{
//    GpioInit( &AntPow, RADIO_ANT_SWITCH_POWER, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

bool SX126xCheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

#if defined( USE_RADIO_DEBUG )
void SX126xDbgPinTxWrite( uint8_t state )
{
    GpioWrite( &DbgPinTx, state );
}

void SX126xDbgPinRxWrite( uint8_t state )
{
    GpioWrite( &DbgPinRx, state );
}
#endif
