
#include "system.h"
#include "../platform.h"

TRACE_TAG(sx12xxdvb);

#if defined(USE_SX1272_RADIO)
#include "../../chip/sx1272/sx1272-Hal.h"
#include "../../sx127x-radio.h"
#include "sx1272-LoRa.h"
#endif
#if defined(USE_SX1276_RADIO)
#include "../../chip/sx1276/sx1276-Hal.h"
#include "../../sx127x-radio.h"
#endif
#if defined(USE_SX1262_RADIO)
#include "../../chip/sx126x/sx126x_Hal.h"
#include "../../sx127x-radio.h"
#endif

// Globals:
tLoRaSettings LoRaSettings =
{
    870000000,        // RFFrequency
    20,               // Power
    0,                // SignalBw [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] 
    8,               // SpreadingFactor [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
    1,                // ErrorCoding [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
    false,            // CrcOn [0: OFF, 1: ON]
    true,             // ImplicitHeaderOn [0: OFF, 1: ON]
    false,            // RxSingleOn [0: Continuous, 1 Single]
    false,            // FreqHopOn [0: OFF, 1: ON]
    4,                // HopPeriod Hops every frequency hopping period symbols
    18,               // PayloadLength (used for implicit header mode)
    2000,             // TxPacketTimeout
    2000,             // RxPacketTimeout
    6                 // PreambleLen
};


// Default settings
tFskSettings FskSettings = 
{
    869000000,       // RFFrequency
    100000,          // Bitrate
#if defined(RF_CARRIER_TEST) && (RF_CARRIER_TEST == 1)
    0,               // Fdev
 #else
    47000,          // Fdev
 #endif
    325000,          // RxBw
    150000,          // RxBwAfc
    10,              // Power
    1,               // CrcOn
    0,               // AfcOn    
    16,              // PayloadLength
};

// Locals:
static tRadioDriver *radio;

#if defined(RF_CARRIER_TEST) && (RF_CARRIER_TEST == 1)
void sx1272_dvb_test(void)
{
    uint32_t result;
    uint8_t buf[16];

    TRACE("RF carrier test is running ... (CTRL+c -> terminate)");
    TRACE("   RFFrequency = %d", FskSettings.RFFrequency);
    TRACE("   Power = %d", FskSettings.Power);
    TRACE("   RF capacity: %d", FskSettings.rf_capacity);

#ifdef SX1272_SPI_CAP_CS
    // Set RF capacitor
    hal_gpio_set(SX1272_SPI_CAP_CS, 0);
    hal_spi_transmit(SX1272_SPI, FskSettings.rf_capacity);
    hal_gpio_set(SX1272_SPI_CAP_CS, 1);
#endif

    while(1)
    {
        memset(buf, 0, sizeof(buf));
        radio->SetTxPacket(buf, sizeof(buf));

        do
        {
            result = radio->Process();
            switch (result)
            {
                case RF_TX_TIMEOUT:
                    TRACE("RF_TX_TIMEOUT");
                    break;

                case RF_TX_DONE:
                    TRACE("RF_TX_DONE -> Send %d bytes", sizeof(buf));
                    break;

                default:
                    osDelay(50);
                    break;
            }
            
        } while(result == RF_BUSY);

        osDelay(500);
    }
}
#else
void sx1272_dvb_test(void)
{
    uint32_t result;
    uint8_t buf[255];
    uint16_t bufsize;

    TRACE("RX test is running ... (CTRL+c -> terminate)");

    while (1)
    {
        result = radio->Process();
        switch (result)
        {
            case RF_RX_TIMEOUT:
                TRACE("RF_RX_TIMEOUT");
                break;

            case RF_RX_DONE:
            {
                radio->GetRxPacket(buf, &bufsize);
                if (bufsize > 0)
                {
                    TRACE("RF_RX_DONE -> Receive %d bytes  RSSI: %f", bufsize, SX1272LoRaGetPacketRssi());
                    TRACE_DUMP(buf, bufsize);
                }
            }
            break;

            default:
                //TRACE("Radio result: 0x%X", result);
                break;
        }

        osDelay(500);
    }
}
#endif

int sx1272_dvb_init(void)
{
    // Get driver
    if ((radio = RadioDriverInit()) == NULL)
    {
        TRACE_ERROR("Radio driver init failed");
        return -1;
    }

    // Initialize radio
    if (radio->Init() != 0)
    {
        TRACE_ERROR("Radio init failed");
        return -1;
    }

#if defined(RF_CARRIER_TEST) && (RF_CARRIER_TEST == 1)
    sx1272_dvb_test();
#endif
    
    // Start receive
    radio->StartRx();

    TRACE("Init");

    return 0;
}


/** 
 * Read received packet
 * @param buf receive buffer
 * @param bufsize receive bufer size
 * @param received RRSI
 * @return number of received packet bytes else 0 if not or -1 if any error
 */
int sx1272_dvb_receive(uint8_t *buf, int bufsize, int *rssi)
{
    uint32_t result;
    uint16_t rxsize = 0;

    result = radio->Process();
    switch (result)
    {
        case RF_RX_TIMEOUT:
            TRACE("RF_RX_TIMEOUT");
            break;

        case RF_RX_DONE:
        {
            radio->GetRxPacket(buf, &rxsize);
            if (rxsize > 0)
            {
                TRACE("RF_RX_DONE -> Received %d bytes  RSSI: %f", rxsize, SX1272LoRaGetPacketRssi());
                TRACE_DUMP(buf, rxsize);

                *rssi = SX1272LoRaGetPacketRssi();

                return rxsize;
            }
        }
        break;

        case RF_TX_DONE:
            TRACE("RF_TX_DONE");
            break;

        default:
            break;
    }

    return rxsize;
}

/** Send radio packet */
int sx1272_dvb_send(uint8_t *buf, int bufsize)
{
    int res = -1;
    uint32_t result;

    radio->SetTxPacket(buf, bufsize);

    do
    {
        result = radio->Process();
        switch (result)
        {
            case RF_TX_TIMEOUT:
                TRACE("RF_TX_TIMEOUT");
                break;

            case RF_TX_DONE:
                TRACE("RF_TX_DONE -> Send %d bytes", bufsize);
                TRACE_DUMP(buf, bufsize);
                res = bufsize;
                break;

            default:
                osDelay(20);
                break;
        }
        
    } while(result == RF_BUSY);

    // Start receive
    radio->StartRx();

    return res;
}
