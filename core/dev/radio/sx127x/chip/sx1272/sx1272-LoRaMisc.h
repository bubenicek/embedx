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
 * \file       sx1272-LoRaMisc.h
 * \brief      SX1272 RF chip high level functions driver
 *
 * \remark     Optional support functions.
 *             These functions are defined only to easy the change of the
 *             parameters.
 *             For a final firmware the radio parameters will be known so
 *             there is no need to support all possible parameters.
 *             Removing these functions will greatly reduce the final firmware
 *             size.
 *
 * \version    2.0.B2 
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#ifndef __SX1272_LORA_MISC_H__
#define __SX1272_LORA_MISC_H__


/*!
 * \brief Writes the new RF frequency value
 *
 * \param [IN] freq New RF frequency value in [Hz]
 */
void SX1272LoRaSetRFFrequency( uint32_t freq );

/*!
 * \brief Reads the current RF frequency value
 *
 * \retval freq Current RF frequency value in [Hz]
 */
uint32_t SX1272LoRaGetRFFrequency( void );

/*!
 * \brief Writes the new RF output power value
 *
 * \param [IN] power New output power value in [dBm]
 */
void SX1272LoRaSetRFPower( int8_t power );

/*!
 * \brief Reads the current RF output power value
 *
 * \retval power Current output power value in [dBm]
 */
int8_t SX1272LoRaGetRFPower( void );

/*!
 * \brief Writes the new Signal Bandwidth value
 *
 * \remark This function sets the IF frequency according to the datasheet
 *
 * \param [IN] factor New Signal Bandwidth value [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]
 */
void SX1272LoRaSetSignalBandwidth( uint8_t bw );

/*!
 * \brief Reads the current Signal Bandwidth value
 *
 * \retval factor Current Signal Bandwidth value [0: 125 kHz, 1: 250 kHz, 2: 500 kHz] 
 */
uint8_t SX1272LoRaGetSignalBandwidth( void );

/*!
 * \brief Writes the new Spreading Factor value
 *
 * \param [IN] factor New Spreading Factor value [7, 8, 9, 10, 11, 12]
 */
void SX1272LoRaSetSpreadingFactor( uint8_t factor );

/*!
 * \brief Reads the current Spreading Factor value
 *
 * \retval factor Current Spreading Factor value [7, 8, 9, 10, 11, 12] 
 */
uint8_t SX1272LoRaGetSpreadingFactor( void );

/*!
 * \brief Writes the new Error Coding value
 *
 * \param [IN] value New Error Coding value [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 */
void SX1272LoRaSetErrorCoding( uint8_t value );

/*!
 * \brief Reads the current Error Coding value
 *
 * \retval value Current Error Coding value [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 */
uint8_t SX1272LoRaGetErrorCoding( void );

/*!
 * \brief Enables/Disables the packet CRC generation
 *
 * \param [IN] enaable [true, false]
 */
void SX1272LoRaSetPacketCrcOn( bool enable );

/*!
 * \brief Reads the current packet CRC generation status
 *
 * \retval enable [true, false]
 */
bool SX1272LoRaGetPacketCrcOn( void );

/*!
 * \brief Enables/Disables the Implicit Header mode in LoRa
 *
 * \param [IN] enable [true, false]
 */
void SX1272LoRaSetImplicitHeaderOn( bool enable );

/*!
 * \brief Check if implicit header mode in LoRa in enabled or disabled
 *
 * \retval enable [true, false]
 */
bool SX1272LoRaGetImplicitHeaderOn( void );

/*!
 * \brief Enables/Disables Rx single instead of Rx continuous
 *
 * \param [IN] enable [true, false]
 */
void SX1272LoRaSetRxSingleOn( bool enable );

/*!
 * \brief Check if LoRa is in Rx Single mode
 *
 * \retval enable [true, false]
 */
bool SX1272LoRaGetRxSingleOn( void );

/*!
 * \brief Enables/Disables the frequency hopping 
 *
 * \param [IN] enable [true, false]
 */
 
void SX1272LoRaSetFreqHopOn( bool enable );

/*!
 * \brief Get the frequency hopping status 
 *
 * \param [IN] enable [true, false]
 */
bool SX1272LoRaGetFreqHopOn( void );

/*!
 * \brief Set symbol period between frequency hops
 *
 * \param [IN] value
 */
void SX1272LoRaSetHopPeriod( uint8_t value );

/*!
 * \brief Get symbol period between frequency hops
 *
 * \retval value symbol period between frequency hops
 */
uint8_t SX1272LoRaGetHopPeriod( void );

/*!
 * \brief Set timeout Tx packet (based on MCU timer, timeout between Tx Mode entry Tx Done IRQ)
 *
 * \param [IN] value timeout (ms)
 */
void SX1272LoRaSetTxPacketTimeout( uint32_t value );

/*!
 * \brief Get timeout between Tx packet (based on MCU timer, timeout between Tx Mode entry Tx Done IRQ)
 *
 * \retval value timeout (ms)
 */
uint32_t SX1272LoRaGetTxPacketTimeout( void );

/*!
 * \brief Set timeout Rx packet (based on MCU timer, timeout between Rx Mode entry and Rx Done IRQ)
 *
 * \param [IN] value timeout (ms)
 */
void SX1272LoRaSetRxPacketTimeout( uint32_t value );

/*!
 * \brief Get timeout Rx packet (based on MCU timer, timeout between Rx Mode entry and Rx Done IRQ)
 *
 * \retval value timeout (ms)
 */
uint32_t SX1272LoRaGetRxPacketTimeout( void );

/*!
 * \brief Set payload length
 *
 * \param [IN] value payload length
 */
void SX1272LoRaSetPayloadLength( uint8_t value );

/*!
 * \brief Get payload length
 *
 * \retval value payload length
 */
uint8_t SX1272LoRaGetPayloadLength( void );

/*!
 * \brief Enables/Disables the 20 dBm PA
 *
 * \param [IN] enable [true, false]
 */
void SX1272LoRaSetPa20dBm( bool enale );

/*!
 * \brief Gets the current 20 dBm PA status
 *
 * \retval enable [true, false]
 */
bool SX1272LoRaGetPa20dBm( void );

/*!
 * \brief Set the RF Output pin 
 *
 * \param [IN] RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
 */
void SX1272LoRaSetPAOutput( uint8_t outputPin );

/*!
 * \brief Gets the used RF Ouptu pin
 *
 * \retval RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
 */
uint8_t SX1272LoRaGetPAOutput( void );
/*!
 * \brief Writes the new PA rise/fall time of ramp up/down value
 *
 * \param [IN] value New PaRamp value
 */
void SX1272LoRaSetPaRamp( uint8_t value );

/*!
 * \brief Reads the current PA rise/fall time of ramp up/down value
 *
 * \retval freq Current PaRamp value
 */
uint8_t SX1272LoRaGetPaRamp( void );

/*!
 * \brief Set Symbol Timeout based on symbol length
 *
 * \param [IN] value number of symbol
 */
void SX1272LoRaSetSymbTimeout( uint16_t value );

/*!
 * \brief  Get Symbol Timeout based on symbol length
 *
 * \retval value number of symbol
 */
uint16_t SX1272LoRaGetSymbTimeout( void );

/*!
 * \brief  Configure the device to optimize low datarate transfers
 *
 * \param [IN] enable Enables/Disables the low datarate optimization
 */
void SX1272LoRaSetLowDatarateOptimize( bool enable );

/*!
 * \brief  Get the status of optimize low datarate transfers
 *
 * \retval LowDatarateOptimize enable or disable
 */
bool SX1272LoRaGetLowDatarateOptimize( void );

/*!
 * \brief Get the preamble length
 *
 * \retval value preamble length
 */
uint16_t SX1272LoRaGetPreambleLength( void );

/*!
 * \brief Set the preamble length
 *
 * \param [IN] value preamble length
 */
void SX1272LoRaSetPreambleLength( uint16_t value );

/*!
 * \brief Set the number or rolling preamble symbol needed for detection
 *
 * \param [IN] value number of preamble symbol
 */
void SX1272LoRaSetNbTrigPeaks( uint8_t value );

/*!
 * \brief Get the number or rolling preamble symbol needed for detection
 *
 * \retval value number of preamble symbol
 */
uint8_t SX1272LoRaGetNbTrigPeaks( void );

void SX1272LoRaSetOverCurrent( int8_t OnOff,  int8_t newOC );

bool SX1272LoRaIsChannelFree(uint32_t freq, int16_t rssiThresh, uint32_t maxCarrierSenseTime );

uint32_t SX1272Random( void );

#endif //__SX1272_LORA_MISC_H__
