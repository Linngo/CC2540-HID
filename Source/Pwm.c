#include <ioCC2540.h>     
#include "PWM.h"    
  
#ifndef BV    
#define BV(n)      (1 << (n))    
#endif    
    
extern uint8 voice_on;
    
//******************************************************************************      
//name:             PWM_Init      
//introduce:        PWM的初始化     
//parameter:        none     
//return:           none    
//author:       甜甜的大香瓜    
//changetime:       2016.01.05    
//******************************************************************************  
void PWM_42K(void)    
{     
/*
  if(voice_on ==1)
  	{
		  P0DIR |= BV(5);         //P05定义为输出     
		  P0SEL |= BV(5);         //将P05设置为外设功能;       
  	}
  else
  	{
		  P0DIR |= BV(5);         //P05定义为输出     
		  P0SEL &= ~(1<<5);         //将P05设置为外设功能;        
		  P0  &= ~(1<<5);
	
  	}

  PERCFG |= 0x03;             // Move USART1&2 to alternate2 location so that T1 is visible
  PERCFG &= ~(1<<6);    //定时器1为外设位置2      
  // Initialize Timer 1
        

  T1CCTL3 = 0x14;             // IM = 0, CMP = Clear output on compare; Mode = Compare
  T1CNTL = 0;                 // Reset timer to 0;

   //必须设置，否则定时器不工作
  T1CCTL0 = 0x24;            // IM = 1, CMP = Clear output on compare; Mode = Compare 

   T1CC0H = 0x00;              // Ticks = 375 (1.5ms initial duty cycle)
  T1CC0L = 0x1D;              //             
  
  T1CC3H = 0x00;              // Ticks = 375 (1.5ms initial duty cycle)
  T1CC3L = 0x1D; 

 
   T1CTL = 0x0e;               // Div = 128, CLR, MODE = Suspended  
   */
}  

void PWM_27K(void)    
{  
/*
  if(voice_on ==1)
  	{
		  P0DIR |= BV(5);         //P05定义为输出     
		  P0SEL |= BV(5);         //将P05设置为外设功能;      
  	}
  else
  	{
		  P0DIR |= BV(5);         //P05定义为输出     
		  P0SEL &= ~(1<<5);         //将P05设置为外设功能;        
		  P0  &= ~(1<<5);
  	}
  
  PERCFG |= 0x03;             // Move USART1&2 to alternate2 location so that T1 is visible
  PERCFG &= ~(1<<6);    //定时器1为外设位置2      
  // Initialize Timer 1
        

  T1CCTL3 = 0x14;             // IM = 0, CMP = Clear output on compare; Mode = Compare
  T1CNTL = 0;                 // Reset timer to 0;

   //必须设置，否则定时器不工作
  T1CCTL0 = 0x24;            // IM = 1, CMP = Clear output on compare; Mode = Compare 

   T1CC0H = 0x00;              // Ticks = 375 (1.5ms initial duty cycle)
  T1CC0L = 0x2D;              //             
  
  T1CC3H = 0x00;              // Ticks = 375 (1.5ms initial duty cycle)
  T1CC3L = 0x2D; 

 
   T1CTL = 0x0e;               // Div = 128, CLR, MODE = Suspended  
   */
}    
    
  
 void PWM_Stop(void)    
{     
/*
  P0DIR |= BV(5);         //P05定义为输出     
  P0SEL &= ~(1<<5);         //将P05设置为外设功能;        
  P0  &= ~(1<<5);
 
   T1CTL = 0x0c;               // Div = 128, CLR, MODE = Suspended  
   */
}   
   
    
    
  

