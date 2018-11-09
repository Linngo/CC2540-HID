#include "Led.h"
#include  "hal_adc.h"

void GLed_onoff (uint8 tmp)
{
    if(tmp ==1)
    	{
    		//CLKCONCMD = 0xA8;
    		//P2DIR |= (1<<3);         //P05定义为输出     
  		//P2SEL &= ~(1<<1);         //将P05设置为外设功能;        
 		//P2_3 =0;
    	}
	else
	{
		//CLKCONCMD = 0xA8;
		//P2DIR |= (1<<3);         //P05定义为输出     
  		//P2SEL &= ~(1<<1);         //将P05设置为外设功能;        
 		//P2_3 =1;
		
	}
	
}