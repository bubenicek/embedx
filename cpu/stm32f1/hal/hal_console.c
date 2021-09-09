
#include "system.h"

#ifndef CFG_HAL_CONSOLE_USE_DMA
#define CFG_HAL_CONSOLE_USE_DMA 0
#endif

#if defined(CFG_HAL_CONSOLE_SWO_ENABLE) && (CFG_HAL_CONSOLE_SWO_ENABLE == 1)

#include "swo.h"

int hal_console_init(void)
{
    swo_init(CFG_HAL_CONSOLE_SWO_BAUDRATE);
    return 0;
}

void hal_console_putchar(char c)
{
    swo_putchar(c);
}

void hal_console_flush(void)
{
}

#else

#ifndef CFG_HAL_CONSOLE_BUFFER_SIZE
#define CFG_HAL_CONSOLE_BUFFER_SIZE 1024
#endif

static UART_HandleTypeDef huart;

#if defined(CFG_HAL_CONSOLE_USE_DMA) && (CFG_HAL_CONSOLE_USE_DMA == 1)
static char txbuf[CFG_HAL_CONSOLE_BUFFER_SIZE];
static uint16_t txbuf_head = 0;
static uint16_t txbuf_tail = 0;

static DMA_HandleTypeDef hdma_usart2_tx;
static uint8_t dma_buf[CFG_HAL_CONSOLE_BUFFER_SIZE];
static volatile bool dma_tx_pending = false;
#endif

int hal_console_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Peripheral clock enable
    __HAL_RCC_USART1_CLK_ENABLE();

#if defined(CFG_HAL_CONSOLE_USE_DMA) && (CFG_HAL_CONSOLE_USE_DMA == 1)
    __HAL_RCC_DMA1_CLK_ENABLE();
#endif

    // USART1 GPIO Configuration
    GPIO_InitStruct.Pin = DEBUG_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DEBUG_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DEBUG_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DEBUG_RX_GPIO_Port, &GPIO_InitStruct);

    huart.Instance = DEBUG_UART;
    huart.Init.BaudRate = 115200;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart);

#if defined(CFG_HAL_CONSOLE_USE_DMA) && (CFG_HAL_CONSOLE_USE_DMA == 1)
    // USART2 DMA Init
    hdma_usart2_tx.Instance = DMA1_Channel7;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
        return -1;
    }

    __HAL_LINKDMA(&huart, hdmatx, hdma_usart2_tx);

    // DMA interrupt init
    HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

#endif

    return 0;
}

#if defined(CFG_HAL_CONSOLE_USE_DMA) && (CFG_HAL_CONSOLE_USE_DMA == 1)

static void start_dma_tx(void)
{
    int len = 0;

    // TODO: optimalizace, posialat head -> tail, 0 -> tail
    while (txbuf_tail != txbuf_head && len < CFG_HAL_CONSOLE_BUFFER_SIZE)
    {
        dma_buf[len++] = txbuf[txbuf_tail];
        txbuf_tail = (txbuf_tail + 1) & (CFG_HAL_CONSOLE_BUFFER_SIZE - 1);
    }

    if (len > 0)
    {
        dma_tx_pending = true;
        HAL_UART_Transmit_DMA(&huart, dma_buf, len);
    }
}

void hal_console_putchar(char c)
{
    txbuf[txbuf_head] = c;
    txbuf_head = (txbuf_head + 1) & (CFG_HAL_CONSOLE_BUFFER_SIZE - 1);

    __disable_irq();
    bool txen = (dma_tx_pending == false) && c == '\n';
    __enable_irq();

    if (txen)
        start_dma_tx();
}

int hal_console_getchar(void)
{
    int c = -1;

    if (__HAL_UART_GET_FLAG(&huart, UART_FLAG_RXNE) == SET)
    {
        c = huart.Instance->DR;
    }

    return c;
}

void hal_console_flush(void)
{
}

void DMA1_Channel7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart2_tx);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_DMAStop(huart);
    dma_tx_pending = false;
    start_dma_tx();
}

#else

void hal_console_putchar(char c)
{
    while (__HAL_UART_GET_FLAG(&huart, UART_FLAG_TXE) == RESET)
        ;
    huart.Instance->DR = c;
}

int hal_console_getchar(void)
{
    int c = -1;

    if (__HAL_UART_GET_FLAG(&huart, UART_FLAG_RXNE) == SET)
    {
        c = huart.Instance->DR;
    }

    return c;
}

void hal_console_flush(void)
{
}

#endif

#endif // CFG_HAL_CONSOLE_SWO_ENABLE