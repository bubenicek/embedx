
#include "system.h"


extern osMutexId hci_mutex;


// Leave critical section
void Enable_IRQ(void)
{
   	osMutexRelease(hci_mutex);
}

// Enter critical section
void Disable_IRQ(void)
{
   	osMutexWait(hci_mutex, osWaitForever);
}

void Hal_Write_Serial(const void* data1, const void* data2, uint16_t n_bytes1, uint16_t n_bytes2)
{
    hal_uart_write(BLUENRG_UART, (void *)data1, n_bytes1);
    hal_uart_write(BLUENRG_UART, (void *)data2, n_bytes2);
}

int HAL_Read_Serial(void *buf, int bufsize, int timeout)
{
    return hal_uart_read(BLUENRG_UART, buf, bufsize, timeout);
}