
#include "system.h"
#include "tm_stm32f4_spi_dma.h"

TRACE_TAG(hal_spi);
#if ! ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif

#ifndef CFG_HAL_SPI_WAIT_TIMEOUT
#define CFG_HAL_SPI_WAIT_TIMEOUT    ((uint32_t)(10 * 0x1000))
#endif

typedef struct
{
   hal_spi_recv_cb_t recv_cb;
   hal_spi_sent_cb_t sent_cb;

   const uint8_t *buf;
   int bufsize;

} spi_device_t;


// Locals:
static const hal_spi_def_t hal_spi_def[] = CFG_HAL_SPI_DEF;
#define NUM_SPI   (sizeof(hal_spi_def) / sizeof(hal_spi_def_t))
static spi_device_t hal_spi_devices[NUM_SPI];


/** Initialize SPI */
int hal_spi_init(hal_spi_t spi)
{
   int ix;
   const hal_spi_def_t *spidef;

   ASSERT(spi < NUM_SPI);
   spidef = &hal_spi_def[spi];

   memset(&hal_spi_devices[spi], 0, sizeof(spi_device_t));

   //
   // Configure native SPI peripherial
   //

   SPI_InitTypeDef  SPI_InitStructure;
   GPIO_InitTypeDef GPIO_InitStructure;

   GPIO_StructInit(&GPIO_InitStructure);
   SPI_StructInit(&SPI_InitStructure);

   // Enable the clock
   switch(spidef->bus_periph)
   {
      case RCC_PERIPH_APB1:
         RCC_APB1PeriphClockCmd(spidef->clk_periph, ENABLE);
         break;

      case RCC_PERIPH_APB2:
         RCC_APB2PeriphClockCmd(spidef->clk_periph, ENABLE);
         break;

      default:
         TRACE_ERROR("Not supported periph clk: %d", spidef->clk_periph);
         return -1;
   }

   // SPI GPIO Configuration
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

   for (ix = 0; ix < NUM_SPI_PINS; ix++)
   {
      GPIO_PinAFConfig(spidef->pins[ix].port, hal_gpio_pin_source(spidef->pins[ix].pin), spidef->pins[ix].alternate_function);
      GPIO_InitStructure.GPIO_Pin = spidef->pins[ix].pin;
      GPIO_Init(spidef->pins[ix].port, &GPIO_InitStructure);
   }

   // SPI periph configuration
   SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
   SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
   SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
   SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;         // SPI mode 0
   SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
   SPI_InitStructure.SPI_BaudRatePrescaler = spidef->baudrate;
   SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
   SPI_InitStructure.SPI_CRCPolynomial = 7;
   SPI_Init(spidef->spidev, &SPI_InitStructure);

   // Enable periph
   SPI_Cmd(spidef->spidev, ENABLE);

   // Init SPI DMA
   TM_SPI_DMA_Init(spidef->spidev);

   return 0;
}

int hal_spi_set_speed(hal_spi_t spi, uint32_t speed)
{
   const hal_spi_def_t *spidef;
   SPI_InitTypeDef  SPI_InitStructure;

   ASSERT(spi < NUM_SPI);
   spidef = &hal_spi_def[spi];

   if (speed <= 3000000)
      speed = SPI_BaudRatePrescaler_32;
   else
      speed = spidef->baudrate;

   SPI_Cmd(spidef->spidev, DISABLE);
   SPI_I2S_DeInit(spidef->spidev);

   // SPI periph configuration
   SPI_StructInit(&SPI_InitStructure);
   SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
   SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
   SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
   SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;         // SPI mode 0
   SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
   SPI_InitStructure.SPI_BaudRatePrescaler = speed;
   SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
   SPI_InitStructure.SPI_CRCPolynomial = 7;

   SPI_Init(spidef->spidev, &SPI_InitStructure);

   // Enable periph
   SPI_Cmd(spidef->spidev, ENABLE);

   return 0;
}

int hal_spi_transmit(hal_spi_t spi, uint8_t c)
{
   ASSERT(spi < NUM_SPI);

   // Loop while DR register in not emplty
   while (SPI_I2S_GetFlagStatus(hal_spi_def[spi].spidev, SPI_I2S_FLAG_TXE) == RESET);

   // Send byte through the SPI peripheral
   SPI_I2S_SendData(hal_spi_def[spi].spidev, c);

   // Wait to receive a byte
   while (SPI_I2S_GetFlagStatus(hal_spi_def[spi].spidev, SPI_I2S_FLAG_RXNE) == RESET);

   // Return the byte read from the SPI bus
   return SPI_I2S_ReceiveData(hal_spi_def[spi].spidev);
}

int hal_spi_read(hal_spi_t spi, uint8_t *buf, int bufsize)
{
   ASSERT(spi < NUM_SPI);

   if (hal_spi_devices[spi].recv_cb != NULL)
   {
      //
      // Non blocking, using DMA complete interrupt handler
      //

      hal_spi_devices[spi].buf = buf;
      hal_spi_devices[spi].bufsize = bufsize;

      // Receive data, sent dummy 0x00 bytes to slave via DMA
      TM_SPI_DMA_Transmit(hal_spi_def[spi].spidev, buf, buf, bufsize);
   }
   else
   {
      //
      // Blocking, using poling
      //

      // Receive data, sent dummy 0x00 bytes to slave via DMA
      if (TM_SPI_DMA_Transmit(hal_spi_def[spi].spidev, buf, buf, bufsize) == 0)
      {
         TRACE_ERROR("TM_SPI_DMA_Transmit failed");
         return -1;
      }

      // Wait till SPI DMA do it's job
      while (TM_SPI_DMA_Working(hal_spi_def[spi].spidev)); 
   }

   return bufsize;
}

int hal_spi_write(hal_spi_t spi, const uint8_t *buf, int bufsize)
{
   ASSERT(spi < NUM_SPI);

   if (hal_spi_devices[spi].sent_cb != NULL)
   {
      //
      // Non blocking, using DMA complete interrupt handler
      //
      hal_spi_devices[spi].buf = buf;
      hal_spi_devices[spi].bufsize = bufsize;

      TM_SPI_DMA_Transmit(hal_spi_def[spi].spidev, (uint8_t *)buf, NULL, bufsize);
   }
   else
   {
      //
      // Blocking, using poling
      //
      TM_SPI_DMA_Transmit(hal_spi_def[spi].spidev, (uint8_t *)buf, NULL, bufsize);
   
      // Wait till SPI DMA do it's job
      while (TM_SPI_DMA_Working(hal_spi_def[spi].spidev));      
   }

   return bufsize;
}

/** Register callback for nonblocking receive data */
int hal_spi_recv(hal_spi_t spi, hal_spi_recv_cb_t recv_cb)
{
   ASSERT(spi < NUM_SPI);

   DISABLE_INTERRUPTS();
   hal_spi_devices[spi].recv_cb = recv_cb;
   TM_SPI_DMA_EnableInterrupts(hal_spi_def[spi].spidev);
   ENABLE_INTERRUPTS();

   return 0;
}

/** Register callback for nonblocking sent data */
int hal_spi_sent(hal_spi_t spi, hal_spi_sent_cb_t sent_cb)
{
   ASSERT(spi < NUM_SPI);

   DISABLE_INTERRUPTS();
   hal_spi_devices[spi].sent_cb = sent_cb;
   TM_SPI_DMA_EnableInterrupts(hal_spi_def[spi].spidev);
   ENABLE_INTERRUPTS();

   return 0;
}

void TM_DMA_TransferCompleteHandler(DMA_Stream_TypeDef* DMA_Stream) 
{
   hal_spi_t spi;

   for (spi = 0; spi < NUM_SPI; spi++)
   {
      if (TM_SPI_DMA_GetStreamTX(hal_spi_def[spi].spidev) == DMA_Stream)
      {
         if (hal_spi_devices[spi].sent_cb != NULL)
            hal_spi_devices[spi].sent_cb(spi, hal_spi_devices[spi].buf, hal_spi_devices[spi].bufsize);

         break;
      }
      else if (TM_SPI_DMA_GetStreamRX(hal_spi_def[spi].spidev) == DMA_Stream)
      {
         if (hal_spi_devices[spi].recv_cb != NULL)
            hal_spi_devices[spi].recv_cb(spi, hal_spi_devices[spi].buf, hal_spi_devices[spi].bufsize);

         break;
      }
   }
}
