/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx1272-Hal.c
 * \brief      SX1272 Hardware Abstraction Layer
 *
 * \version    2.0.B2 
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#include "system.h"

#include "../platform.h"
#include "../../chip/sx1272/sx1272-Hal.h"

TRACE_TAG(sx1272_hal);

// Prototypes:
typedef struct 
{
    uint8_t rxflag;
    uint8_t txflag;

} diomap_t;

static uint8_t sx1272_read_dio(uint8_t dio);


// Locals:
static bool tx_enable = false;

/**
 *            RX                            TX
 * DIO0     RxDone                      TxDone
 * DIO1     RxTimeout                   RxTimeout
 * DIO2     FhssChangeChannel           FhssChangeChannel
 * DIO3     CadDone                     ValidHeader
 * DIO4     CadDetected                 PllLock
 * DIO5     ModeReady                   Mode Ready
 */
static diomap_t diomap[] = 
{
/* DIO[0] */    {(1 << 6), (1 << 3)},     // RxDone, TxDone
/* DIO[1] */    {(1 << 7), (1 << 7)},     // RxTimeout, RxTimeout
/* DIO[2] */    {(1 << 1), (1 << 1)},     // FhssChangeChannel, FhssChangeChannel
/* DIO[3] */    {(1 << 2), (1 << 4)},     // CadDone, ValidHeader
/* DIO[4] */    {(1 << 0), (1 << 0)},     // CadDetected, PllLock
/* DIO[5] */    {(1 << 0), (1 << 0)},     // ModeReady, Mode Ready
};
#define DIOMAP_SIZE  (sizeof(diomap) / sizeof(diomap_t))


void SX1272InitIo( void )
{
    // Enable power
    hal_gpio_set(SX1272_PWR_EN, 1);
    hal_delay_ms(100);

    // Initialize SPI
    if (hal_spi_init(SX1272_SPI) != 0)
    {
        TRACE_ERROR("Init SX1272 spi[%d] failed", SX1272_SPI);
        return;
    }

#ifdef SX1272_SPI_CAP_CS
    // Set RF capacitor
    hal_gpio_set(SX1272_SPI_CAP_CS, 0);
    hal_spi_transmit(SX1272_SPI, SX1272_RF_CAPACITY);
    hal_gpio_set(SX1272_SPI_CAP_CS, 1);
#endif

    TRACE("%s", __FUNCTION__);
}

void SX1272SetReset( uint8_t state )
{
//    TRACE("%s  %d", __FUNCTION__, state);
}

void SX1272Write( uint8_t addr, uint8_t data )
{
    SX1272WriteBuffer(addr, &data, 1);
}

void SX1272Read( uint8_t addr, uint8_t *data )
{
    SX1272ReadBuffer(addr, data, 1);
}

void SX1272WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    hal_gpio_set(SX1272_SPI_CS, 0);

    hal_spi_transmit(SX1272_SPI, addr | 0x80);
    for (int i = 0; i < size; i++)
        hal_spi_transmit(SX1272_SPI, buffer[i]);

    hal_gpio_set(SX1272_SPI_CS, 1);
}

void SX1272ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    hal_gpio_set(SX1272_SPI_CS, 0);

    hal_spi_transmit(SX1272_SPI, addr & 0x7F);
    for (int i = 0; i < size; i++)
        buffer[i] = hal_spi_transmit(SX1272_SPI, 0x00);

    hal_gpio_set(SX1272_SPI_CS, 1);
}

void SX1272WriteFifo(uint8_t *buffer, uint8_t size)
{
    SX1272WriteBuffer(0, buffer, size);
}

void SX1272ReadFifo(uint8_t *buffer, uint8_t size)
{
    SX1272ReadBuffer(0, buffer, size);
}

// TxDone
uint8_t SX1272ReadDio0(void)
{
    return sx1272_read_dio(0);
}

// RxTimeout
uint8_t SX1272ReadDio1( void )
{
    return sx1272_read_dio(1);
}

// FhssChangeChannel
uint8_t SX1272ReadDio2( void )
{
    return sx1272_read_dio(2);
}

// ValidHeader
uint8_t SX1272ReadDio3( void )
{
    return sx1272_read_dio(3);
}

// PllLock
uint8_t SX1272ReadDio4( void )
{
    return sx1272_read_dio(4);
}

//
uint8_t SX1272ReadDio5( void )
{
    return sx1272_read_dio(5);
}

void SX1272WriteRxTx( uint8_t txEnable )
{
    //TRACE("TX %s", txEnable ? "ON" : "OFF");
    tx_enable = txEnable;
}

static uint8_t sx1272_read_dio(uint8_t dio)
{
    uint8_t reg;
    uint8_t state;

    ASSERT(dio < DIOMAP_SIZE);

    // Read Lora IRQ status reg
    SX1272Read(0x12, &reg);

    if (reg & (tx_enable ? diomap[dio].txflag : diomap[dio].rxflag))
        state = 1;
    else
        state = 0;

    //if (reg != 0) {
    //    TRACE("RegIrqFlags[0x12] = 0x%X   DIO[%d] = %d", reg, dio, state);
    //}

    return state;
}
