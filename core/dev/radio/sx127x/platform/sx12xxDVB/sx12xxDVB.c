
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
    11,               // SpreadingFactor [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
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
tFskSettings FskSettings; 

// Locals:
static tRadioDriver *radio;


void sx1272_dvb_test(void)
{
    uint32_t result;
    uint8_t buf[255];
    uint16_t bufsize;

    TRACE("RX test is running ...");

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


#if 0
void RadioOn(void)
{
  uint8_t i, *ptr;
  uint32_t result;

  result = Radio->Process();
  switch (result)
  {
  case RF_RX_TIMEOUT:
    break;

  case RF_RX_DONE:
    Radio->GetRxPacket(&Buffer[0], (uint16_t *)&BufferSize);
    if (BufferSize > 0)
    {
      //         LedSet(LED_3, 1);   // Rx Done
      Set_LED_End_Time(50);
#if (HW_PLATFORM_MODE != SCUBA_DIVER_MODE)
      {
        uint16_t sts;

        calc_aes_sign(&Buffer[0], &Buffer[BufferSize], BufferSize - 2);
        if (!memcmp(&Buffer[BufferSize - 2], &Buffer[BufferSize], 2))
        {
          //           sts = ctrl_aes_sign(&Buffer[0], 12, &Buffer[14]);
          Check_Scuba_IDE(&Buffer[0]);
          USB_Send_Nav_Msg(&Buffer[0], SCUBA_DIVER_MODE);
          Buffer[0] = BASE_STATION_MODE;
          BufferSize = 9;
          LedSet(LED_RED, 1);
          Radio->SetTxPacket(Buffer, BufferSize);
        }
      }
#endif
#if (HW_PLATFORM_MODE == SCUBA_DIVER_MODE)
      {
        //            SX12xx_SetPower(PWR_OFF);
        if ((BufferSize == LoRaSettings.PayloadLength))
        {
          if (Wait_RF_Ack)
          {
            if ((Buffer[0] == BASE_STATION_MODE) && !memcmp(&Buffer[1], &RF_SendBuff[1], BufferSize - 1))
            {
#if ((PCB_VERSION == PCB_VERSION_2V1M) || (PCB_VERSION == PCB_VERSION_3V1F) || (PCB_VERSION == PCB_VERSION_4V1A))
              Base_RSSI = SX1272LR->RegPktRssiValue;
              SX1272LoRaSetOpMode(RFLR_OPMODE_SLEEP);
#endif
              if (radio_ack < 10)
                //            if (radio_ack<3)
                radio_ack++;
              Wait_RF_Ack = 0;
              LedSet(LED_RED, 1); // Ack
              delay_ms(5);
              ALLLedSet(0x00);
            }
          }
        }
      }
#endif
    }
    break;
  case RF_TX_DONE:
    LedSet(LED_RED, 0);
    GPIO_PinModeSet(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN_1, gpioModePushPull, 0);
    Radio->StartRx();
#if (HW_PLATFORM_MODE == SCUBA_DIVER_MODE)
    //         Wait_RF_Ack = 1;
#endif
    break;
  default:
    if (RF_SendBuffPtr != NULL)
    {
#if ((PCB_VERSION == PCB_VERSION_2V1M) || (PCB_VERSION == PCB_VERSION_3V1F) || (PCB_VERSION == PCB_VERSION_4V1A))
      if (SX1272LoRaIsChannelFree(DVBCfg->Sys.LoRaSettings.RFFrequency, -80, 10))
#else
      if (1)
#endif
      {
        BufferSize = RF_SendBuffLen;
        memcpy(&Buffer[0], RF_SendBuffPtr, BufferSize);
        RF_SendBuffPtr = NULL;
        LedSet(LED_RED, 1);
        Radio->SetTxPacket(Buffer, BufferSize);
#if (HW_PLATFORM_MODE == SCUBA_DIVER_MODE)
        Base_RSSI = 0;
        Wait_RF_Ack = 1;
#endif
      }
      else
      {
      }
    }
    break;
  }
}


void Radio_Control( void )
{
#if (HW_PLATFORM_MODE != SCUBA_DIVER_MODE)
 RadioOn();
#endif
#if (HW_PLATFORM_MODE == SCUBA_DIVER_MODE)
 uint32_t startTick = TickCounter;;
 
 do 
 {
  RadioOn();
 } while ( Wait_RF_Ack  && (( TickCounter - startTick ) < (2000/1) ));
 if (Wait_RF_Ack)
 {
  Wait_RF_Ack = 0;
  if (radio_ack)
   radio_ack--;
 }
// LedSet(LED_RED, 0);
#endif
}

#endif
