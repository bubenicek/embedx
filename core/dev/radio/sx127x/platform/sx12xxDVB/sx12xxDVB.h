
#ifndef __SX12XXDVB_H__
#define __SX12XXDVB_H__



/*!
 * Functions return codes definition
 */
typedef enum
{
    SX_OK,
    SX_ERROR,
    SX_BUSY,
    SX_EMPTY,
    SX_DONE,
    SX_TIMEOUT,
    SX_UNSUPPORTED,
    SX_WAIT,
    SX_CLOSE,
    SX_YES,
    SX_NO,          

}tReturnCodes;


typedef struct sLoRaSettings
{
    uint32_t  RFFrequency;
    int8_t    Power;
    uint8_t   SignalBw;                   // LORA [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] 
    uint8_t   SpreadingFactor;            // LORA [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
    uint8_t   ErrorCoding;                // LORA [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
    bool      CrcOn;                         // [0: OFF, 1: ON]
    bool      ImplicitHeaderOn;              // [0: OFF, 1: ON]
    bool      RxSingleOn;                    // [0: Continuous, 1 Single]
    bool      FreqHopOn;                     // [0: OFF, 1: ON]
    uint8_t   HopPeriod;                  // Hops every frequency hopping period symbols
    uint8_t   PayloadLength;
    uint32_t  TxPacketTimeout;
    uint32_t  RxPacketTimeout;
    uint16_t  PreambleLen;

} tLoRaSettings;


typedef struct sFskSettings
{
    uint32_t RFFrequency;
    uint32_t Bitrate;
    uint32_t Fdev;
    uint32_t RxBw;
    uint32_t RxBwAfc;
    int8_t Power;
    bool CrcOn;
    bool AfcOn;
    uint8_t PayloadLength;

} tFskSettings;

extern tLoRaSettings LoRaSettings;
extern tFskSettings FskSettings;


/** Initialize radio */
int sx1272_dvb_init(void);

/** 
 * Read received packet
 * @param buf receive buffer
 * @param bufsize receive bufer size
 * @param received RRSI
 * @return number of received packet bytes else 0 if not or -1 if any error
 */
int sx1272_dvb_receive(uint8_t *buf, int bufsize, int *rssi);

/** Send radio packet */
int sx1272_dvb_send(uint8_t *buf, int bufsize);

void sx1272_dvb_test(void);

#endif // __SX12XXEIGER_H__
