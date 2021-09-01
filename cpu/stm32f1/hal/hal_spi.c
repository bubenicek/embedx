
#include "system.h"

TRACE_TAG(hal_spi);
#if !ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

#ifndef CFG_SPI_DMA_BUFFER_SIZE
#define CFG_SPI_DMA_BUFFER_SIZE     32768
#endif

#ifndef CFG_SPI_READ_TIMEOUT
#define CFG_SPI_READ_TIMEOUT        1000
#endif

static SPI_HandleTypeDef hspi;
static DMA_HandleTypeDef hdma_spi1_rx;

static hal_spi_recv_cb_t recv_cb = NULL;
static bool recv_enabled = false;

static uint8_t recv_buf[CFG_SPI_DMA_BUFFER_SIZE];
static size_t dma_head = 0;
#define dma_tail ((CFG_SPI_DMA_BUFFER_SIZE - hspi.hdmarx->Instance->CNDTR) & (CFG_SPI_DMA_BUFFER_SIZE - 1))


int hal_spi_init(hal_spi_t spi)
{
   GPIO_InitTypeDef GPIO_InitStruct;

   __HAL_RCC_DMA1_CLK_ENABLE();
   __HAL_RCC_SPI1_CLK_ENABLE();

   /** SPI1 GPIO Configuration    
    PA4     ------> SPI1_NSS
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI 
    */
   GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = GPIO_PIN_6;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   // Configure SPI periph
   hspi.Instance = SPI1;
   hspi.Init.Mode = SPI_MODE_SLAVE;
   hspi.Init.Direction = SPI_DIRECTION_2LINES;
   hspi.Init.DataSize = SPI_DATASIZE_8BIT;
   hspi.Init.CLKPolarity = SPI_POLARITY_HIGH;
   hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
   hspi.Init.NSS = SPI_NSS_HARD_INPUT;
   hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
   hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
   hspi.Init.TIMode = SPI_TIMODE_DISABLE;
   hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
   hspi.Init.CRCPolynomial = 10;
   if (HAL_SPI_Init(&hspi) != HAL_OK) 
   {
      TRACE_ERROR("Init SPI failed");
      return -1;
   }

   // SPI1 DMA Init
   hdma_spi1_rx.Instance = DMA1_Channel2;
   hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
   hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
   hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
   hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
   hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
   hdma_spi1_rx.Init.Mode = DMA_CIRCULAR;
   hdma_spi1_rx.Init.Priority = DMA_PRIORITY_LOW;
   if (HAL_DMA_Init(&hdma_spi1_rx) != HAL_OK)
   {
      TRACE_ERROR("HAL_DMA_Init failed");
      return -1;
   }
   __HAL_LINKDMA(&hspi, hdmarx, hdma_spi1_rx);

   // DMA interrupt init
   HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

   // SPI1 interrupt Init
   //HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
   //HAL_NVIC_EnableIRQ(SPI1_IRQn);

   TRACE("SPI[%d] init", spi);

   return 0;
}

int hal_spi_set_speed(hal_spi_t spi, uint32_t speed)
{
   return 0;
}

int hal_spi_transmit(hal_spi_t spi, uint8_t c)
{
   VERIFY(HAL_SPI_TransmitReceive(&hspi, &c, &c, 1, HAL_MAX_DELAY) == HAL_OK);
   return c;
}

int hal_spi_write(hal_spi_t spi, const uint8_t *buf, int bufsize)
{
   VERIFY(HAL_SPI_Transmit(&hspi, (uint8_t *)buf, bufsize, HAL_MAX_DELAY) == HAL_OK);
}

int hal_spi_read(hal_spi_t spi, uint8_t *buf, int bufsize)
{
   int len = 0;

   while (len < bufsize)
   {
      if (dma_head == dma_tail)
      {
         hal_time_t tmo = hal_time_ms() + CFG_SPI_READ_TIMEOUT;

         // Wait for receive byte
         while(dma_head == dma_tail && hal_time_ms() < tmo);

         if (hal_time_ms() >= tmo)
            return 0;
      }

      buf[len++] = recv_buf[dma_head];
      dma_head = (dma_head + 1) & CFG_SPI_DMA_BUFFER_SIZE - 1;
   }

   return len;
}

/** Clear recv buffer */
int hal_spi_recv_clear(hal_spi_t spi)
{
   dma_head = dma_tail;
}

/** Enable/disable receive */
int hal_spi_recv_enable(hal_spi_t spi, bool enable)
{
   recv_enabled = enable;

   if (enable)
   {
      dma_head = 0;

      // Start receiving
      HAL_SPI_Receive_DMA(&hspi, recv_buf, CFG_SPI_DMA_BUFFER_SIZE);
   }
   else
   {
      // Stop receiving
      HAL_SPI_DMAStop(&hspi);
   }

   return 0;  
}

/** Register nonblocking data receive callback */
int hal_spi_recv(hal_spi_t spi, hal_spi_recv_cb_t _recv_cb)
{
   __disable_irq();
   recv_cb = _recv_cb;
   __enable_irq();

   return 0;
}


//
// IRQ callbacks
//

void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
   if (recv_cb != NULL)
      recv_cb(HAL_SPI0, recv_buf, CFG_SPI_DMA_BUFFER_SIZE / 2);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
   if (recv_cb != NULL)
      recv_cb(HAL_SPI0, &recv_buf[CFG_SPI_DMA_BUFFER_SIZE / 2], CFG_SPI_DMA_BUFFER_SIZE / 2);
}

/*
void SPI1_IRQHandler(void)
{
   HAL_SPI_IRQHandler(&hspi);
}
*/

void DMA1_Channel2_IRQHandler(void)
{
   HAL_DMA_IRQHandler(&hdma_spi1_rx);
}
