#include "Display_EPD_W21_Aux.h"

extern uint8 LCD_exist ;

void driver_delay_xms(unsigned long xms)	
{	
    unsigned long i = 0 , j=0;

    for(j=0;j<xms;j++)
	{
        for(i=0; i<256*40; i++);
    }
}
void EPD_GPIOinit(void)
{
		P2DIR &= ~ (1<<3);         //P05����Ϊ����   LCD BUSY
  		P2SEL &= ~(1<<1);         //��P05����Ϊ���蹦��;
  		if(P2_3 == 0)
  			{
  				LCD_exist = 1;
  			}
		else
			{
				LCD_exist = 0;
			}

		P2DIR |=  (1<<4);         //P05����Ϊ����   SPI_CS_LCD
  		P2SEL &= ~(1<<2);         //��P05����Ϊ���蹦��;     
		P2_4  =1;

		P2DIR |=  (1<<0);         //P05����Ϊ����   LCD_D/C
  		P2SEL &= ~(1<<0);         //��P05����Ϊ���蹦��;  

		P1DIR |=  (1<<5);         //P05����Ϊ����   SPI_CS_LCD
  		P1SEL &= ~(1<<5);         //��P05����Ϊ���蹦��;     
		

		P1DIR |=  (1<<3);         //P05����Ϊ����   SPI_CS_LCD
  		P1SEL &= ~(1<<3);         //��P05����Ϊ���蹦��;     
		

}
/***********************************************************
						end file
***********************************************************/

