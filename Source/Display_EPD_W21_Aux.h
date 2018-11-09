
#ifndef _DISPLAY_EPD_W21_AUX_H_
#define _DISPLAY_EPD_W21_AUX_H_
#include  "hal_adc.h"


//#define EPD_W21_MOSI_0	GPIO_ResetBits(GPIOD, GPIO_Pin_10)
//#define EPD_W21_MOSI_1	GPIO_SetBits(GPIOD, GPIO_Pin_10)

//#define EPD_W21_CLK_0	GPIO_ResetBits(GPIOD, GPIO_Pin_9)
//#define EPD_W21_CLK_1	GPIO_SetBits(GPIOD, GPIO_Pin_9)

#define EPD_W21_CS_0	P2_4  =0
#define EPD_W21_CS_1	P2_4  =1

#define EPD_W21_DC_0	P2_0  =0
#define EPD_W21_DC_1	P2_0  =1

//#define EPD_W21_RST_0	GPIO_ResetBits(GPIOE, GPIO_Pin_14)
//#define EPD_W21_RST_1	GPIO_SetBits(GPIOE, GPIO_Pin_14)

//#define EPD_W21_BS_0	GPIO_ResetBits(GPIOE, GPIO_Pin_11)

#define EPD_W21_BUSY_LEVEL 0
#define isEPD_W21_BUSY P2_3  



void driver_delay_xms(unsigned long xms);
void EPD_GPIOinit(void);
#endif
/***********************************************************
						end file
***********************************************************/


