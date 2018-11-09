/******************************************************************************

 @file  hidemukbd_Main.c

 @brief This file contains the main and callback functions for the
        HID emulated keyboard sample application.

 Group: WCS, BTS
 Target Device: CC2540, CC2541

 ******************************************************************************
 
 Copyright (c) 2011-2016, Texas Instruments Incorporated
 All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights
 granted under the terms of a software license agreement between the user
 who downloaded the software, his/her employer (which must be your employer)
 and Texas Instruments Incorporated (the "License"). You may not use this
 Software unless you agree to abide by the terms of the License. The License
 limits your use, and you acknowledge, that the Software may not be modified,
 copied or distributed unless embedded on a Texas Instruments microcontroller
 or used solely and exclusively in conjunction with a Texas Instruments radio
 frequency transceiver, which is integrated into your product. Other than for
 the foregoing purpose, you may not use, reproduce, copy, prepare derivative
 works of, modify, distribute, perform, display or sell this Software and/or
 its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
 NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
 NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
 LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
 OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
 OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 Should you have any questions regarding your right to use this Software,
 contact Texas Instruments Incorporated at www.TI.com.

 ******************************************************************************
 Release Name: ble_sdk_1.4.2.2
 Release Date: 2016-06-09 06:57:10
 *****************************************************************************/

/**************************************************************************************************
 *                                           Includes
 **************************************************************************************************/
/* Hal Drivers */
#include "hal_types.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "hal_drivers.h"
#include "hal_led.h"

/* OSAL */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"
#include "osal_snv.h"
#include "OnBoard.h"
#include "Display_EPD_W21_Aux.h"

/**************************************************************************************************
 * @fn          main
 *
 * @brief       Start of application.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void driver_delay_xms2(unsigned long xms)	
{	
    unsigned long i = 0 , j=0;

    for(j=0;j<xms;j++)
	{
        for(i=0; i<256*40; i++);
    }
}
extern uint8 scan_head ;
void detectsacnhed(void)
{

	  P0DIR |= (1<<3);         //P04定义为输出     
  	  P0SEL &= ~(1<<3);         //将P04设置为IO;   
  	  P0_3 = 0x0;
  	  P0DIR |= (1<<4);         //P04定义为输出     
  	  P0SEL &= ~(1<<4);         //将P04设置为IO;   
  	  P0_4 = 0x1;  
	  driver_delay_xms2(3885); //driver_delay_xms2(28000);   28000   108MS
          
	  P2INP |= (1<<6);          // PUT DOWN
	  P1SEL &= ~(1<<7);        //IO
	  P1DIR &= ~(1<<7);         //input
	  if(P1_7 == 0)
	  	{
			scan_head = 0;
	  	}
	  else
	  	{
	  		scan_head = 1;
	  	}
           P2INP &= ~(1<<6);          // PUT UP
           P0INP |= (1<<2);          // PUT DOWN
}
void Poweronoff(void)
{
//	uint32 i=0,j=0;
      P1_0 = 0x0;
      P1SEL = P1SEL & 0xFC;
      P1DIR = P1DIR & 0xFC;
	P1DIR = P1DIR |  0x1;

	  P0DIR |= (1<<5);         //P05定义为输出     
  	  P0SEL &= ~(1<<5);         //将P05设置为外设功能;        
        P0  &= ~(1<<5);
  
	 //for(i=0;i<1000;i++)   1ms



	  P1_0 = 0x1;

	  P1DIR &= ~ (1<<1);         //P05定义为输出     
  	  P1SEL &= ~(1<<1);         //将P05设置为外设功能;       

	  P0DIR |=  (1<<6);         //P05定义为输出     
  	  P0SEL &= ~(1<<6);         //将P05设置为外设功能;    

	  P0DIR |= (1<<1);         //P01定义为输出     
  	  P0SEL &= ~(1<<1);         //将P01设置为IO;   

	  P0DIR &= ~(1<<7);         //P07定义为输入
  	  P0SEL &= ~(1<<7);         //将P07设置为外设功能;    

	  P0DIR &= ~(1<<5);         //P07定义为输入
  	  P0SEL &= ~(1<<5);         //将P07设置为外设功能;   
          

	  

           P0_4 = 0x0; 
	  

	  P0DIR &= ~(1<<1);        //P01定义为输出     
  	  P0SEL &= ~(1<<1);         //将P01设置为IO;   
	  P0INP |= (1<<1);
  	  
	  P0DIR &= ~(1<<2);         //P04定义为输出     
  	  P0SEL &= ~(1<<2);         //将P04设置为IO;  

	P0_3 = 0x0;
	driver_delay_xms2(10);	
	P0_3 = 0x1;
	driver_delay_xms2(10);

	P0_3 = 0x0;
	driver_delay_xms2(10);	
	P0_3 = 0x1;
	driver_delay_xms2(10);


	P0_3 = 0x0;
	driver_delay_xms2(10);	
	P0_3 = 0x1;
	driver_delay_xms2(10);

	P0_3 = 0x0;
	driver_delay_xms2(10);	
	P0_3 = 0x1;
	driver_delay_xms2(10);

	P0_3 = 0x0;
	driver_delay_xms2(10);	
	P0_3 = 0x1;
	driver_delay_xms2(10);


	P0_3 = 0x0;
	driver_delay_xms2(10);	
	P0_3 = 0x1;
	driver_delay_xms2(10);


	P0_3 = 0x0;
	driver_delay_xms2(10);	
	P0_3 = 0x1;
	driver_delay_xms2(10);


	P0_3 = 0x0;
	driver_delay_xms2(10);	
	P0_3 = 0x1;
	driver_delay_xms2(10);
	P0_3 = 0x0;	
	
  	  EPD_GPIOinit();
}
int main(void)
{
  /* Initialize hardware */

  HAL_BOARD_INIT();

  detectsacnhed();
  // Initialize board I/O
  InitBoard( OB_COLD );

  /* Initialze the HAL driver */
  HalDriverInit();

  /* Initialize NV system */
  osal_snv_init();
  Poweronoff();
  /* Initialize LL */

  /* Initialize the operating system */
  osal_init_system();

  /* Enable interrupts */
  HAL_ENABLE_INTERRUPTS();

  // Final board initialization
  InitBoard( OB_READY );

  #if defined ( POWER_SAVING )
    osal_pwrmgr_device( PWRMGR_BATTERY );
  #endif
   // usbHidInit();
  /* Start OSAL */
  osal_start_system(); // No Return from here

  return 0;
}

/**************************************************************************************************
                                           CALL-BACKS
**************************************************************************************************/


/*************************************************************************************************
**************************************************************************************************/
