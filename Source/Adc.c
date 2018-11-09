#include "Adc.h"
#include  "hal_adc.h"

uint16 Getbatlevel (void)
{
	uint16 adc=0;
	P0DIR &= ~(1<<0);         //P00定义为输入
 	P0SEL |= (1<<0);         //将P00设置为外设功能; 
        P0INP |= 0x01;            //P0.0 P0.0高阻态
 	APCFG |= (1<<0);
	HalAdcSetReference ( HAL_ADC_REF_AVDD );
	adc = HalAdcRead ( HAL_ADC_CHN_AIN0, HAL_ADC_RESOLUTION_12 );
        return adc;
	
}