#include "Adc.h"
#include  "hal_adc.h"

uint16 Getbatlevel (void)
{
	uint16 adc=0;
	P0DIR &= ~(1<<0);         //P00����Ϊ����
 	P0SEL |= (1<<0);         //��P00����Ϊ���蹦��; 
        P0INP |= 0x01;            //P0.0 P0.0����̬
 	APCFG |= (1<<0);
	HalAdcSetReference ( HAL_ADC_REF_AVDD );
	adc = HalAdcRead ( HAL_ADC_CHN_AIN0, HAL_ADC_RESOLUTION_12 );
        return adc;
	
}