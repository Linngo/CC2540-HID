#include "Led.h"
#include  "hal_adc.h"

void GLed_onoff (uint8 tmp)
{
    if(tmp ==1)
    	{
    		//CLKCONCMD = 0xA8;
    		//P2DIR |= (1<<3);         //P05����Ϊ���     
  		//P2SEL &= ~(1<<1);         //��P05����Ϊ���蹦��;        
 		//P2_3 =0;
    	}
	else
	{
		//CLKCONCMD = 0xA8;
		//P2DIR |= (1<<3);         //P05����Ϊ���     
  		//P2SEL &= ~(1<<1);         //��P05����Ϊ���蹦��;        
 		//P2_3 =1;
		
	}
	
}