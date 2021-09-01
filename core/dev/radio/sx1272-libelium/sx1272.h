/*
 *  Library for LoRa 868 / 915MHz SX1272 LoRa module
 *  
 *  Copyright (C) Libelium Comunicaciones Distribuidas S.L. 
 *  http://www.libelium.com 
 *  
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 3 of the License, or 
 *  (at your option) any later version. 
 *  
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see http://www.gnu.org/licenses/. 
 *  
 *  Version:           1.1
 *  Design:            David Gascón 
 *  Implementation:    Covadonga Albiñana & Victor Boria
 */
 
/*
 * Many changes have been performed to v1.1 released by Libelium
 * C. Pham, see change logs in SX1272.cpp
 */

#ifndef __SX1272_H
#define __SX1272_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdlib.h>
#include <stdint.h>

/******************************************************************************
 * Definitions & Declarations
 *****************************************************************************/

// added by C. Pham
// do not remove!
#define W_REQUESTED_ACK
//#define W_NET_KEY
//#define W_INITIALIZATION

//it is not mandatory to wire this pin
//we take pin 4 as it is available on many boards
#define SX1272_WRST
#define SX1272_RST  4

#if defined ARDUINO_AVR_FEATHER32U4 || defined ARDUINO_SAMD_FEATHER_M0
// on the Adafruit Feather, the RFM95W is embeded and CS pin is normally on pin 8
#define SX1272_SS 8
#elif defined ARDUINO_ESP8266_ESP01
#define SX1272_SS 15
#else
// starting from November 3rd, 2017, the CS pin is always pin number 10 on Arduino boards
// if you use the Libelium Multiprotocol shield to connect a Libelium LoRa then change the CS pin to pin 2
#define SX1272_SS 10
#endif

#define SX1272Chip  0
#define SX1276Chip  1
// end

#define SX1272_debug_mode 1
// added by C. Pham
//#define SX1272_led_send_receive

#ifdef SX1272_led_send_receive
#define SX1272_led_send 2
#define SX1272_led_receive 3
#endif

//! MACROS //
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)  // read a bit
#define bitSet(value, bit) ((value) |= (1UL << (bit)))    // set bit to '1'
#define bitClear(value, bit) ((value) &= ~(1UL << (bit))) // set bit to '0'

//! REGISTERS //

#define        REG_FIFO        					0x00
#define        REG_OP_MODE        				0x01
#define        REG_BITRATE_MSB    				0x02
#define        REG_BITRATE_LSB    				0x03
#define        REG_FDEV_MSB   					0x04
#define        REG_FDEV_LSB    					0x05
#define        REG_FRF_MSB    					0x06
#define        REG_FRF_MID    					0x07
#define        REG_FRF_LSB    					0x08
#define        REG_PA_CONFIG    				0x09
#define        REG_PA_RAMP    					0x0A
#define        REG_OCP    						0x0B
#define        REG_LNA    						0x0C
#define        REG_RX_CONFIG    				0x0D
#define        REG_FIFO_ADDR_PTR  				0x0D
#define        REG_RSSI_CONFIG   				0x0E
#define        REG_FIFO_TX_BASE_ADDR 		    0x0E
#define        REG_RSSI_COLLISION    			0x0F
#define        REG_FIFO_RX_BASE_ADDR   			0x0F
#define        REG_RSSI_THRESH    				0x10
#define        REG_FIFO_RX_CURRENT_ADDR   		0x10
#define        REG_RSSI_VALUE_FSK	    		0x11
#define        REG_IRQ_FLAGS_MASK    			0x11
#define        REG_RX_BW		    			0x12
#define        REG_IRQ_FLAGS	    			0x12
#define        REG_AFC_BW		    			0x13
#define        REG_RX_NB_BYTES	    			0x13
#define        REG_OOK_PEAK	    				0x14
#define        REG_RX_HEADER_CNT_VALUE_MSB  	0x14
#define        REG_OOK_FIX	    				0x15
#define        REG_RX_HEADER_CNT_VALUE_LSB  	0x15
#define        REG_OOK_AVG	 					0x16
#define        REG_RX_PACKET_CNT_VALUE_MSB  	0x16
#define        REG_RX_PACKET_CNT_VALUE_LSB  	0x17
#define        REG_MODEM_STAT	  				0x18
#define        REG_PKT_SNR_VALUE	  			0x19
#define        REG_AFC_FEI	  					0x1A
#define        REG_PKT_RSSI_VALUE	  			0x1A
#define        REG_AFC_MSB	  					0x1B
#define        REG_RSSI_VALUE_LORA	  			0x1B
#define        REG_AFC_LSB	  					0x1C
#define        REG_HOP_CHANNEL	  				0x1C
#define        REG_FEI_MSB	  					0x1D
#define        REG_MODEM_CONFIG1	 		 	0x1D
#define        REG_FEI_LSB	  					0x1E
#define        REG_MODEM_CONFIG2	  			0x1E
#define        REG_PREAMBLE_DETECT  			0x1F
#define        REG_SYMB_TIMEOUT_LSB  			0x1F
#define        REG_RX_TIMEOUT1	  				0x20
#define        REG_PREAMBLE_MSB_LORA  			0x20
#define        REG_RX_TIMEOUT2	  				0x21
#define        REG_PREAMBLE_LSB_LORA  			0x21
#define        REG_RX_TIMEOUT3	 				0x22
#define        REG_PAYLOAD_LENGTH_LORA		 	0x22
#define        REG_RX_DELAY	 					0x23
#define        REG_MAX_PAYLOAD_LENGTH 			0x23
#define        REG_OSC		 					0x24
#define        REG_HOP_PERIOD	  				0x24
#define        REG_PREAMBLE_MSB_FSK 			0x25
#define        REG_FIFO_RX_BYTE_ADDR 			0x25
#define        REG_PREAMBLE_LSB_FSK 			0x26
// added by C. Pham
#define        REG_MODEM_CONFIG3	  			0x26
// end
#define        REG_SYNC_CONFIG	  				0x27
#define        REG_SYNC_VALUE1	 				0x28
#define        REG_SYNC_VALUE2	  				0x29
#define        REG_SYNC_VALUE3	  				0x2A
#define        REG_SYNC_VALUE4	  				0x2B
#define        REG_SYNC_VALUE5	  				0x2C
#define        REG_SYNC_VALUE6	  				0x2D
#define        REG_SYNC_VALUE7	  				0x2E
#define        REG_SYNC_VALUE8	  				0x2F
#define        REG_PACKET_CONFIG1	  			0x30
#define        REG_PACKET_CONFIG2	  			0x31
#define        REG_DETECT_OPTIMIZE              0x31
#define        REG_PAYLOAD_LENGTH_FSK			0x32
// added by C. Pham
#define        REG_INVERT_IQ	  				0x33
#define        REG_INVERT_IQ2	  				0x3B
#define        REG_BROADCAST_ADRS	 		 	0x34
#define        REG_FIFO_THRESH	  				0x35
#define        REG_SEQ_CONFIG1	  				0x36
#define        REG_SEQ_CONFIG2	  				0x37
#define        REG_DETECTION_THRESHOLD          0x37
#define        REG_TIMER_RESOL	  				0x38
// added by C. Pham
#define        REG_SYNC_WORD                    0x39
//end
#define        REG_TIMER1_COEF	  				0x39
#define        REG_TIMER2_COEF	  				0x3A
#define        REG_IMAGE_CAL	  				0x3B
#define        REG_TEMP		  					0x3C
#define        REG_LOW_BAT	  					0x3D
#define        REG_IRQ_FLAGS1	  				0x3E
#define        REG_IRQ_FLAGS2	  				0x3F
#define        REG_DIO_MAPPING1	  				0x40
#define        REG_DIO_MAPPING2	  				0x41
#define        REG_VERSION	  					0x42
#define        REG_AGC_REF	  					0x43
#define        REG_AGC_THRESH1	  				0x44
#define        REG_AGC_THRESH2	  				0x45
#define        REG_AGC_THRESH3	  				0x46
#define        REG_PLL_HOP	  					0x4B
#define        REG_TCXO		  					0x58
#define        REG_PA_DAC		  				0x5A
#define        REG_PLL		  					0x5C
#define        REG_PLL_LOW_PN	  				0x5E
#define        REG_FORMER_TEMP	  				0x6C
#define        REG_BIT_RATE_FRAC	  			0x70

// added by C. Pham
// copied from LoRaMAC-Node
/*!
 * RegImageCal
 */
#define RF_IMAGECAL_AUTOIMAGECAL_MASK               0x7F
#define RF_IMAGECAL_AUTOIMAGECAL_ON                 0x80
#define RF_IMAGECAL_AUTOIMAGECAL_OFF                0x00  // Default

#define RF_IMAGECAL_IMAGECAL_MASK                   0xBF
#define RF_IMAGECAL_IMAGECAL_START                  0x40

#define RF_IMAGECAL_IMAGECAL_RUNNING                0x20
#define RF_IMAGECAL_IMAGECAL_DONE                   0x00  // Default

#define RF_IMAGECAL_TEMPCHANGE_HIGHER               0x08
#define RF_IMAGECAL_TEMPCHANGE_LOWER                0x00

#define RF_IMAGECAL_TEMPTHRESHOLD_MASK              0xF9
#define RF_IMAGECAL_TEMPTHRESHOLD_05                0x00
#define RF_IMAGECAL_TEMPTHRESHOLD_10                0x02  // Default
#define RF_IMAGECAL_TEMPTHRESHOLD_15                0x04
#define RF_IMAGECAL_TEMPTHRESHOLD_20                0x06

#define RF_IMAGECAL_TEMPMONITOR_MASK                0xFE
#define RF_IMAGECAL_TEMPMONITOR_ON                  0x00 // Default
#define RF_IMAGECAL_TEMPMONITOR_OFF                 0x01

// added by C. Pham
// The crystal oscillator frequency of the module
#define RH_LORA_FXOSC 32000000.0
 
// The Frequency Synthesizer step = RH_LORA_FXOSC / 2^^19
#define RH_LORA_FCONVERT  (524288 / RH_LORA_FXOSC)

// Frf = frf(Hz)*2^19/RH_LORA_FXOSC

/////

//FREQUENCY CHANNELS:
// added by C. Pham for Senegal
#define CH_04_868 0xD7CCCC // channel 04, central freq = 863.20MHz
#define CH_05_868 0xD7E000 // channel 05, central freq = 863.50MHz
#define CH_06_868 0xD7F333 // channel 06, central freq = 863.80MHz
#define CH_07_868 0xD80666 // channel 07, central freq = 864.10MHz
#define CH_08_868 0xD81999 // channel 08, central freq = 864.40MHz
#define CH_09_868 0xD82CCC // channel 09, central freq = 864.70MHz
//
#define CH_10_868 0xD84CCC // channel 10, central freq = 865.20MHz, = 865200000*RH_LORA_FCONVERT
#define CH_11_868 0xD86000 // channel 11, central freq = 865.50MHz
#define CH_12_868 0xD87333 // channel 12, central freq = 865.80MHz
#define CH_13_868 0xD88666 // channel 13, central freq = 866.10MHz
#define CH_14_868 0xD89999 // channel 14, central freq = 866.40MHz
#define CH_15_868 0xD8ACCC // channel 15, central freq = 866.70MHz
#define CH_16_868 0xD8C000 // channel 16, central freq = 867.00MHz
#define CH_17_868 0xD90000 // channel 17, central freq = 868.00MHz

// added by C. Pham
#define CH_18_868 0xD90666 // 868.1MHz for LoRaWAN test
// end
#define CH_00_900 0xE1C51E // channel 00, central freq = 903.08MHz
#define CH_01_900 0xE24F5C // channel 01, central freq = 905.24MHz
#define CH_02_900 0xE2D999 // channel 02, central freq = 907.40MHz
#define CH_03_900 0xE363D7 // channel 03, central freq = 909.56MHz
#define CH_04_900 0xE3EE14 // channel 04, central freq = 911.72MHz
#define CH_05_900 0xE47851 // channel 05, central freq = 913.88MHz
#define CH_06_900 0xE5028F // channel 06, central freq = 916.04MHz
#define CH_07_900 0xE58CCC // channel 07, central freq = 918.20MHz
#define CH_08_900 0xE6170A // channel 08, central freq = 920.36MHz
#define CH_09_900 0xE6A147 // channel 09, central freq = 922.52MHz
#define CH_10_900 0xE72B85 // channel 10, central freq = 924.68MHz
#define CH_11_900 0xE7B5C2 // channel 11, central freq = 926.84MHz
#define CH_12_900 0xE4C000 // default channel 915MHz, the module is configured with it

// added by C. Pham
#define CH_00_433 0x6C5333 // 433.3MHz
#define CH_01_433 0x6C6666 // 433.6MHz
#define CH_02_433 0x6C7999 // 433.9MHz
#define CH_03_433 0x6C9333 // 434.3MHz
// end

//LORA BANDWIDTH:
// modified by C. Pham
#define SX1272_BW_125 0x00
#define SX1272_BW_250 0x01
#define SX1272_BW_500 0x02

// use the following constants with setBW()
#define BW_7_8 0x00
#define BW_10_4 0x01
#define BW_15_6 0x02
#define BW_20_8 0x03
#define BW_31_25 0x04
#define BW_41_7 0x05
#define BW_62_5 0x06
#define BW_125 0x07
#define BW_250 0x08
#define BW_500 0x09
// end

//LORA CODING RATE:
#define CR_5 0x01
#define CR_6 0x02
#define CR_7 0x03
#define CR_8 0x04

//LORA SPREADING FACTOR:
#define SF_6 0x06
#define SF_7 0x07
#define SF_8 0x08
#define SF_9 0x09
#define SF_10 0x0A
#define SF_11 0x0B
#define SF_12 0x0C

//LORA MODES:
#define LORA_SLEEP_MODE 0x80
#define LORA_STANDBY_MODE 0x81
#define LORA_TX_MODE 0x83
#define LORA_RX_MODE 0x85

// added by C. Pham
#define LORA_CAD_MODE               0x87
#define LNA_MAX_GAIN                0x23
#define LNA_OFF_GAIN                0x00
#define LNA_LOW_GAIN		    0x20
// end

#define LORA_STANDBY_FSK_REGS_MODE 0xC1

//FSK MODES:
#define FSK_SLEEP_MODE 0x00
#define FSK_STANDBY_MODE 0x01
#define FSK_TX_MODE 0x03
#define FSK_RX_MODE 0x05

//OTHER CONSTANTS:

#define HEADER_ON 0
#define HEADER_OFF 1
#define CRC_ON 1
#define CRC_OFF 0
#define LORA 1
#define FSK 0
#define BROADCAST_0 0x00
#define MAX_LENGTH 255
#define MAX_PAYLOAD 251
#define MAX_LENGTH_FSK 64
#define MAX_PAYLOAD_FSK 60
//modified by C. Pham, 7 instead of 5 because we added a type field which should be PKT_TYPE_ACK and the SNR
#define  ACK_LENGTH 7
// added by C. Pham
#ifdef W_NET_KEY
#define NET_KEY_LENGTH 2
#define OFFSET_PAYLOADLENGTH 4+NET_KEY_LENGTH
#define net_key_0  0x12
#define net_key_1  0x34
#else
// modified by C. Pham to remove the retry field and the length field
// which will be replaced by packet type field
#define OFFSET_PAYLOADLENGTH 4
#endif
#define OFFSET_RSSI 139
#define NOISE_FIGURE 6.0
#define NOISE_ABSOLUTE_ZERO 174.0
#define MAX_TIMEOUT  10000		//10000 msec = 10.0 sec
#define MAX_WAIT  12000		//12000 msec = 12.0 sec
#define MAX_RETRIES 5
#define CORRECT_PACKET 0
#define INCORRECT_PACKET 1
#define INCORRECT_PACKET_TYPE 2

// added by C. Pham
// Packet type definition

#define PKT_TYPE_MASK   0xF0
#define PKT_FLAG_MASK   0x0F

#define PKT_TYPE_DATA   0x10
#define PKT_TYPE_ACK    0x20

#define PKT_FLAG_ACK_REQ            0x08
#define PKT_FLAG_DATA_ENCRYPTED     0x04
#define PKT_FLAG_DATA_WAPPKEY       0x02
#define PKT_FLAG_DATA_DOWNLINK      0x01

#define SX1272_ERROR_ACK        3
#define SX1272_ERROR_TOA        4

#define INVERT_IQ_RX			0x00
#define INVERT_IQ_TX			0x01

//! Structure :
/*!
 */
struct pack
{
	// added by C. Pham
#ifdef W_NET_KEY	
	uint8_t netkey[NET_KEY_LENGTH];
#endif	
	//! Structure Variable : Packet destination
	/*!
 	*/
	uint8_t dst;

    // added by C. Pham
    //! Structure Variable : Packet type
    /*!
    */
    uint8_t type;

	//! Structure Variable : Packet source
	/*!
 	*/
	uint8_t src;

	//! Structure Variable : Packet number
	/*!
 	*/
	uint8_t packnum;

    // modified by C. Pham
    // will not be used in the transmitted packet
	//! Structure Variable : Packet length
	/*!
 	*/
	uint8_t length;

    // modified by C. Pham
    // use a pointer instead of static variable to same memory footprint
	//! Structure Variable : Packet payload
	/*!
 	*/
    uint8_t* data;

    // modified by C. Pham
    // will not be used in the transmitted packet
	//! Structure Variable : Retry number
	/*!
 	*/
	uint8_t retry;
};


//! class constructor
/*!
It does nothing
\param void
\return void
    */
int sx1272_init();

//! It puts the module ON
/*!
\param void
\return uint8_t setLORA state
    */
int sx1272_ON();

//! It puts the module OFF
/*!
\param void
\return void
    */
void sx1272_OFF();

//! It reads an internal module register.
/*!
\param byte address : address register to read from.
\return the content of the register.
    */
uint8_t sx1272_readRegister(uint8_t address);

//! It writes in an internal module register.
/*!
\param byte address : address register to write in.
\param byte data : value to write in the register.
    */
void sx1272_writeRegister(uint8_t address, uint8_t data);

//! It clears the interruption flags.
/*!
\param void
\return void
    */
void sx1272_clearFlags();

//! It sets the LoRa mode on.
/*!
It stores in global '_LORA' variable '1' when success
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_setLORA();

//! It sets the FSK mode on.
/*!
It stores in global '_FSK' variable '1' when success
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_setFSK();

//! It gets the BW, SF and CR of the module.
/*!
It stores in global '_bandwidth' variable the BW
It stores in global '_codingRate' variable the CR
It stores in global '_spreadingFactor' variable the SF
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_getMode();

//! It sets the BW, SF and CR of the module.
/*!
It stores in global '_bandwidth' variable the BW
It stores in global '_codingRate' variable the CR
It stores in global '_spreadingFactor' variable the SF
\param uint8_t mode : there is a mode number to different values of
the	configured parameters with this function.
\return '0' on success, '1' otherwise
    */
int8_t sx1272_setMode(uint8_t mode);

//! It gets the header mode configured.
/*!
It stores in global '_header' variable '0' when header is sent
(explicit header mode) or '1' when is not sent (implicit header
mode).
\return '0' on success, '1' otherwise
    */
uint8_t	sx1272_getHeader();

//! It sets explicit header mode.
/*!
It stores in global '_header' variable '1' when success
\return '0' on success, '1' otherwise
    */
int8_t	sx1272_setHeaderON();

//! It sets implicit header mode.
/*!
It stores in global '_header' variable '0' when success
\return '0' on success, '1' otherwise
    */
int8_t	sx1272_setHeaderOFF();

//! It gets the CRC configured.
/*!
It stores in global '_CRC' variable '1' enabling CRC generation on
payload, or '0' disabling the CRC.
\return '0' on success, '1' otherwise
    */
uint8_t	sx1272_getCRC();

//! It sets CRC on.
/*!
It stores in global '_CRC' variable '1' when success
\return '0' on success, '1' otherwise
    */
uint8_t	sx1272_setCRC_ON();

//! It sets CRC off.
/*!
It stores in global '_CRC' variable '0' when success
\return '0' on success, '1' otherwise
    */
uint8_t	sx1272_setCRC_OFF();

//! It is true if the SF selected exists.
/*!
\param uint8_t spr : spreading factor value to check.
\return 'true' on success, 'false' otherwise
    */
bool	sx1272_isSF(uint8_t spr);

//! It gets the SF configured.
/*!
It stores in global '_spreadingFactor' variable the current value of SF
\return '0' on success, '1' otherwise
    */
int8_t	sx1272_getSF();

//! It sets the SF.
/*!
It stores in global '_spreadingFactor' variable the current value of SF
\param uint8_t spr : spreading factor value to set in the configuration.
\return '0' on success, '1' otherwise
    */
uint8_t	sx1272_setSF(uint8_t spr);

//! It is true if the BW selected exists.
/*!
\param uint16_t band : bandwidth value to check.
\return 'true' on success, 'false' otherwise
    */
bool	sx1272_isBW(uint16_t band);

//! It gets the BW configured.
/*!
It stores in global '_bandwidth' variable the BW selected
in the configuration
\return '0' on success, '1' otherwise
    */
int8_t	sx1272_getBW();

//! It sets the BW.
/*!
It stores in global '_bandwidth' variable the BW selected
in the configuration
\param uint16_t band : bandwidth value to set in the configuration.
\return '0' on success, '1' otherwise
    */
int8_t sx1272_setBW(uint16_t band);

//! It is true if the CR selected exists.
/*!
\param uint8_t cod : the coding rate value to check.
\return 'true' on success, 'false' otherwise
    */
bool	sx1272_isCR(uint8_t cod);

//! It gets the CR configured.
/*!
It stores in global '_codingRate' variable the CR selected
in the configuration
\return '0' on success, '1' otherwise
    */
int8_t	sx1272_getCR();

//! It sets the CR.
/*!
It stores in global '_codingRate' variable the CR selected
in the configuration
\param uint8_t cod : coding rate value to set in the configuration.
\return '0' on success, '1' otherwise
    */
int8_t	sx1272_setCR(uint8_t cod);


//! It is true if the channel selected exists.
/*!
\param uint32_t ch : frequency channel value to check.
\return 'true' on success, 'false' otherwise
    */
bool sx1272_isChannel(uint32_t ch);

//! It gets frequency channel the module is using.
/*!
It stores in global '_channel' variable the frequency channel
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_getChannel();

//! It sets frequency channel the module is using.
/*!
It stores in global '_channel' variable the frequency channel
\param uint32_t ch : frequency channel value to set in the configuration.
\return '0' on success, '1' otherwise
    */
int8_t sx1272_setChannel(uint32_t ch);

//! It gets the output power of the signal.
/*!
It stores in global '_power' variable the output power of the signal
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_getPower();

//! It sets the output power of the signal.
/*!
It stores in global '_power' variable the output power of the signal
\param char p : 'M', 'H' or 'L' if you want Maximum, High or Low
output power signal.
\return '0' on success, '1' otherwise
    */
int8_t sx1272_setPower(char p);

//! It sets the output power of the signal.
/*!
It stores in global '_power' variable the output power of the signal
\param uint8_t pow : value to set as output power.
\return '0' on success, '1' otherwise
    */
int8_t sx1272_setPowerNum(uint8_t pow);

//! It gets the preamble length configured.
/*!
It stores in global '_preamblelength' variable the preamble length
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_getPreambleLength();

//! It sets the preamble length.
/*!
It stores in global '_preamblelength' variable the preamble length
\param uint16_t l : preamble length to set in the configuration.
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_setPreambleLength(uint16_t l);

//! It gets the payload length of the last packet to send/receive.
/*!
It stores in global '_payloadlength' variable the payload length of
the last packet to send/receive.
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_getPayloadLength();

//! It sets the packet length to send/receive.
/*!
It stores in global '_payloadlength' variable the payload length of
the last packet to send/receive.
\param uint8_t l : payload length to set in the configuration.
\return '0' on success, '1' otherwise
    */
int8_t sx1272_setPacketLength(uint8_t l);

//! It gets the node address of the mote.
/*!
returns '_nodeAddress' variable the node address
    */
uint8_t sx1272_getNodeAddress();

//! It sets the node address of the mote.
/*!
It stores in global '_nodeAddress' variable the node address
\param uint8_t addr : address value to set as node address.
\return '0' on success, '-1' otherwise
    */
int8_t sx1272_setNodeAddress(uint8_t addr);

//! It gets the SNR of the latest received packet.
/*!
It stores in global '_SNR' variable the SNR
\return '0' on success, '1' otherwise
    */
int8_t sx1272_getSNR();

//! It gets the current value of RSSI.
/*!
It stores in global '_RSSI' variable the current value of RSSI
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_getRSSI();

//! It gets the RSSI of the latest received packet.
/*!
It stores in global '_RSSIpacket' variable the RSSI of the latest
packet received.
\return '0' on success, '1' otherwise
    */
int16_t sx1272_getRSSIpacket();

//! It sets the total of retries when a packet is not correctly received.
/*!
It stores in global '_maxRetries' variable the number of retries.
\param uint8_t ret : number of retries.
\return '0' on success, '1' otherwise
    */
//uint8_t setRetries(uint8_t ret);

//! It gets the maximum current supply by the module.
/*!
    *
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_getMaxCurrent();

//! It sets the maximum current supply by the module.
/*!
It stores in global '_maxCurrent' variable the maximum current supply.
\param uint8_t rate : maximum current supply.
\return '0' on success, '1' otherwise
    */
int8_t sx1272_setMaxCurrent(uint8_t rate);

//! It gets the content of the main configuration registers.
/*!
It stores in global '_bandwidth' variable the BW.
It stores in global '_codingRate' variable the CR.
It stores in global '_spreadingFactor' variable the SF.
It stores in global '_power' variable the output power of the signal.
It stores in global '_channel' variable the frequency channel.
It stores in global '_CRC' variable '1' enabling CRC generation on
payload, or '0' disabling the CRC.
It stores in global '_header' variable '0' when header is sent
(explicit header mode) or '1' when is not sent (implicit header
mode).
It stores in global '_preamblelength' variable the preamble length.
It stores in global '_payloadlength' variable the payload length of
the last packet to send/receive.
It stores in global '_nodeAddress' variable the node address.
It stores in global '_temp' variable the module temperature.
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_getRegs();

//! It sets the maximum number of bytes from a frame that fit in a packet structure.
/*!
It stores in global '_payloadlength' variable the maximum number of bytes.
\param uint16_t length16 : total frame length.
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_truncPayload(uint16_t length16);

//! It writes an ACK in FIFO to send it.
/*!
    *
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_setACK();

//! It puts the module in reception mode.
/*!
    *
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_receive();

//! It receives a packet before MAX_TIMEOUT.
/*!
    *
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_receivePacketMAXTimeout();

//! It receives a packet before a timeout.
/*!
\param uint16_t wait : time to wait to receive something.
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_receivePacketTimeout(uint16_t wait);

//! It receives a packet before MAX_TIMEOUT and reply with an ACK.
/*!
    *
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_receivePacketMAXTimeoutACK();

//! It receives a packet before a timeout and reply with an ACK.
/*!
\param uint16_t wait : time to wait to receive something.
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_receivePacketTimeoutACK(uint16_t wait);

//! It puts the module in 'promiscuous' reception mode with a timeout.
/*!
\param uint16_t wait : time to wait to receive something.
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_receiveAll(uint16_t wait);

//! It checks if there is an available packet and its destination before a timeout.
/*!
    *
\param uint16_t wait : time to wait while there is no a valid header received.
\return 'true' on success, 'false' otherwise
    */
bool	sx1272_availableData(uint16_t wait);

//! It writes a packet in FIFO in order to send it.
/*!
\param uint8_t dest : packet destination.
\param char *payload : packet payload.
\return '0' on success, '1' otherwise
*/
//uint8_t setPacket(uint8_t dest, char *payload);

//! It writes a packet in FIFO in order to send it.
/*!
\param uint8_t dest : packet destination.
\param uint8_t *payload: packet payload.
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_setPacket(uint8_t dest, uint8_t *payload);

//! It reads a received packet from the FIFO, if it arrives before ending MAX_TIMEOUT time.
/*!
    *
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_getPacketMAXTimeout();

//! It receives and gets a packet from FIFO, if it arrives before ending 'wait' time.
/*!
    *
\param uint16_t wait : time to wait while there is no a complete packet received.
\return '0' on success, '1' otherwise
*/
int8_t sx1272_getPacket(uint16_t wait);

//! It sends the packet stored in FIFO before ending MAX_TIMEOUT.
/*!
    *
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_sendWithMAXTimeout();

//! It tries to send the packet stored in FIFO before ending 'wait' time.
/*!
\param uint16_t wait : time to wait to send the packet.
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_sendWithTimeout(uint16_t wait);

//! It tries to send the packet wich payload is a parameter before ending MAX_TIMEOUT.
/*!
\param uint8_t dest : packet destination.
\param char *payload : packet payload.
\return '0' on success, '1' otherwise
*/
//uint8_t sendPacketMAXTimeout(uint8_t dest, char *payload);

//! It tries to send the packet wich payload is a parameter before ending MAX_TIMEOUT.
/*!
\param uint8_t dest : packet destination.
\param uint8_t *payload : packet payload.
\param uint16_t length : payload buffer length.
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_sendPacketMAXTimeout(uint8_t dest, uint8_t *payload, uint16_t length);

//! It sends the packet wich payload is a parameter before ending 'wait' time.
/*!
\param uint8_t dest : packet destination.
\param char *payload : packet payload.
\param uint16_t wait : time to wait.
\return '0' on success, '1' otherwise
*/
//uint8_t sendPacketTimeout(uint8_t dest, char *payload, uint16_t wait);

//! It sends the packet wich payload is a parameter before ending 'wait' time.
/*!
\param uint8_t dest : packet destination.
\param uint8_t *payload : packet payload.
\param uint16_t length : payload buffer length.
\param uint16_t wait : time to wait.
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_sendPacketTimeout(uint8_t dest, uint8_t *payload, uint16_t length, uint16_t wait);

//! It sends the packet wich payload is a parameter before MAX_TIMEOUT, and replies with ACK.
/*!
\param uint8_t dest : packet destination.
\param char *payload : packet payload.
\return '0' on success, '1' otherwise
*/
//uint8_t sendPacketMAXTimeoutACK(uint8_t dest, char *payload);

//! It sends the packet wich payload is a parameter before MAX_TIMEOUT, and replies with ACK.
/*!
\param uint8_t dest : packet destination.
\param uint8_t payload: packet payload.
\param uint16_t length : payload buffer length.
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_sendPacketMAXTimeoutACK(uint8_t dest, uint8_t *payload, uint16_t length);

//! It sends the packet wich payload is a parameter before a timeout, and replies with ACK.
/*!
\param uint8_t dest : packet destination.
\param char *payload : packet payload.
\return '0' on success, '1' otherwise
*/
//uint8_t sendPacketTimeoutACK(uint8_t dest, char *payload);

//! It sends the packet wich payload is a parameter before 'wait' time, and replies with ACK.
/*!
\param uint8_t dest : packet destination.
\param uint8_t payload: packet payload.
\param uint16_t length : payload buffer length.
\param uint16_t wait : time to wait to send the packet.
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_sendPacketTimeoutACK(uint8_t dest, uint8_t *payload, uint16_t length, uint16_t wait);

//! It sets the destination of a packet.
/*!
\param uint8_t dest : value to set as destination address.
\return '0' on success, '1' otherwise
    */
int8_t sx1272_setDestination(uint8_t dest);

//! It sets the waiting time to send a packet.
/*!
It stores in global '_sendTime' variable the time for each mode.
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_setTimeout();

//! It sets the payload of the packet that is going to be sent.
/*!
\param char *payload : packet payload.
\return '0' on success, '1' otherwise
    */
//uint8_t setPayload(char *payload);

//! It sets the payload of the packet that is going to be sent.
/*!
\param uint8_t payload: packet payload.
\return '0' on success, '1' otherwise
    */
uint8_t sx1272_setPayload(uint8_t *payload);

//! It receives and gets an ACK from FIFO, if it arrives before ending 'wait' time.
/*!
    *
\param uint16_t wait : time to wait while there is no an ACK received.
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_getACK(uint16_t wait);

//! It sends a packet, waits to receive an ACK and updates the _retries value, before ending MAX_TIMEOUT time.
/*!
\param uint8_t dest : packet destination.
\param char *payload : packet payload.
\return '0' on success, '1' otherwise
*/
//uint8_t sendPacketMAXTimeoutACKRetries(uint8_t dest, char *payload);

//! It sends a packet, waits to receive an ACK and updates the _retries value, before ending MAX_TIMEOUT time.
/*!
\param uint8_t dest : packet destination.
\param uint8_t *payload : packet payload.
\param uint16_t length : payload buffer length.
\return '0' on success, '1' otherwise
*/
//uint8_t sendPacketMAXTimeoutACKRetries(uint8_t dest, uint8_t *payload, uint16_t length);

//! It sends a packet, waits to receive an ACK and updates the _retries value.
/*!
\param uint8_t dest : packet destination.
\param char *payload : packet payload.
\return '0' on success, '1' otherwise
*/
//uint8_t sendPacketTimeoutACKRetries(uint8_t dest, char *payload);

//! It sends a packet, waits to receive an ACK and updates the _retries value.
/*!
\param uint8_t dest : packet destination.
\param uint8_t *payload : packet payload.
\param uint16_t length : payload buffer length.
\return '0' on success, '1' otherwise
*/
//uint8_t sendPacketTimeoutACKRetries(uint8_t dest, uint8_t *payload, uint16_t length);

//! It sends a packet, waits to receive an ACK and updates the _retries value, before ending 'wait' time.
/*!
\param uint8_t dest : packet destination.
\param char *payload : packet payload.
\param uint16_t wait : time to wait while trying to send the packet.
\return '0' on success, '1' otherwise
*/
//uint8_t sendPacketTimeoutACKRetries(uint8_t dest, char *payload, uint16_t wait);

//! It sends a packet, waits to receive an ACK and updates the _retries value, before ending 'wait' time.
/*!
\param uint8_t dest : packet destination.
\param uint8_t *payload : packet payload.
\param uint16_t length : payload buffer length.
\param uint16_t wait : time to wait while trying to send the packet.
\return '0' on success, '1' otherwise
*/
//uint8_t sendPacketTimeoutACKRetries(uint8_t dest, uint8_t *payload, uint16_t length, uint16_t wait);

//! It gets the internal temperature of the module.
/*!
It stores in global '_temp' variable the module temperature.
\return '0' on success, '1' otherwise
*/
uint8_t sx1272_getTemp();

// added by C. Pham
void sx1272_setPacketType(uint8_t type);
void sx1272_RxChainCalibration();
uint8_t sx1272_doCAD(uint8_t counter);
uint16_t sx1272_getToA(uint8_t pl);
void sx1272_CarrierSense(uint8_t cs);
void sx1272_CarrierSense1();
void sx1272_CarrierSense2();
void sx1272_CarrierSense3(); 
int8_t sx1272_setSyncWord(uint8_t sw);
int8_t sx1272_getSyncWord();
int8_t sx1272_setSleepMode();
int8_t sx1272_setPowerDBM(uint8_t dbm);
long sx1272_limitToA();
long sx1272_getRemainingToA();
long sx1272_removeToA(uint16_t toa);
int8_t sx1272_setFreqHopOn();
int8_t sx1272_invertIQ(uint8_t dir, bool invert);
void sx1272_setCSPin(uint8_t cs);

#endif   // __SX1272_H
