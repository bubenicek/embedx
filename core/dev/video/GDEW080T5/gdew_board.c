
#include "system.h"

#include "gdew_board.h"
#include "data.h"

TRACE_TAG(gdew);


void Delay(unsigned long nCount)
{
	volatile unsigned long i;
	for(i = 0; i < nCount; i++);
}

void Delay1ms(unsigned long ms)
{	
	for(;ms;ms--)
	{
		hal_delay_ms(1);
	}
}

void Delay100ms(unsigned long cnt)
{
  for(; cnt != 0; cnt--)
	  hal_delay_ms(100);
}

void IO_Init(void)
{
	SYS_WAKEUP_L;
	LED_01_OFF;
	LED_02_OFF;

	FLASH_CS_H;

	AVT_RST_L;
	AVT_HDC_L;
	AVT_HWE_H;
	AVT_HRD_H;
	AVT_HCS_H;
	AVT_DAT_SETIN;

	TPS_WAKEUP_L;
	TPS_PWRCOM_L;
	TPS_PWRUP_L;
	TPS_SCL_H;
	TPS_SDA_H;
}

void SYS_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	

	GPIO_InitStructure.Pin = PORT_A_OUT_PP; 
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_EPD;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);	

	GPIO_InitStructure.Pin = PORT_B_OUT_PP; 
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_EPD;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = PORT_B_OUT_OD; 
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Speed = GPIO_SPEED_EPD; 
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = PORT_C_OUT_PP; 
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_EPD;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = PORT_D_OUT_PP; 
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_EPD;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = PORT_D_IN_NOPULL; 
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_EPD;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	IO_Init();
}

void Debug_str(char *s)
{
	TRACE_PRINTF("%s", s);
}

void Debug_hex(unsigned int dat)
{
	char buf[8];
	unsigned char i;
	
	buf[0]='0';
	buf[1]='x';
	i=(dat>>12)&0x0f;
	if (i>9)
		buf[2]='A'-10+i;
	else
		buf[2]='0'+i;

	i=(dat>>8)&0x0f;
	if (i>9)
		buf[3]='A'-10+i;
	else
		buf[3]='0'+i;

	i=(dat>>4)&0x0f;
	if (i>9)
		buf[4]='A'-10+i;
	else
		buf[4]='0'+i;

	i=dat&0x0f;
	if (i>9)
		buf[5]='A'-10+i;
	else
		buf[5]='0'+i;

	buf[6]=' ';
	buf[7]=0;

	Debug_str(buf);
}

void Debug_dec(unsigned int dat)
{
	char buf[7];
	
	buf[0]='0'+dat/10000;
	buf[1]='0'+(dat% 10000) / 1000;
	buf[2]='0'+(dat% 1000) / 100;
	buf[3]='0'+(dat% 100) / 10;
	buf[4]='0'+(dat% 10) ;
	buf[5]=' ';
	buf[6]=0;
	Debug_str(buf);
}

void SPIx_Init(void)
{
	SPI_HandleTypeDef hspi;
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_SPI1_CLK_ENABLE();


   // SPI1 GPIO Configuration    
   // PA5     ------> SPI1_SCK
   // PA6     ------> SPI1_MISO
   // PA7     ------> SPI1_MOSI 
   //
   GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = GPIO_PIN_6;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   // Configure SPI periph
   hspi.Instance = SPI1;
   hspi.Init.Mode = SPI_MODE_MASTER;
   hspi.Init.Direction = SPI_DIRECTION_2LINES;
   hspi.Init.DataSize = SPI_DATASIZE_8BIT;
   hspi.Init.CLKPolarity = SPI_POLARITY_HIGH;
   hspi.Init.CLKPhase = SPI_PHASE_2EDGE;
   hspi.Init.NSS = SPI_NSS_SOFT;
   hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
   hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
   hspi.Init.TIMode = SPI_TIMODE_DISABLE;
   hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
   hspi.Init.CRCPolynomial = 7;
   HAL_SPI_Init(&hspi);
}
