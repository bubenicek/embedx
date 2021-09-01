/**
  ******************************************************************************
  * @file    stm32f4x7_eth_conf.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-October-2011
  * @brief   Configuration file for the STM32F4x7 Ethernet driver.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4x7_ETH_CONF_H
#define __STM32F4x7_ETH_CONF_H

#include "stm32f4xx.h"

#if CFG_CMSIS_OS_API
#include "cmsis_os.h"
#endif


/* MII and RMII mode selection, for STM324xG-EVAL Board(MB786) RevB ***********/
#define RMII_MODE  // User have to provide the 50 MHz clock by soldering a 50 MHz
                     // oscillator (ref SM7745HEV-50.0M or equivalent) on the U3
                     // footprint located under CN3 and also removing jumper on JP5.
                     // This oscillator is not provided with the board.
                     // For more details, please refer to STM3240G-EVAL evaluation
                     // board User manual (UM1461).


//#define MII_MODE
/* Uncomment the define below to clock the PHY from external 25MHz crystal (only for MII mode) */
#ifdef 	MII_MODE
 #define PHY_CLOCK_MCO
#endif

#ifndef CFG_HAL_ETH_IRQ_PRIORITY
#define CFG_HAL_ETH_IRQ_PRIORITY       configLIBRARY_KERNEL_INTERRUPT_PRIORITY
#endif

#define DP83848_PHY_ADDRESS            0x00  // 1 /* Relative to STM324xG-EVAL Board */

/* Specific defines for EXTI line, used to manage Ethernet link status */
#define ETH_LINK_EXTI_LINE             EXTI_Line3
#define ETH_LINK_EXTI_PORT_SOURCE      EXTI_PortSourceGPIOA
#define ETH_LINK_EXTI_PIN_SOURCE       EXTI_PinSource3
#define ETH_LINK_EXTI_IRQn             EXTI3_IRQn
#define ETH_LINK_PIN                   GPIO_Pin_3
#define ETH_LINK_GPIO_PORT             GPIOA
#define ETH_LINK_GPIO_CLK              RCC_AHB1Periph_GPIOA


/* Uncomment the line below when using time stamping and/or IPv4 checksum offload */
#define USE_ENHANCED_DMA_DESCRIPTORS

/* Uncomment the line below if you want to use user defined Delay function
   (for precise timing), otherwise default _eth_delay_ function defined within
   the Ethernet driver is used (less precise timing) */
//#define USE_Delay

#ifdef USE_Delay
  #include "main.h"                /* Header file where the Delay function prototype is exported */
  #define _eth_delay_    Delay     /* User can provide more timing precise _eth_delay_ function */
#else
  #define _eth_delay_    ETH_Delay /* Default _eth_delay_ function with less precise timing */
#endif


/* Uncomment the line below to allow custom configuration of the Ethernet driver buffers */
#define CUSTOM_DRIVER_BUFFERS_CONFIG

#ifdef  CUSTOM_DRIVER_BUFFERS_CONFIG
/* Redefinition of the Ethernet driver buffers size and count */
 #define ETH_RX_BUF_SIZE    ETH_MAX_PACKET_SIZE /* buffer size for receive */
 #define ETH_TX_BUF_SIZE    ETH_MAX_PACKET_SIZE /* buffer size for transmit */
 #define ETH_RXBUFNB        16                  /* Rx buffers of size ETH_RX_BUF_SIZE */
 #define ETH_TXBUFNB        32                  /* Tx buffers of size ETH_TX_BUF_SIZE */
#endif


/* PHY configuration section **************************************************/
/* PHY Reset delay */
#define PHY_RESET_DELAY    ((uint32_t)0x000FFFFF)
/* PHY Configuration delay */
#define PHY_CONFIG_DELAY   ((uint32_t)0x00FFFFFF)

/* The PHY status register value change from a PHY to another, so the user have
   to update this value depending on the used external PHY */
#define PHY_SR    ((uint16_t)16) /* Value for DP83848 PHY */

/* The Speed and Duplex mask values change from a PHY to another, so the user
   have to update this value depending on the used external PHY */
#define PHY_SPEED_STATUS            ((uint16_t)0x0002) /* Value for DP83848 PHY */
#define PHY_DUPLEX_STATUS           ((uint16_t)0x0004) /* Value for DP83848 PHY */



/* MAC ADDRESS*/
#define MAC_ADDR0   0x7C
#define MAC_ADDR1   0x1E
#define MAC_ADDR2   0xB3
#define MAC_ADDR3   0x01
#define MAC_ADDR4   0x03
#define MAC_ADDR5   0xDC


#endif /* __STM32F4x7_ETH_CONF_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

