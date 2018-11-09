/******************************************************************************

 @file  hidemukbd.c

 @brief This file contains the HID emulated keyboard sample application for use
        with the CC2540 Bluetooth Low Energy Protocol Stack.

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


/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "hal_led.h"
#include "hal_key.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "gatt_profile_uuid.h"
#include "linkdb.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "hidkbdservice.h"
#include "devinfoservice.h"
#include "battservice.h"
#include "hiddev.h"
#include "hidemukbd.h"
#include "Npi.h"
// HID includes
#include "usb_framework.h"
#include "usb_hid.h"
#include "usb_hid_reports.h"
#include "usb_suspend.h"
#include "Pwm.h"
#include "Adc.h"
#include "Led.h"
#include "spi.h"
#include "OSAL_Timers.h"
#include "OSAL_Clock.h"
#include "hal_flash.h"
#include "Display_EPD_W21.h"
#include "simpleGATTprofile.h"

/*********************************************************************
 * MACROS
 */

// Selected HID keycodes
#define KEY_RIGHT_ARROW             0x4F
#define KEY_LEFT_ARROW              0x50
#define KEY_NONE                    0x00

// Selected HID LED bitmaps
#define LED_NUM_LOCK                0x01
#define LED_CAPS_LOCK               0x02

// Selected HID mouse button values
#define MOUSE_BUTTON_1              0x01
#define MOUSE_BUTTON_NONE           0x00

// HID keyboard input report length
#define HID_KEYBOARD_IN_RPT_LEN     8

// HID LED output report length
#define HID_LED_OUT_RPT_LEN         1

// HID mouse input report length
#define HID_MOUSE_IN_RPT_LEN        5

/*********************************************************************
 * CONSTANTS
 */

// HID idle timeout in msec; set to zero to disable timeout
#define DEFAULT_HID_IDLE_TIMEOUT              1800000
// 40 0 2000
// Minimum connection interval (units of 1.25ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     24

// Maximum connection interval (units of 1.25ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     24

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         4

// Supervision timeout value (units of 10ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         10

// Default passcode
#define DEFAULT_PASSCODE                      0

// Default GAP pairing mode
#define DEFAULT_PAIRING_MODE                  GAPBOND_PAIRING_MODE_WAIT_FOR_REQ

// Default MITM mode (TRUE to require passcode or OOB when pairing)
#define DEFAULT_MITM_MODE                     FALSE

// Default bonding mode, TRUE to bond
#define DEFAULT_BONDING_MODE                  TRUE

// Default GAP bonding I/O capabilities
#define DEFAULT_IO_CAPABILITIES               GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT

// Battery level is critical when it is less than this %
#define DEFAULT_BATT_CRITICAL_LEVEL           6
#define CC_ID_D0  0x3589   // 任意的一个数 
#define CC_ID_D1  0x3978   // 任意的一个数 
#define CC_ID_D2  0x4568   // 任意的一个数 
#define CC_ID_D3  0x5694   // 任意的一个数 
#define CC_ID_D4  0x6123   // 任意的一个数 
#define CC_ID_D5  0x7805   // 任意的一个数 
#define CC_ID_D6  54   // 任意的一个数 
volatile uint16 CC2540_ID_addr[6]={0x780E - CC_ID_D0,0x780F - CC_ID_D1,0x7810 - CC_ID_D2,0x7811 - CC_ID_D3,0x7812 - CC_ID_D4,0x7813 - CC_ID_D5};
volatile uint16 CC2540load_ID_addr[3]={0xF800 - CC_ID_D0,0xF802 - CC_ID_D1,124 -CC_ID_D6};

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Task ID
uint8 hidEmuKbdTaskId;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// GAP Profile - Name attribute for SCAN RSP data
static uint8 scanData[] =
{
  0x08,                             // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,   // AD Type = Complete local name
  'N',
  'T',
  '1',
  '0',
  '0',
  '0',
#ifdef EPDDOT152	
  'C',		
#endif
#ifdef EPDDOT200	
  'B',	
#endif
};

static uint8 scan_mobileData[] =
{
  0x06,                             // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,   // AD Type = Complete local name
  'N',
  'P',
  'W',
  '1',
 #ifdef EPDDOT152	
  'C',		
#endif
#ifdef EPDDOT200	
  'B',	
#endif
};

// Advertising data
static uint8 advData[] =
{
  // flags
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // appearance
  0x03,   // length of this data
  GAP_ADTYPE_APPEARANCE,
  LO_UINT16(GAP_APPEARE_HID_KEYBOARD),
  HI_UINT16(GAP_APPEARE_HID_KEYBOARD),

  // service UUIDs
  0x05,   // length of this data
  GAP_ADTYPE_16BIT_MORE,
  LO_UINT16(HID_SERV_UUID),
  HI_UINT16(HID_SERV_UUID),
  LO_UINT16(BATT_SERV_UUID),
  HI_UINT16(BATT_SERV_UUID)
};

// Device name attribute value
static  uint8 attDeviceNameS[GAP_DEVICE_NAME_LEN] = "NT1000C";
static  uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "NPW1C";
// HID Dev configuration
static hidDevCfg_t hidEmuKbdCfg =
{
  DEFAULT_HID_IDLE_TIMEOUT,   // Idle timeout
  HID_KBD_FLAGS               // HID feature flags
};

// TRUE if boot mouse enabled
static uint8 hidBootMouseEnabled = FALSE;

static uint8 keyboardtrans[1024]={0x0,0x0,0x0,0x0,0x0};
static uint16  keyboard_read=0,keyboard_write=0,keyboard_read_tmp =2000,keyboardread_copy=0;
uint8 Bleadapter=0,unicode_tmp[4]={0,0,0,0};
static uint8 g_bIsEng = 0,key_chinese_value[5]={0,0,0,0,0},keyvlauetmp[12]={0,0,0,0,0,0,0,0,0,0,0,0},keyvlauetmp_count=0,USB_conected =0,chinese_value =0,datavlaue[10]={0x62,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,0x60,0x61};;
static uint16 HIDkeyboardvalue=0,USB_datect=0,HIDkeyboardvalue_tmp=0,BLE_datect=0;
uint32 Bat_level=4200,Batlevel_tmp=4200,unicode_address=0,AUTOpowertime=0,timelong_tmp=0,timelong_count;
uint8 LedG_onoff=0,LedG_tol=0,shift_key=0;
uint8 P06time=0,Buzzer_on=1,Powerkey=1,powerofftime=0,buzzer_continus=0;
uint8 Eng_send=0,key_endvalue=0,chongfukey=128,eepromear_ok=0;
uint8  Sector_wip=0,data_suspend=0,Eeprom_add_change =0,wip_data_count =0,wip_data_tmp =0,   Wip_copy[260];
uint32  Eeprom_write=0x38000,Eeprom_read=0x38000,Eepromread_tmp=0x38000,Eeprom_ear=0,Eeprom_addessear=0x38000;
uint32  FLASH_READADDRESS =0,Eeprom_adda_copy=0;
uint8    Eeprom_full =0,power_fisrt=1,BLE_conected =0;
uint8    ledGcount=0,camera_ok=0,batadccount=0,camera_gbk=0,batlow_powerdown=0,batlow_count=0,batlowvoice =0;
uint8    batcharge =0 ;
uint32    charge_datectcount = 10000;
uint8 ASCIIvalue[128]={0,0,0,0,0,0,0,0,0x2A,0x2B ,   //9   
	0x28,0x2B,0,0x28,0,0,0,0,0,0,   //19
	0,0,0,0,0,0,0,0,0,0,  //29
	0,0,0x2C,0x9D ,0xB3,0x9f,0xa0,0xa1,0xa3,0x34 ,  //39
	0xa5,0xa6,  0xa4 ,0xad, 0x36 ,0x56,  0x37,0x38,  39, 30,  //49
	31,32,33,34,35, 36 ,37,38,0xB2,0x33,  //59 
	0xB5,0x2E ,0xB6,0xB7, 0x9E,      0x83, 0x84,0x85, 0x86,0x87,  //69
	0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,  //79
	0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,  //89
	0x9C,0x2F ,   0x31,0x30,   0xA2,0xAC,  0,   4,   5,6,  //99
	7,8,   9,10,   11,12,   13,14,   15,16,  //109
	17,18,   19,20,   21,22,  23,24,  25,26,  //119
	27,28,   29,0xAE,   0,0xAF,   0,0x63        // 127

};//~;,.nyear2012w
uint8 cameravalue[] ={0x7E,0x3B,0x2C,0x2E,0x6E,0x79,0x65,0x61,0x72,0x32,0x30,0x31,0x32,0x32},cameravalue_ok=0;
uint8 cameragbkvalue  =0x47;
uint8 camerauncodevalue  =0x55;
uint8 cameraearflash  =0x46,Earflash=0;
uint16 Earflash_timecount =0;
uint8 cameravoice   =0x59 ,voice_on=1;
uint8 Poweroff_1scan   =0x31 ,Poweroff_5scan   =0x35 ;
uint8 bonddelteasci = 0x45 ,poweroffbond =0;
uint8 dataoffline_on = 0x4B,dataline_off =0x42,dataoffline =0;
uint16 Poweroffmscan=300;
uint8 tmp_buffer[21] ;
uint8 sector_buffer[12] ;
uint8  sector_chang=0;
uint32 Ble_ConnInterval=12;
uint8  Encrypted_SUCESS=1;
volatile uint8  voice_start=0,voice_tg1 = 0,voice_reset = 0;
uint8 LCD_exist = 0,LCD_chanel = 0;
uint8 Picture_on[3] = {0,0x03,0xFF};
uint8 sound_on[2] = {0xFF,0xFF};
uint8 retrue_BLE = 0;
uint8 Charging_mark = 0;
uint8 charge_quantity = 100;
uint8 keyrepeat = 0;
uint32 keycountrepeat  = 0;
bool scan_flag = 1;

uint8 Left_key = 0,Right_key = 0,leftright =0,njnjnjn = 0;
uint16 Leftcount_key = 0,Rightcount_key = 0;
uint8 scan_head = 0;
volatile uint32 display_picture = 0x1,displaypicture_on =0;
extern uint32 Flashpicture_address ;
extern uint8 earpicture_flash ;
extern uint8 BLEtime_out;

//////////////////////////////////////////
extern uint8 firmware_flash_task;
extern uint32 firmware_size;
extern uint32 firmware_addr;
extern uint8 firmware_crc;
/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void hidEmuKbd_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void hidEmuKbd_HandleKeys( uint8 shift, uint8 keys );
static void hidEmuKbd_ProcessGattMsg( gattMsgEvent_t *pMsg );
static void hidEmuKbdSendReport( uint8 keycode );
static void hidEmuKbdSendReport_chinese( uint8 keycode );
static void hidUSBKbdSendReport_chinese( uint8 keycode );
static void hidUSBKbdSendReport( uint8 keycode );
static void hidEmuKbdSendMouseReport( uint8 buttons );
static uint8 hidEmuKbdRcvReport( uint8 len, uint8 *pData );
static uint8 hidEmuKbdRptCB( uint8 id, uint8 type, uint16 uuid,
                             uint8 oper, uint8 *pLen, uint8 *pData );
static void hidEmuKbdEvtCB( uint8 evt );

static void simpleBLE_NpiSerial1Callback( uint8 port, uint8 events );

static uint16 crc16(uint16 crc, uint8 val);
static uint16 calcCRC(void);
/*********************************************************************
 * PROFILE CALLBACKS
 */

static hidDevCB_t hidEmuKbdHidCBs =
{
  hidEmuKbdRptCB,
  hidEmuKbdEvtCB,
  NULL
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      HidEmuKbd_Init
 *
 * @brief   Initialization function for the HidEmuKbd App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */

void Init_Watchdog(void) 
{ 
    WDCTL = 0x00;       //打开IDLE才能设置看门狗
    WDCTL |= 0x08;      //定时器间隔选择,间隔一秒
}

void FeetDog(void) 
{ 
    WDCTL = 0xa0;       //清除定时器。当0xA跟随0x5写到这些位，定时器被清除
    WDCTL = 0x50; 
}

void driver_delay_xms1(unsigned long xms)	
{	
    unsigned long i = 0 , j=0;

    for(j=0;j<xms;j++)
	{
        for(i=0; i<256*40; i++);
    }
}
void Flash_earaALL(void);
void FLASH_Initialization(void)
{
	uint8 tmp=0;

	Flash_SectorErase(0x20000);	
	
	tmp = (Eeprom_write >>24) & 0xFF;	
	Flash_WriteByte(0x20000, tmp );

	tmp = (Eeprom_write >>16) & 0xFF;	
	Flash_WriteByte(0x20001, tmp );

	tmp = (Eeprom_write >>8) & 0xFF;	
	Flash_WriteByte(0x20002, tmp );

	tmp = (Eeprom_write >>0) & 0xFF;	
	Flash_WriteByte(0x20003, tmp );
	

	tmp = (Eepromread_tmp >>24) & 0xFF;	
	Flash_WriteByte(0x20004, tmp );

	tmp = (Eepromread_tmp >>16) & 0xFF;	
	Flash_WriteByte(0x20005, tmp );

	tmp = (Eepromread_tmp >>8) & 0xFF;	
	Flash_WriteByte(0x20006, tmp );

	tmp = (Eepromread_tmp >>0)  & 0xFF;	
	Flash_WriteByte(0x20007, tmp );



	Flash_WriteByte(0x20008, camera_ok );

	Flash_WriteByte(0x20009, camera_gbk );

	Flash_WriteByte(0x2000a, voice_on );

	Flash_WriteByte(0x2000b, 17 );

	Flash_WriteByte(0x2000c, 11 );

	Flash_WriteByte(0x20000+14, Picture_on[0] );
	Flash_WriteByte(0x20000+15, Picture_on[1] );
	Flash_WriteByte(0x20000+16, Picture_on[2] );
	
	Flash_WriteByte(0x20000+17, sound_on[0] );
	Flash_WriteByte(0x20000+18, sound_on[1] );
	Flash_WriteByte(0x20000+19, BLEtime_out);

	if(Encrypted_SUCESS == 0)
		{
			Flash_earaALL();
		}
}

void FLASH_poweron_write(void)
{
	uint8 tmp=0;
	Flash_SectorErase(FLASH_READADDRESS);	
	
	tmp = (Eeprom_write >>24) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS, tmp );

	tmp = (Eeprom_write >>16) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+1, tmp );

	tmp = (Eeprom_write >>8) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+2, tmp );

	tmp = (Eeprom_write >>0) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+3, tmp );
	

	tmp = (Eepromread_tmp >>24) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+4, tmp );

	tmp = (Eepromread_tmp >>16) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+5, tmp );

	tmp = (Eepromread_tmp >>8) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+6, tmp );

	tmp = (Eepromread_tmp >>0) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+7, tmp );



	Flash_WriteByte(FLASH_READADDRESS+8, camera_ok );

	Flash_WriteByte(FLASH_READADDRESS+9, camera_gbk );

	Flash_WriteByte(FLASH_READADDRESS+10, voice_on );

	Flash_WriteByte(FLASH_READADDRESS+11, 17 );

	if((Poweroffmscan == 1800) ||(Poweroffmscan == 300))
		{
			Flash_WriteByte(FLASH_READADDRESS+12, 11 );
		}
	else
		{
			Flash_WriteByte(FLASH_READADDRESS+12, 00 );
		}
	if(dataoffline == 1 )
		{
			Flash_WriteByte(FLASH_READADDRESS+13, 00 );
		}
	Flash_WriteByte(FLASH_READADDRESS+14, Picture_on[0] );
	Flash_WriteByte(FLASH_READADDRESS+15, Picture_on[1] );
	Flash_WriteByte(FLASH_READADDRESS+16, Picture_on[2] );
	Flash_WriteByte(FLASH_READADDRESS+17, sound_on[0] );
	Flash_WriteByte(FLASH_READADDRESS+18, sound_on[1] );
	
}

void FLASH_write(void)
{
	uint8 tmp=0;
	Flash_SectorErase(FLASH_READADDRESS);	
	if(FLASH_READADDRESS == 0x20000)
		{
			FLASH_READADDRESS = 0x21000;
		}
	else if(FLASH_READADDRESS == 0x21000)
		{
			FLASH_READADDRESS = 0x22000;
		}
	else if(FLASH_READADDRESS == 0x22000)
		{
			FLASH_READADDRESS = 0x1FF000;
		}
	else
		{
			FLASH_READADDRESS = 0x20000;
		}

	Flash_SectorErase(FLASH_READADDRESS);	
	
	tmp = (Eeprom_write >>24) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS, tmp );

	tmp = (Eeprom_write >>16) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+1, tmp );

	tmp = (Eeprom_write >>8) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+2, tmp );

	tmp = (Eeprom_write >>0) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+3, tmp );
	

	tmp = (Eepromread_tmp >>24) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+4, tmp );

	tmp = (Eepromread_tmp >>16) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+5, tmp );

	tmp = (Eepromread_tmp >>8) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+6, tmp );

	tmp = (Eepromread_tmp >>0) & 0xFF;	
	Flash_WriteByte(FLASH_READADDRESS+7, tmp );



	Flash_WriteByte(FLASH_READADDRESS+8, camera_ok );

	Flash_WriteByte(FLASH_READADDRESS+9, camera_gbk );

	Flash_WriteByte(FLASH_READADDRESS+10, voice_on );

	Flash_WriteByte(FLASH_READADDRESS+11, 34 );

	if((Poweroffmscan == 1800) ||(Poweroffmscan == 300))
		{
			Flash_WriteByte(FLASH_READADDRESS+12, 11 );
		}
	else
		{
			Flash_WriteByte(FLASH_READADDRESS+12, 00 );
		}
	if(dataoffline == 1 )
		{
			Flash_WriteByte(FLASH_READADDRESS+13, 00 );
		}
	Flash_WriteByte(FLASH_READADDRESS+14, Picture_on[0] );
	Flash_WriteByte(FLASH_READADDRESS+15, Picture_on[1] );
	Flash_WriteByte(FLASH_READADDRESS+16, Picture_on[2] );
	Flash_WriteByte(FLASH_READADDRESS+17, sound_on[0] );
	Flash_WriteByte(FLASH_READADDRESS+18, sound_on[1] );
	Flash_WriteByte(FLASH_READADDRESS+19, BLEtime_out);

}

void Flash_earaALL(void)
{
			Eeprom_addessear=0x38000;	
			Earflash_timecount =0;
			do
				{
					if(Earflash_timecount < 25 )
						{
							P0_4 = 0x1;
						}
					else if(Earflash_timecount == 25)
						{
							P0_4 = 0x0;

							P0_3 = 0x0;
							driver_delay_xms1(10);	
							P0_3 = 0x1;
							driver_delay_xms1(10);
							P0_3 = 0x0;
						}
					else if(Earflash_timecount > 70 )
						{
							Earflash_timecount =0;

						}
					Earflash_timecount++;
					FeetDog();
					Flash_SectorErase(Eeprom_addessear);	
					Eeprom_addessear=Eeprom_addessear + 0x1000;
				}
			while(Eeprom_addessear < 0x1E0000);//0x1ff000
			Eeprom_write      = 0x38000;
			Eeprom_read       = 0x38000;
			Eepromread_tmp = 0x38000;

			Eeprom_addessear=0x38000;
				
			camera_ok=0;
			camera_gbk =0;
			voice_on =1;
					    if(scan_head == 1)
					    	{
							Poweroffmscan = 1800;
					    	}
					    else
					    	{
							Poweroffmscan = 300;
					    	}
			dataoffline =0;
			Picture_on[0]  = 0xFF ;
			Picture_on[1]  = 0x03;
			Picture_on[2]  = 0x00 ;
			sound_on[0]   = 0xFF ;
			sound_on[1]   = 0xFF ;
			FLASH_write();
			Earflash =0;
			Eeprom_full =0;
			P1_0 = 0x0;
			P0_6= 1;
			while(1)
				{
					FeetDog();
				}	
}
void FLash_all_readdata(void)
{
			Eeprom_addessear=0x38000;	
			do
				{

					
					Flash_FAST_ReadData(Eeprom_addessear, 1022, keyboardtrans);	
					
					if(keyboardtrans[0] != 0xFF)
						{
							voice_on =0;

							Flash_FAST_ReadData(Eeprom_addessear-25, 1022, keyboardtrans);
							voice_on =1;
							
							
						}
                                        Eeprom_addessear=Eeprom_addessear + 0x1000;
				}
			while(Eeprom_addessear < 0x1E0000);//0x1FF000
			
			Eeprom_addessear=0x38000;
}

void FLASH_read(void)
{
		Flash_FAST_ReadData(FLASH_READADDRESS, 21, tmp_buffer);



		Eeprom_write = BUILD_UINT32(tmp_buffer[3],tmp_buffer[2],tmp_buffer[1],tmp_buffer[0]);


		Eepromread_tmp = BUILD_UINT32(tmp_buffer[7],tmp_buffer[6],tmp_buffer[5],tmp_buffer[4]);


		Eeprom_read = Eepromread_tmp;

		//camera_ok     =tmp_buffer[8]; 
		camera_ok     =1;
		camera_gbk   =tmp_buffer[9] ;
		voice_on        =tmp_buffer[10] ;

		Picture_on[0]  = tmp_buffer[14] ;
		Picture_on[1]  = tmp_buffer[15] ;
		Picture_on[2]  = tmp_buffer[16] ;
		sound_on[0]   = tmp_buffer[17] ;
		sound_on[1]   = tmp_buffer[18] ;
		BLEtime_out = tmp_buffer[19];

		if((tmp_buffer[11] != 0x22) || (Encrypted_SUCESS == 0))
			{
				Flash_earaALL();                       
			}
		else
			{
				FLASH_poweron_write();
			}
		if(tmp_buffer[12] != 0x00)
			{
					    if(scan_head == 1)
					    	{
							Poweroffmscan = 1800;
					    	}
					    else
					    	{
							Poweroffmscan = 300;
					    	}
			}
		else
			{
				
					    if(scan_head == 1)
					    	{
							Poweroffmscan = 9000;
					    	}
					    else
					    	{
							Poweroffmscan = 600;
					    	}
			}

		if(tmp_buffer[13] != 0xFF)
			{
				dataoffline =1;
			}
		else
			{
				dataoffline =0;
			}
                Flash_FAST_ReadData(Eepromread_tmp-225, 1022, keyboardtrans);
		
}

void ttttttttt(void)
{
	if((voice_start == 0) && ( P0_2 == 0))
{
	voice_start = 12;
	voice_reset = 1;
	P0_4 = 0x1;
	osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
}
}
extern UTCTime OSAL_timeSeconds ;
uint32 flashdownpic =0x000257C0;
		uint32 numnumnum =0;
		uint8 yyyyyyy=0,xxxxxxxxx=0;
void HidEmuKbd_Init( uint8 task_id )
{
  hidEmuKbdTaskId = task_id;
  NPI_InitTransport(simpleBLE_NpiSerial1Callback);

  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    uint8 initial_advertising_enable =1;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advData ), advData );
    if(scan_head == 1)
    	{
		GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanData ), scanData );
		Poweroffmscan = 1800;
    	}
    else
    	{
    		GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scan_mobileData ), scan_mobileData );
		Poweroffmscan = 300;
    	}

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }
    if(scan_head == 1)
    	{
		  // Set the GAP Characteristics
#ifdef EPDDOT200	
		  attDeviceNameS[6] = 'B';
#endif
		  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, (void *) attDeviceNameS );
    	}
    else
    	{
		  // Set the GAP Characteristics
#ifdef EPDDOT200	
		  attDeviceName[4] = 'B';
#endif
		  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, (void *) attDeviceName );
    	}
  
  // Setup the GAP Bond Manager
  {
    uint32 passkey = DEFAULT_PASSCODE;
    uint8 pairMode = DEFAULT_PAIRING_MODE;
    uint8 mitm = DEFAULT_MITM_MODE;
    uint8 ioCap = DEFAULT_IO_CAPABILITIES;
    uint8 bonding = DEFAULT_BONDING_MODE;
	    uint8 autoSync = TRUE;
    uint8 bondFailAction = GAPBOND_FAIL_NO_ACTION;
    uint16 BondTimeOut = 10000;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof( uint8 ), &bonding );
    VOID GAPBondMgr_SetParameter( GAPBOND_AUTO_SYNC_WL, sizeof( uint8 ), &autoSync );
    VOID GAPBondMgr_SetParameter( GAPBOND_BOND_FAIL_ACTION, sizeof( uint8 ), &bondFailAction );
    GAPBondMgr_SetParameter(GAPBOND_INITIATE_WAIT, sizeof(uint16), &BondTimeOut);
  }

  // Setup Battery Characteristic Values
  {
    uint8 critical = DEFAULT_BATT_CRITICAL_LEVEL;
    Batt_SetParameter( BATT_PARAM_CRITICAL_LEVEL, sizeof (uint8), &critical );
  }

  // Set up HID keyboard service
  HidKbd_AddService( );

  // Register for HID Dev callback
  HidDev_Register( &hidEmuKbdCfg, &hidEmuKbdHidCBs );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( hidEmuKbdTaskId );
//  HCI_EXT_SetTxPowerCmd( HCI_EXT_TX_POWER_4_DBM );

#if defined( CC2540_MINIDK )
  // makes sure LEDs are off
  HalLedSet( (HAL_LED_1 | HAL_LED_2), HAL_LED_MODE_OFF );

  // For keyfob board set GPIO pins into a power-optimized state
  // Note that there is still some leakage current from the buzzer,
  // accelerometer, LEDs, and buttons on the PCB.

  //P0SEL = 0; // Configure Port 0 as GPIO
  //P1SEL = 0; // Configure Port 1 as GPIO
 // P2SEL = 0; // Configure Port 2 as GPIO

  P0DIR = 0x58; // Port 0 pins P0.0 and P0.1 as input (buttons),
                // all others (P0.2-P0.7) as output
 // P1DIR = 0xFF; // All port 1 pins (P1.0-P1.7) as output
 // P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output

  P0 = 0x03; // All pins on port 0 to low except for P0.0 and P0.1 (buttons)
  //P1 = 0;   // All pins on port 1 to low
 // P2 = 0;   // All pins on port 2 to low

#endif // #if defined( CC2540_MINIDK )
HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_DISABLE_CLK_DIVIDE_ON_HALT );
HCI_EXT_HaltDuringRfCmd(HCI_EXT_HALT_DURING_RF_DISABLE);
  // Setup a delayed profile startup  
  osal_set_event( hidEmuKbdTaskId, START_DEVICE_EVT );
 
  osal_set_event( hidEmuKbdTaskId, START_KEYBOARD_DETECT );
  AUTOpowertime =  OSAL_timeSeconds;
  SPI_Flash_Init();

  	{
//Flash_SectorErase(0x25000);
//Flash_SectorErase(0x26000);
/*
		numnumnum =sizeof(keyboardtrans);
		for(uint32 i=0;i<numnumnum;i++)
			{
				xxxxxxxxx = ~keyboardtrans[i];
				Flash_WriteByte(flashdownpic, xxxxxxxxx);
				yyyyyyy =0x87;
				Flash_ReadData(flashdownpic, &yyyyyyy);
				if(yyyyyyy != xxxxxxxxx)
					{
						while(1);
					}
				
				flashdownpic++;
				
			}
			*/
	}

	

  if(LCD_exist == 1)
  	{
 		 EPD_init_Full();
  	}
  Init_Watchdog() ;

  FlashReadID();

//          uint32 test;
//          Flash_FAST_ReadData(0x01, 4, (uint8 *)&test);

  
  Flash_ReadData(0x20000, &sector_buffer[0]);
  Flash_ReadData(0x21000, &sector_buffer[1]);
  Flash_ReadData(0x22000, &sector_buffer[2]);
  Flash_ReadData(0x1FF000, &sector_buffer[3]);

 Flash_FAST_ReadData(0x23000, 1022,keyboardtrans);
Flash_FAST_ReadData(0x38000, 1022, keyboardtrans);

 //FLash_all_readdata();
  if(sector_buffer[0] != 0xFF)
  	{
  		FLASH_READADDRESS = 0x20000;
		FLASH_read();
  				
  	}
  else if(sector_buffer[1] != 0xFF)
	{
		FLASH_READADDRESS = 0x21000;
		FLASH_read();
	}
  else if(sector_buffer[2] != 0xFF)
	{
		FLASH_READADDRESS = 0x22000;
		FLASH_read();
	}
  else if(sector_buffer[3] != 0xFF)
	{
	      FLASH_READADDRESS = 0x1FF000;		
		FLASH_read();
	}
   else
	{
		FLASH_READADDRESS = 0x20000;
		FLASH_Initialization();
				
	}

//Picture_on[0] =0xFF;
//Picture_on[1] =0x03;
//Picture_on[2] =0x00;
}
void Serialdatawriteflash(uint8 data)
{
	uint8 tmp=0;
		
	if((1))
		{
			Flash_ReadData(Eeprom_write, &tmp);
			 if(tmp != 0xFF)
				{
					Eeprom_full=1;
				}
							
		}
	if(Eeprom_full == 0)
		{
				if(Sector_wip == 0)
					{
						if((wip_data_count !=0) &&  Eeprom_add_change == 0)
							{
								Eeprom_adda_copy = Eeprom_write;
								Eeprom_write = Eeprom_write+  wip_data_count;
								Eeprom_add_change =1;
										
								  osal_set_event( hidEmuKbdTaskId, START_Eeprom_change );
								
							}
								Flash_WriteByte(Eeprom_write, data);
								Eeprom_write ++;
								if(Eeprom_write >=0x1FF000)
								{
										Eeprom_write =0x38000;
								}
																				
								if(Eeprom_write == Eeprom_read)
								{
										Eeprom_full=1;
								}
					
					}
				else
					{
						Wip_copy[wip_data_count] =data;
						wip_data_count ++;
					}
							
		}

			
	
}
extern gaprole_States_t hidDevGapState ;
extern uint32 OSAL_timeSecondsMS ;

uint32 hhhhhhhhhh=0,jjjjjjjjj=0 ;


void payyinzi(uint8 tmp)
{
	if(tmp == 1)
		{
			if((voice_start == 0) && ( P0_2 == 0) && ( (sound_on[0] &0x40) == 0x40))
				{
					voice_start = 14;
					voice_reset = 1;
					P0_4 = 0x1;
					osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
				}
		}
	else if(tmp == 2)
		{
			if((voice_start == 0) && ( P0_2 == 0) && ( (sound_on[0] &0x80) == 0x80))
				{
					voice_start = 15;
					voice_reset = 1;
					P0_4 = 0x1;
					osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
				}
		}
	else if(tmp == 3)
		{
			if((voice_start == 0) && ( P0_2 == 0) && ( (sound_on[1] &0x01) == 0x01))
				{
					voice_start = 16;
					voice_reset = 1;
					P0_4 = 0x1;
					osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
				}		
		}
}

void IP5310_charge(void)
{
	if(hhhhhhhhhh == 0)
		{
			hhhhhhhhhh =OSAL_timeSecondsMS;
		}
	if( P0_1 == 1)
		{
			if(batcharge == 0)
				{
					if((voice_start == 0) && ( P0_2 == 0) && ( (sound_on[0] &0x20) == 0x20))
						{
							Charging_mark = 1;
							if(BLE_conected ==1)
								{
									retrue_BLE =1;
								}
							voice_start = 11;
							voice_reset = 1;
							P0_4 = 0x1;
							osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
						}						
				}
			batcharge = 1;	
			charge_datectcount = 10000;
		}
	else
		{
			if(charge_datectcount > 0)
				{
					charge_datectcount--;
					if(charge_datectcount == 0)
						{
							batcharge = 0;
							if(BLE_conected ==1)
								{
									retrue_BLE =1;
								}
							Charging_mark = 0;
							jjjjjjjjj = OSAL_timeSecondsMS;
						}
				}
		}
}
volatile uint32 flash_picutradress =0;
//uint8 double_click=0;
void Switch_picutres(void)
{
     // uint8 param;      
	uint32  displaypicture_tmp = 0;
	if((P0_5 == 0) && (Left_key == 0))
		{

			if(Leftcount_key < 700)
				{
					Leftcount_key++;
				}
			else
				{
		//			double_click++;
					Left_key = 1;
					Leftcount_key = 0;
				}
		}
	else
		{
			if(Left_key == 0)
				{
					Leftcount_key = 0;
					Left_key = 0;
				}

		}

	if((P0_7 == 0) && (Right_key == 0))
		{

			if(Rightcount_key < 700)
				{
					Rightcount_key++;
				}
			else
				{
	//			       double_click++;
					Right_key = 1;
					Rightcount_key = 0;
				}
		}
	else
		{

			if(Right_key == 0)
				{
					Rightcount_key = 0;
					Right_key = 0;
				}
		}

	if( (Right_key  == 1) && (LCD_chanel == 0))
		{
			displaypicture_on =0;
                   displaypicture_on = displaypicture_on | ((Picture_on[0] << 0) &  0x000000FF);
                   displaypicture_on = displaypicture_on | ((Picture_on[1] << 8) &  0x0000FF00);
			displaypicture_tmp = 0;
			displaypicture_tmp =  Picture_on[2] ;
			displaypicture_on = displaypicture_on | displaypicture_tmp<< 16;
			

			displaypicture_tmp = display_picture << 1;

			if(displaypicture_on != 0)
				{
					if(display_picture == displaypicture_on)
					{
						LCD_chanel = 1;
							njnjnjn = 1;
							while(1)
								{
									if((displaypicture_tmp & 0x1) == 1)
										{
											break;
										}
									else
										{
											displaypicture_tmp = displaypicture_tmp >> 1;
											njnjnjn++;
										}
								}
							flash_picutradress = njnjnjn-1;
							flash_picutradress = flash_picutradress *0x2000;
							flash_picutradress = 0x23000 + flash_picutradress;
							njnjnjn = njnjnjn *7;
						
					}	
					else if(displaypicture_on >= displaypicture_tmp )
						{
							for(uint16 i=0;i<20;i++)
								{
									if( (displaypicture_on & displaypicture_tmp) == displaypicture_tmp)
										{
											display_picture = displaypicture_tmp;
											leftright = 1;
											LCD_chanel = 1;
												njnjnjn = 1;
												while(1)
													{
														if((displaypicture_tmp & 0x1) == 1)
															{
																break;
															}
														else
															{
																displaypicture_tmp = displaypicture_tmp >> 1;
																njnjnjn++;
															}
													}
												flash_picutradress = njnjnjn-1;
												flash_picutradress = flash_picutradress *0x2000;
												flash_picutradress = 0x23000 + flash_picutradress;
												njnjnjn = njnjnjn *7;
												break;
										}
									else
										{
											displaypicture_tmp = displaypicture_tmp << 1;
										}
								}
						}
					else
						{
							displaypicture_tmp = 1;
							for(uint16 i=0;i<20;i++)
								{
									if( (displaypicture_on & displaypicture_tmp) == displaypicture_tmp)
										{
											display_picture = displaypicture_tmp;
											leftright = 1;
											LCD_chanel = 1;
												njnjnjn = 1;
												while(1)
													{
														if((displaypicture_tmp & 0x1) == 1)
															{
																break;
															}
														else
															{
																displaypicture_tmp = displaypicture_tmp >> 1;
																njnjnjn++;
															}
													}
												flash_picutradress = njnjnjn-1;
												flash_picutradress = flash_picutradress *0x2000;
												flash_picutradress = 0x23000 + flash_picutradress;
												njnjnjn = njnjnjn *7;
												break;
										}
									else
										{
											displaypicture_tmp = displaypicture_tmp << 1;
										}
								}							
						}
					
				}
				
	
		}
	if((Left_key == 1) && (LCD_chanel == 0))
		{
			displaypicture_on =0;
                   displaypicture_on = displaypicture_on | ((Picture_on[0] << 0) &  0x000000FF);
                   displaypicture_on = displaypicture_on | ((Picture_on[1] << 8) &  0x0000FF00);
			displaypicture_tmp = 0;
			displaypicture_tmp =  Picture_on[2] ;
			displaypicture_on = displaypicture_on | displaypicture_tmp<< 16;

			if(displaypicture_on != 0)
				{
					if(display_picture == displaypicture_on)
					{
											LCD_chanel = 1;
												njnjnjn = 1;
												while(1)
													{
														if((displaypicture_tmp & 0x1) == 1)
															{
																break;
															}
														else
															{
																displaypicture_tmp = displaypicture_tmp >> 1;
																njnjnjn++;
															}
													}
												flash_picutradress = njnjnjn-1;
												flash_picutradress = flash_picutradress *0x2000;
												flash_picutradress = 0x23000 + flash_picutradress;
												njnjnjn = njnjnjn *7;
					}	
					else 
					{
						if(display_picture == 1)
							{
								displaypicture_tmp =  0xF0000000;
							}
						else
							{
								displaypicture_tmp = display_picture - 1;
								if((displaypicture_on & displaypicture_tmp) == 0)
									{
										displaypicture_tmp =  0xF0000000;
									}
								else
									{
										displaypicture_tmp =  0x0F000000;
									}
								
							}

						if(displaypicture_tmp == 0xF0000000)
							{
								displaypicture_tmp = 0x80000;
								for(uint16 i=0;i<20;i++)
									{
										if( (displaypicture_on & displaypicture_tmp) == displaypicture_tmp)
											{
												display_picture = displaypicture_tmp;
												leftright = 2 ;
												LCD_chanel = 1;
												njnjnjn = 1;
												while(1)
													{
														if((displaypicture_tmp & 0x1) == 1)
															{
																break;
															}
														else
															{
																displaypicture_tmp = displaypicture_tmp >> 1;
																njnjnjn++;
															}
													}
												flash_picutradress = njnjnjn-1;
												flash_picutradress = flash_picutradress *0x2000;
												flash_picutradress = 0x23000 + flash_picutradress;
												njnjnjn = njnjnjn *7;
												break;
											}
										else
											{
												displaypicture_tmp = displaypicture_tmp >> 1;
											}
									}								
							}
						else
							{
								displaypicture_tmp = display_picture >> 1;
								
								for(uint16 i=0;i<20;i++)
									{
										if( (displaypicture_on & displaypicture_tmp) == displaypicture_tmp)
											{
												display_picture = displaypicture_tmp;
												leftright = 2 ;
												LCD_chanel = 1;
												njnjnjn = 1;
												while(1)
													{
														if((displaypicture_tmp & 0x1) == 1)
															{
																break;
															}
														else
															{
																displaypicture_tmp = displaypicture_tmp >> 1;
																njnjnjn++;
															}
													}
												flash_picutradress = njnjnjn-1;
												flash_picutradress = flash_picutradress *0x2000;
												flash_picutradress = 0x23000 + flash_picutradress;
												njnjnjn = njnjnjn *7;
												break;
											}
										else
											{
												displaypicture_tmp = displaypicture_tmp >> 1;
											}
									}									
							}
						
					}

				}
							
		}
	  	  if((LCD_exist == 1) && ((Left_key == 1) || (Right_key == 1))&&(LCD_chanel != 0))
	  	  	{
	  	  		if(LCD_chanel == 1)
	  	  			{
	  	  			 Epaper_First_line(njnjnjn,flash_picutradress);
					 LCD_chanel = 2;
	  	  			}
#ifdef EPDDOT152	
				else if(LCD_chanel ==152)			
#endif
#ifdef EPDDOT200	
				else if(LCD_chanel ==200)			
#endif
					{
						Epaper_line(njnjnjn);

						Epaper_Update();
						
						Left_key = 2;
						Right_key = 2;
						
					}
				else if(LCD_chanel > 1)
					{
						Epaper_line(njnjnjn);
						LCD_chanel ++;
					}
	  	  	}

		  if(Left_key == 2 && (P0_5 == 1))
		  	{
		  		//if(Leftcount_key <700)
		  		//	{
		  		//		Leftcount_key++;
		  		//	}
				//else
					{
						Left_key =0;
						Leftcount_key =0;
						LCD_chanel =0;
					}
		  	}
		  if(Right_key == 2 && (P0_7 == 1))
		  	{
		  		//if(Rightcount_key <700)
		  		//	{
		  		//		Rightcount_key++;
		  		//	}
				//else
					{
						Right_key =0;
						Rightcount_key =0;
						LCD_chanel =0;
					}
		  	}

                  if((LCD_exist == 0) && ((Left_key == 1) || (Right_key == 1))){
                          Left_key = 2;
                          Right_key = 2;
                          P0_6= 0;
                          P06time = 1;
                          scan_flag = 0;                   
//        shift_key = 0x12;
//        hidEmuKbdSendReport( 0x2C );
//        hidEmuKbdSendReport( 0x00 );
//        shift_key = 0x00;
                       //   param = TRUE;
                       //   if((BLE_conected == 0) || (USB_conected== 0))
                       //   GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &param );
                  }
}
/*********************************************************************
 * @fn      HidEmuKbd_ProcessEvent
 *
 * @brief   HidEmuKbd Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
extern uint8 jjjjj;

uint16 HidEmuKbd_ProcessEvent(uint8 task_id, uint16 events)
{

  VOID task_id; // OSAL required parameter that isn't used in this function
  uint8 i=0;//buffer[8];
  uint8 param;
  osalTimerRec_t *keyboardtimer=NULL;

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( hidEmuKbdTaskId )) != NULL )
    {
      hidEmuKbd_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & START_DEVICE_EVT )
  {
    return ( events ^ START_DEVICE_EVT );
  }

  if ( events & START_Gled_onoff)
  {
	if(P1_1 == 1)
	{
		osal_start_timerEx( hidEmuKbdTaskId, START_Gled_onoff,10 );
		keycountrepeat = 0;
	}
	else
	{
		keycountrepeat ++;
		if(keycountrepeat < 5)
			{
				osal_start_timerEx( hidEmuKbdTaskId, START_Gled_onoff,10 );
			}
		else
			{
				keycountrepeat = 0;
				keyrepeat = 0;
				
			}
	}
    return ( events ^ START_Gled_onoff );
  }
  
  if ( events & START_KEYBOARD_DETECT)
  {
    if( (P1_1 == 1) || (Powerkey==1))                                                     //OK
	{
		if((P06time ==0) && ((Powerkey==1)))
		{
			 P0_6= 0;
			 P06time =1; 
                       //  scan_flag = 0;
		}
		if((P1_1 == 1) && (Powerkey==0))
		{
			if(powerofftime<20)
				{
					powerofftime ++;
				}
			else
				{
					if(powerofftime < 100)
						{
							
							if((voice_start == 0) && ( P0_2 == 0))
							{
								powerofftime =100;
								voice_start = 9;
								voice_reset = 1;
								P0_4 = 0x1;
								osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
							}									
						}
				}
	}

	AUTOpowertime =  OSAL_timeSeconds;
	}
	else
	{
		if(powerofftime<99)
			{
				powerofftime =0;
			}
			
	}


	if(P06time >0)                                                           //OK
		{
			if(P06time < 20)
				{
					P06time ++;
				}
			else
				{
					P06time =0;
					P0_6= 1;
					Powerkey =0;
					if((scan_flag == 0)&&(BLEtime_out == 10)){
						if(BLE_conected == 1){
							GAPRole_TerminateConnection();
						}
						else
						{
							uint8 param = TRUE;
							GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &param );
						} 
					}
				}
			
		}

  	if(batadccount < 50)                                                //OK
  		{
  			batadccount ++;
  		}
	else
		{
			if(Bat_level == 4200)
				{
					Getbatlevel ();
					Batlevel_tmp = Bat_level;
					Bat_level = 2100;
				      batadccount=0;						
				}
			else
				{
					uint32 Getbatdata =0;
					Getbatdata=Getbatlevel ();
					Getbatdata = Getbatdata*3300/2048;
					if(Getbatdata > 2025 )                   //4050MV
						{
							charge_quantity = 100 ; 
						}
					else if(Getbatdata > 1950 )                //3900MV
						{
							charge_quantity = 75 ; 
						}
					else if(Getbatdata > 1875 )                //3750MV
						{
							charge_quantity = 50 ; 
						}
					else if(Getbatdata > 1800 )                 //3600MV
						{
							charge_quantity = 25 ; 
						}
					else                                         //3400MV
						{
							charge_quantity = 10 ; 
						}
					Batlevel_tmp = Bat_level;
					Bat_level = Getbatdata;
				      batadccount=0;	
				}

		}
		/*

		if((Bat_level - 50) > Batlevel_tmp )  
		{
			if(batcharge == 1)
				{
					if((voice_start == 0) && ( P0_2 == 0) && ( (sound_on[0] &0x20) == 0x20))
						{
							voice_start = 11;
							voice_reset = 1;
							P0_4 = 0x1;
							osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
						}						
				}
			batcharge = 1;
		}
		*/


	if(Bat_level < 1765 )  
		{
			if(batlowvoice == 0)
				{
					if((voice_start == 0) && ( P0_2 == 0) && ( (sound_on[0] &0x10) == 0x10))
						{
							voice_start = 10;
							voice_reset = 1;
							P0_4 = 0x1;
							osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
						}						
				}
			batlowvoice = 1;
		}
	if(Bat_level < 1691 )                                             //OK
		{

			if(batlow_count < 150)
				{
					batlow_count++;
				}
			else
				{
					batlow_powerdown =1;
				}
			
		}
	else
		{
			batlow_count =0;
		}

/*
	if(  P0_4  ==1  )
  		{
  			if(USB_datect < 20)
  				{
  					USB_datect ++;
  				}
			else
				{
					USB_conected =1;
				}
  			
  		}
	   else
	   	{
	   		USB_datect =0;
			USB_conected =0;
	   	}

*/
 	    if(Bat_level > 1759)                                                           //OK  
	    	{

		   if(BLE_conected == 1)
		   	{
		   		GLed_onoff (1);
		   	}
		   else
		   	{
		    		if(ledGcount < 5)
		    			{
	                                ledGcount++;
		    				GLed_onoff (0);
		    			}
				else if(ledGcount < 10)
					{
	                                ledGcount++;
						GLed_onoff (1);
					}
				else
					{
						ledGcount =0;
					}
		   	}

	    	}
		else
			{

		    		if(ledGcount < 20)
		    			{
	                                ledGcount++;
		    				GLed_onoff (0);
		    			}
				else if(ledGcount < 40)
					{
	                                ledGcount++;
						GLed_onoff (1);
					}
				else
					{
						ledGcount =0;
					}			

			}

		

	     if(  USBADDR !=0  )                                                                   //OK
  		{
  			if(USB_datect < 40)
  				{
  					USB_datect ++;
  				}
			else
				{
					
					if( USB_conected ==0)
				   	{
						if((voice_start == 0) && ( P0_2 == 0))
							{
								voice_start = 1;
								voice_reset = 1;
								P0_4 = 0x1;
								osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
							}
						
				   	}
					USB_conected =1;
				}
  			
  		}
	   else
	   	{
	   		USB_datect =0;
			USB_conected =0;
	   	}


	     if(  hidDevGapState ==GAPROLE_CONNECTED  )                          //OK
  		{
  			if(BLE_datect < BLEtime_out)//100 //BLEtime_out=100
  				{
  					BLE_datect ++;
  				}
			else
				{
					 if(BLE_conected ==0)
					   	{
							if((voice_start == 0) && ( P0_2 == 0) && ( (sound_on[0] &0x04) == 0x04))
								{
									voice_start = 6;
									voice_reset = 1;
									P0_4 = 0x1;
									osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
								}
							if(Bleadapter == 0)
								{
									Ble_ConnInterval=40;
								}
							
					   	}
					BLE_conected =1;

				}
  			
  		}
	   else
	   	{
	   		if(BLE_conected ==1)
	   			{
					if((voice_start == 0) && ( P0_2 == 0) && ( (sound_on[0] &0x08) == 0x08))
						{
							voice_start = 7;
							voice_reset = 1;
							P0_4 = 0x1;
							osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
						}	   				
	   			}
	   		BLE_datect =0;
			BLE_conected =0;
			Ble_ConnInterval=10;
	   	}


		if(eepromear_ok ==1)                                               //OK
			{
				if(Sector_wip == 0)
					{
						Flash_Sector_Erase(Eeprom_ear);	
						
					}
				
				Sector_wip = Flash_ReadWip();
				if(Sector_wip == 0)
					{
						eepromear_ok =0;
						Eeprom_full =0;	
						
					}

			}
		if((Sector_wip == 0) && (wip_data_count ==0))
			{
				data_suspend=0;	
			}
		else
			{
				data_suspend=1;
				 
			}

		if(earpicture_flash == 1)  
			{
				if(Flash_ReadWip() == 0)
					{
						earpicture_flash = 2;
						Flash_Sector_Erase(Flashpicture_address +0x1000);	
					}
			}

		if(earpicture_flash == 2)
			{
				if(Flash_ReadWip() == 0)
					{
						uint8 tmp_value[10]={0xE0,0,0,0,0,0,0,0,0,0}; 
						earpicture_flash = 0;
						SimpleProfile_SetParameter( 3, 10, tmp_value );
                                                jjjjj = 0xdf;
					}
			}

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    static uint8 Sector_count=0;
    static uint8 count = 1;
    if(firmware_flash_task == start_Empty_flash)
    {
      if(Flash_ReadWip() == 0)
      {
       // Flash_WriteByte(firmware_addr, 0x01);  //升级标志
        Flash_WriteByte(firmware_addr+1, (uint8)firmware_size&0xFF);   //固件size
        Flash_WriteByte(firmware_addr+2, (uint8)(firmware_size>>8)&0xFF);
        Flash_WriteByte(firmware_addr+3, (uint8)(firmware_size>>16)&0xFF);
        Flash_WriteByte(firmware_addr+4, (uint8)(firmware_size>>24)&0xFF);
        
        Sector_count = (firmware_size)/4096+((firmware_size)%4096?1:0);
        if(Sector_count)
          Flash_Sector_Erase(firmware_addr+4096);
        
        firmware_flash_task = next_Empty_flash;
      }
    }else
    if(firmware_flash_task == next_Empty_flash)
    {
      if(Flash_ReadWip() == 0)
      {
        if(Sector_count > 1)
        {
          Flash_Sector_Erase(firmware_addr+(uint32)Sector_count*0x1000);
          Sector_count--;
        }else
        {
          uint8 tmp_value[10]={0xEE,0,0,0,0,0,0,0,0,0}; 
          firmware_addr += 0x1000;
          firmware_flash_task = firmware_task_close;
          SimpleProfile_SetParameter( 3, 10, tmp_value );
          count = 1;
        }
      }
    }else
      if(firmware_flash_task == flash_receipt)
      {
          uint8 tmp_value[10]={0xEF,0,count,0,0,0,0,0,0,0};         
          count++;
          firmware_flash_task = firmware_task_close;
          SimpleProfile_SetParameter( 3, 10, tmp_value );
      }
    static uint16 crc[2];
    static uint32 addr; 
    if(firmware_crc == 1)
    {
      count = 0;
      firmware_crc = 2;
     
      Flash_FAST_ReadData(firmware_base+0x1090, 4, (uint8 *)&crc);
      Flash_FAST_ReadData(firmware_base+1, 4, (uint8 *)(&firmware_size));
      crc[1] = 0;
      uint8 buf[144];
      Flash_FAST_ReadData((firmware_base+0x1000), 0x90, (uint8 *)&buf);
      for (uint8 idx = 0; idx < 0x90; idx++)
      {
        crc[1] = crc16(crc[1], buf[idx]);
      }
      addr = firmware_base+0x1094;
    }
    else
      if(firmware_crc == 2)
      {
        uint8 end;
        if(((uint32)0x3F000+firmware_base-addr)<0x2710)
            end = 38;
        else
            end = 100;
        for(uint8 idy = 0; idy < end; idy++)
        {
          uint8 buf[100];
          Flash_FAST_ReadData(addr, 100, (uint8 *)&buf);
          for (uint8 idx = 0; idx < 100; idx++)
          {
            crc[1] = crc16(crc[1], buf[idx]);
          }
          addr += 100;
        }
        if(end == 38)
        {
          uint8 buf[4];
          Flash_FAST_ReadData(addr, 4, (uint8 *)&buf);
          for (uint8 idx = 0; idx < 4; idx++)
          {
            crc[1] = crc16(crc[1], buf[idx]);
          }
          crc[1] = crc16(crc[1], 0);
          crc[1] = crc16(crc[1], 0);          
          
          if(crc[1] == crc[0])
          {
              firmware_crc = 3;
              uint8 tmp_value[10]={0x11,0,0,0,0,0,0,0,0,0}; 
              SimpleProfile_SetParameter( 3, 10, tmp_value );
          }else 
          {
            firmware_crc = 0;
          }
        }
      }else if(firmware_crc == 3){
        firmware_crc = 4;
      }else if(firmware_crc == 4){
        firmware_crc = 0;
        HAL_DISABLE_INTERRUPTS();
        HAL_SYSTEM_RESET();
      }
    //////////////////////////////////////////////////////////////////////////////////////////////////
		if(retrue_BLE == 1)
			{
				uint8 tmp_value[10]={0,0,0,0,0,0,0,0,0,0};  
				retrue_BLE = 0;

				tmp_value[0] = Picture_on[0];
				tmp_value[1]= Picture_on[1];
				tmp_value[2]= Picture_on[2];

				tmp_value[3]= sound_on[0];
				tmp_value[4]= sound_on[1];
				
				if((Poweroffmscan == 300) || (Poweroffmscan == 1800))
					{
						tmp_value[6] = tmp_value[6] |0x1;
					}

				if(voice_on == 1)
					{
						tmp_value[6] = tmp_value[6] |0x2;
					}

				if(camera_gbk == 1)
					{
						tmp_value[6] = tmp_value[6] |0x4;
					}
				if(camera_ok == 1)
					{
						tmp_value[6] = tmp_value[6] |0x8;
					}
				if(Earflash == 1)
					{
						tmp_value[6] = tmp_value[6] |0x10;
					}
				
				tmp_value[7]= Charging_mark;
				tmp_value[8]= charge_quantity;

#ifdef EPDDOT200	
		  		tmp_value[9]= 1;
#endif
				SimpleProfile_SetParameter( 3, 10, tmp_value );

				
			}

		
	if(Eeprom_full == 1)                                                  //OK
		{
					if((voice_start == 0) && ( P0_2 == 0))
						{
							voice_start = 1;
							voice_reset = 1;
							P0_4 = 0x1;
							osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,10000); 
						}						
		}

		
	if(Earflash == 1)                                                                                              //OK   提示澹?
		{

			Flash_earaALL();			
		}

	
		if(buzzer_continus ==2)                                                              //OK
			{
				
				if(Eepromread_tmp ==  Eeprom_write)
					{
						buzzer_continus =0;
					}
					if((voice_start == 0) && ( P0_2 == 0))
						{
							voice_start = 1;
							voice_reset = 1;
							P0_4 = 0x1;
							osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
						}	
			}


	   	if(((BLE_conected == 1) || (USB_conected== 1)) && (Eeprom_read != Eeprom_write) && (data_suspend ==0) && (dataoffline ==0))             //OK
	   		{
				for(i=0;i<30;i++)
					{
						if(keyboard_read_tmp != keyboard_read)
							{
								if(Eeprom_read != Eeprom_write)
									{

										Flash_ReadData(Eeprom_read, &keyboardtrans[keyboard_write]);
										
										keyboard_write++;
										if(keyboard_write >=1024)
										{
											keyboard_write =0;
										}

										Eeprom_read++;
										if(Eeprom_read >=0x1FF000)
										{
											Eeprom_read =0x38000;
										}
										
									     if(buzzer_continus ==1)
									     	{
									     		buzzer_continus=2;
									     	}

										if(keyboard_write == keyboard_read )
											{
												break;
											}
										
									}
								else
									{
										break;
									}
								
							}
						  else
							{
								break;
							}

						
							
					}

				keyboard_read_tmp  =  keyboard_read;
				AUTOpowertime =  OSAL_timeSeconds;
			

                        
			 keyboardtimer = osalFindTimer( hidEmuKbdTaskId , START_keyboard_transmission );
			 if((keyboardtimer == NULL) )
			 	{
					osal_start_timerEx( hidEmuKbdTaskId, START_keyboard_transmission,100);

			 	}
                        
				
	   	}


	if(((OSAL_timeSeconds -AUTOpowertime) >Poweroffmscan) || (batlow_powerdown ==  1 ))           //OK
		{
			if(Poweroffmscan !=9000)
			{
				if(powerofftime < 100)
					{
						
						if((voice_start == 0) && ( P0_2 == 0))
							{
								powerofftime =100;
								voice_start = 9;      //
								voice_reset = 1;
								P0_4 = 0x1;
								osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
							}									
					}
				}
		}

	
		if((powerofftime ==101)&& ( P0_2 == 0))
			{
				//if(sector_chang == 1)
					{
						FLASH_write();
						sector_chang = 0;
					}
				P1_0 = 0x0;
				P0_6= 1;
				while(1)
					{
						FeetDog();
					}
			}


	   if((poweroffbond == 1) || (poweroffbond == 2))
	   	{
	   		if((BLE_conected ==1) && (poweroffbond == 1))
	   			{
	   				HidDev_Close();
					poweroffbond = 2;
	   			}
			if(BLE_conected ==0)
				{
					VOID GAPBondMgr_SetParameter( GAPBOND_ERASE_ALLBONDS, 0, NULL );
					poweroffbond = 0;
					powerofftime =100;
					 if(Buzzer_on ==0)
						{
							Buzzer_on=1;
						}	
				}
	   		
	   	}

    osal_start_timerEx( hidEmuKbdTaskId, START_KEYBOARD_DETECT,100 );
    FeetDog();

    return ( events ^ START_KEYBOARD_DETECT );
  }
   
  if ( events & START_keyboard_transmission )
  {

			if(((BLE_conected == 1) || (USB_conected== 1))&& (data_suspend ==0))
			{
					if((chinese_value ==0) && (camera_ok == 1))
					{
							if(((keyboardtrans[keyboard_read] >=0x81) && (Bleadapter == 1))  || (g_bIsEng ==1) || ((keyboardtrans[keyboard_read] >=0x81) && ( USB_conected ==1)) )
								{
											if(( g_bIsEng ==0) && (keyboardtrans[keyboard_read] >=0x81) )
											  	{
											  		g_bIsEng =1;
													HIDkeyboardvalue =0;
													HIDkeyboardvalue =  HIDkeyboardvalue |(keyboardtrans[keyboard_read] <<8);
											  	}
											  else if(g_bIsEng ==1)
											  	{
											  		g_bIsEng =0;
													HIDkeyboardvalue =  HIDkeyboardvalue |(keyboardtrans[keyboard_read] <<0);

													if(camera_gbk == 1)
														{
															HIDkeyboardvalue_tmp =  HIDkeyboardvalue;
															unicode_address =HIDkeyboardvalue_tmp -0x8140;
															unicode_address =unicode_address*4;
															Flash_FAST_ReadData(unicode_address, 4, unicode_tmp);
															HIDkeyboardvalue_tmp=0;
															HIDkeyboardvalue_tmp =  HIDkeyboardvalue_tmp |(unicode_tmp[0] <<0);
															HIDkeyboardvalue_tmp =  HIDkeyboardvalue_tmp |(unicode_tmp[1] <<8);
															if(HIDkeyboardvalue == HIDkeyboardvalue_tmp)
																{
																	HIDkeyboardvalue=0;
																	HIDkeyboardvalue =  HIDkeyboardvalue |(unicode_tmp[2] <<0);
																	HIDkeyboardvalue =  HIDkeyboardvalue |(unicode_tmp[3] <<8);
																}
															
														}


												     for(i=0;i<5;i++)
												     	{

														key_chinese_value[4-i]  =0;

												     		key_chinese_value[4-i]  = datavlaue[HIDkeyboardvalue  % 10];
														HIDkeyboardvalue = HIDkeyboardvalue / 10;
														 
												     	}
												  for(i=0;i<12;i++)
												     	{

														keyvlauetmp[i]  =0;
														 
												     	}
												  	keyvlauetmp[1]=key_chinese_value[0];
													keyvlauetmp[3]=key_chinese_value[1];
													keyvlauetmp[5]=key_chinese_value[2];
													keyvlauetmp[7]=key_chinese_value[3];
													keyvlauetmp[9]=key_chinese_value[4];
													 chinese_value =1;	
											  	}				
									
								}
					}
					if((chinese_value ==1) && (camera_ok == 1) )
					{
							
							if(keyvlauetmp_count <11)
								{
									if( (BLE_conected  ==  1) && (Bleadapter == 1))
										{
											hidEmuKbdSendReport_chinese( keyvlauetmp[keyvlauetmp_count] );
										}
									if( USB_conected ==1)
										{
											hidUSBKbdSendReport_chinese( keyvlauetmp[keyvlauetmp_count] );

										}
									keyvlauetmp_count ++;

									osal_start_timerEx( hidEmuKbdTaskId, START_keyboard_transmission,Ble_ConnInterval);  //10 OK  9OK   8 no ok
									//osal_set_event( hidEmuKbdTaskId, START_keyboard_transmission );
									return ( events ^ START_keyboard_transmission );
								}
							else
								{
									if( (BLE_conected  ==  1) && (Bleadapter == 1))
										{
											hidEmuKbdSendReport( keyvlauetmp[keyvlauetmp_count] );
										}
									if( USB_conected ==1)
										{
											hidUSBKbdSendReport( keyvlauetmp[keyvlauetmp_count] );

										}
									keyvlauetmp_count =0;
			
									chinese_value =0;
								}
							
							
						}
					else if(g_bIsEng ==1)
					{					
					}
					else
					{
						if(keyboardtrans[keyboard_read] < 128)
							{
								shift_key =0;
								Eng_send = ASCIIvalue[keyboardtrans[keyboard_read]];
								if(Eng_send > 127)
									{
										Eng_send = Eng_send -127;
										//shift_key = 0x20;
//                                                                                if(USB_conected ==1)
										  shift_key = 0x02;
//                                                                                if(!caps && (BLE_conected  ==  1) ){
//                                                                                  caps = 1;
//                                                                                  hidEmuKbdSendReport( 0x39 );
//                                                                                  hidEmuKbdSendReport( 0x00 );
//                                                                                }
									}
//                                                                else
//                                                                        {
//                                                                          if(caps)
//                                                                          {
//                                                                               caps = 0;
//                                                                               hidEmuKbdSendReport( 0x39 );
//                                                                               hidEmuKbdSendReport( 0x00 );
//                                                                          }
//                                                                        }
								if(chongfukey != keyboardtrans[keyboard_read] )
									{
										if( (BLE_conected  ==  1) )
											{
												hidEmuKbdSendReport( Eng_send );
												
											}
										if( USB_conected ==1)
											{
												hidUSBKbdSendReport( Eng_send );

												
											}	
										chongfukey = keyboardtrans[keyboard_read]; 
									}
								else
                                                                {
                                                                                shift_key =0;
										Eng_send=0;
                                                                          
										if( (BLE_conected  ==  1) )
											{
												hidEmuKbdSendReport( Eng_send );
												
											}
										if( USB_conected ==1)
											{
												hidUSBKbdSendReport( Eng_send );
												
											}	
										chongfukey =128;
                                                                                
										 osal_start_timerEx( hidEmuKbdTaskId, START_keyboard_transmission,Ble_ConnInterval);
										 return ( events ^ START_keyboard_transmission );
										}

								shift_key =0;
							}
						else
							{
								//while(1);
							}

					}

					keyboardread_copy=keyboard_read;
					keyboard_read++;
					Eepromread_tmp++;
					sector_chang =1;
					if(Eepromread_tmp >=0x1FF000)
						{
							Eepromread_tmp =0x38000;
							Eeprom_ear = 0x1FE000;
							eepromear_ok =1;
						}
					else
						{
							if((Eepromread_tmp & 0xFFF)==0)
								{
									Eeprom_ear = Eepromread_tmp-0x1000;
									eepromear_ok =1;
								}
						}
					if(keyboard_read >=1024)
						{
							keyboard_read =0;
						}

					if(keyboard_write !=  keyboard_read) 
					{	
									if(keyboardtrans[keyboardread_copy] == 0x00)
										{
											osal_start_timerEx( hidEmuKbdTaskId, START_keyboard_transmission,300); 
										}
									else
										{
											osal_start_timerEx( hidEmuKbdTaskId, START_keyboard_transmission,Ble_ConnInterval); 
										}		                                                                                            
					}
					else
					{
					  data_suspend =0;
					  keyboard_read_tmp =2000;
					}

					//if(keyboard_write ==  keyboard_read){
					//	GAPRole_TerminateConnection();
					//}
				}
			else
			{
				if(((BLE_conected == 1) || (USB_conected== 1))&& (data_suspend ==1)&&(chinese_value ==0))
				{
							if( (BLE_conected  ==  1) )
								{
									hidEmuKbdSendReport( 0 );	
								}
							if( USB_conected ==1)
								{
									hidUSBKbdSendReport( 0 );	
								}	
							osal_start_timerEx( hidEmuKbdTaskId, START_keyboard_transmission,Ble_ConnInterval); 
					
				}
				else
				{
					if(Eeprom_read != Eepromread_tmp)
						{						
							Eeprom_read  = Eepromread_tmp;
						}
					keyboard_write =  keyboard_read;
					keyboard_read_tmp =2000;

					if(buzzer_continus > 0)
						{
							buzzer_continus =1;
						}
				}
			}
  
    return ( events ^ START_keyboard_transmission );
  }

  
   if ( events & START_UART_DATAEND)
	{	
		Serialdatawriteflash(00);
		return ( events ^ START_UART_DATAEND );
	}

	  
   if ( events & START_Eeprom_change)
  {

   if(wip_data_tmp < wip_data_count)
	{
		Flash_WriteByte(Eeprom_adda_copy, Wip_copy[wip_data_tmp]);
		wip_data_tmp ++;
		Eeprom_adda_copy++;
		//osal_set_event( hidEmuKbdTaskId,  );
		 osal_start_timerEx( hidEmuKbdTaskId, START_Eeprom_change,2); 
		
	}
   else
	{
		wip_data_count =0;
		wip_data_tmp =0;
		Eeprom_adda_copy=0;
		Eeprom_add_change =0;
		
	}
	
	return ( events ^ START_Eeprom_change );
  }

	   
   if ( events & START_voice_change)
  {
  
	P0_4 = 0x0;

	for(uint8 i=0;i<voice_start;i++)
		{
			P0_3 = 0x0;
			driver_delay_xms1(10);	     //62US
			P0_3 = 0x1;
			driver_delay_xms1(10);
		}
	P0_3 = 0x0;
	voice_start =0;
	if(powerofftime == 100)
		{
			powerofftime =101;
		}
		
	    return ( events ^ START_voice_change );
	  }
	   
   return 0;
}

/*********************************************************************
 * @fn      hidEmuKbd_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void hidEmuKbd_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
    case KEY_CHANGE:
      hidEmuKbd_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
      break;

    case GATT_MSG_EVENT:
      hidEmuKbd_ProcessGattMsg( (gattMsgEvent_t *)pMsg );
      break;
      
    default:
      break;
  }
}



//extern uint32 OSAL_timeSecondsMS ;
uint32 copy_num=0;
  uint8 zz[40]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint8 zz_copy[40]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// 串口回调函数
static void simpleBLE_NpiSerial1Callback( uint8 port, uint8 events )
{
	uint8 i=0,j=0;
	
	(void)port;
    
    if (events & (HAL_UART_RX_TIMEOUT | HAL_UART_RX_FULL))   //串口有数据               //OK
    {
        uint8 numBytes = 0;

        numBytes = NPI_RxBufLen();           //读出串口缓冲区有多少字节
        
        if(numBytes > 0)
        {
                //NPI_ReadTransport1(zz+copy_num,numBytes); 
                NPI_ReadTransport(zz,numBytes); 

		   copy_num = copy_num+ numBytes;
		   if((BLE_conected == 0) && (USB_conected== 0))
		   	{
		   		buzzer_continus =1;
		   	}
		   if(zz[i] !=0)
		   	{
		   		sector_chang =1;
		   	}

		for(i=0;i<numBytes;i++)
			{

				 if( zz[i] ==  cameravalue[cameravalue_ok])
				 	{
				 		zz_copy[cameravalue_ok] =zz[i];
				 		cameravalue_ok++;
						if(cameravalue_ok >= 14)
							{
								camera_ok =1;
                                                   cameravalue_ok =0;
							}
						
				 	}
				 else if( (zz[i] ==  cameragbkvalue) && (cameravalue_ok == 13))
				 	{
				 		zz_copy[cameravalue_ok] =zz[i];
				 		cameravalue_ok++;
						if(cameravalue_ok >= 14)
							{
								camera_gbk =0;
                                                   cameravalue_ok =0;
							}
						
				 	}
				 else if( (zz[i] ==  camerauncodevalue) && (cameravalue_ok == 13))
				 	{
				 		zz_copy[cameravalue_ok] =zz[i];
				 		cameravalue_ok++;
						if(cameravalue_ok >= 14)
							{
								camera_gbk =1;
                                                   cameravalue_ok =0;
							}
						
				 	}
				 else if( (zz[i] ==  cameraearflash) && (cameravalue_ok == 13))
				 	{
				 		zz_copy[cameravalue_ok] =zz[i];
				 		cameravalue_ok++;
						if(cameravalue_ok >= 14)
							{
								Earflash =1;
                                                   cameravalue_ok =0;
							}
						
				 	}
				 else if( (zz[i] ==  cameravoice) && (cameravalue_ok == 13))
				 	{
				 		zz_copy[cameravalue_ok] =zz[i];
				 		cameravalue_ok++;
						if(cameravalue_ok >= 14)
							{
								if(voice_on == 1)
									{
										voice_on =0;
									}
								else
									{
										voice_on =1;
									}
								
                                                   cameravalue_ok =0;
							}
						
				 	}
				else if( (zz[i] ==  Poweroff_1scan) && (cameravalue_ok == 13))
				 	{
				 		zz_copy[cameravalue_ok] =zz[i];
				 		cameravalue_ok++;
						if(cameravalue_ok >= 14)
							{
								    if(scan_head == 1)
								    	{
										Poweroffmscan = 1800;
								    	}
								    else
								    	{
										Poweroffmscan = 300;
								    	}

								
                                                   cameravalue_ok =0;
							}
						
				 	}
				else if( (zz[i] ==  Poweroff_5scan) && (cameravalue_ok == 13))
				 	{
				 		zz_copy[cameravalue_ok] =zz[i];
				 		cameravalue_ok++;
						if(cameravalue_ok >= 14)
							{
								    if(scan_head == 1)
								    	{
										Poweroffmscan = 9000;
								    	}
								    else
								    	{
										Poweroffmscan = 600;
								    	}
								
                                                   cameravalue_ok =0;
							}
						
				 	}
				else if( (zz[i] ==  bonddelteasci) && (cameravalue_ok == 13))
				 	{
				 		zz_copy[cameravalue_ok] =zz[i];
				 		cameravalue_ok++;
						if(cameravalue_ok >= 14)
							{
								poweroffbond = 1;
								
                                                   cameravalue_ok =0;
							}
						
				 	}
				else if( (zz[i] ==  dataoffline_on) && (cameravalue_ok == 13))
				 	{
				 		zz_copy[cameravalue_ok] =zz[i];
				 		cameravalue_ok++;
						if(cameravalue_ok >= 14)
							{
								dataoffline = 1;
								
                                                   cameravalue_ok =0;
							}
						
				 	}
				else if( (zz[i] ==  dataline_off) && (cameravalue_ok == 13))
				 	{
				 		zz_copy[cameravalue_ok] =zz[i];
				 		cameravalue_ok++;
						if(cameravalue_ok >= 14)
							{
								dataoffline = 0;
								
                                                   cameravalue_ok =0;
							}
						
				 	}
				 else if( (zz[i] ==  'V')&& (cameravalue_ok == 13) ){
										    BLEtime_out = BLEtime_out > 10 ? 10:100;
                                            cameravalue_ok =0;
				 	}
                                 else if( (zz[i] ==  'W')&& (cameravalue_ok == 13) ){
                                            extern void appForceBoot(void);
                                            appForceBoot();
				 	}

				 else
				 	{
				 		if(cameravalue_ok !=0)
				 			{
				 				for(j=0;j<cameravalue_ok;j++)
				 					{
				 						Serialdatawriteflash(zz_copy[j]);
				 								
				 					}
								cameravalue_ok =0;
								Serialdatawriteflash(zz[i]);	
								
								osal_start_timerEx( hidEmuKbdTaskId, START_UART_DATAEND,16);
				 			}
						else
							{
								Serialdatawriteflash(zz[i]);
								osal_start_timerEx( hidEmuKbdTaskId, START_UART_DATAEND,16);
								
							}
				 	}
				
			}
			scan_flag = 1;
                P0_6= 1;
               Powerkey =0;
               P06time =0;
               AUTOpowertime =  OSAL_timeSeconds;		   
                    if((voice_start == 0) && ( P0_2 == 0) && ( (sound_on[0] &0x01) == 0x01) )
                    {
                            voice_start = 2;
                            voice_reset = 1;
                            P0_4 = 0x1;
                            osal_start_timerEx( hidEmuKbdTaskId, START_voice_change,110); 
                    }
                    uint8 param;
                    GAPRole_GetParameter(GAPROLE_ADVERT_ENABLED, &param);
                    if(param == FALSE){
                     param = TRUE;
                     GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &param );
                    }
        }
    }
}


/*********************************************************************
 * @fn      hidEmuKbd_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */

uint32 keyboardfirsttime =0;

static void hidEmuKbd_HandleKeys( uint8 shift, uint8 keys )
{

 // uint8 i=0;

  (void)shift;  // Intentionally unreferenced parameter

  if ( (keys & HAL_KEY_SW_1)  )                                                                            //OK
  {
		
			if((OSAL_timeSecondsMS - keyboardfirsttime) >200)
			{
		        		if((P06time==0) && (keyrepeat == 0))
		    			{
				     		 P0_6= 0;
						 P06time =1;  
						 keyrepeat = 1;
                                                 scan_flag = 0;
						 osal_start_timerEx( hidEmuKbdTaskId, START_Gled_onoff,100);
						
		    			}
				keyboardfirsttime = OSAL_timeSecondsMS;
						
			}

  }
  else if ( !(keys & HAL_KEY_SW_1) )
  {

  }

  
}

/*********************************************************************
 * @fn      hidEmuKbd_ProcessGattMsg
 *
 * @brief   Process GATT messages
 *
 * @return  none
 */
static void hidEmuKbd_ProcessGattMsg( gattMsgEvent_t *pMsg )
{
  GATT_bm_free( &pMsg->msg, pMsg->method );
}

/*********************************************************************
 * @fn      hidEmuKbdSendReport
 *
 * @brief   Build and send a HID keyboard report.
 *
 * @param   keycode - HID keycode.
 *
 * @return  none
 */

static void hidEmuKbdSendReport_chinese( uint8 keycode )
{
  uint8 buf[HID_KEYBOARD_IN_RPT_LEN];

  buf[0] = 0x40;         // Modifier keys
  buf[1] = 0;         // Reserved
  buf[2] = keycode;   // Keycode 1
  buf[3] = 0;         // Keycode 2
  buf[4] = 0;         // Keycode 3
  buf[5] = 0;         // Keycode 4
  buf[6] = 0;         // Keycode 5
  buf[7] = 0;         // Keycode 6

  HidDev_Report( HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                HID_KEYBOARD_IN_RPT_LEN, buf );
}
static void hidEmuKbdSendReport( uint8 keycode )
{
  uint8 buf[HID_KEYBOARD_IN_RPT_LEN];

  buf[0] = shift_key;         // Modifier keys
  buf[1] = 0;         // Reserved
  buf[2] = keycode;   // Keycode 1
  buf[3] = 0;         // Keycode 2
  buf[4] = 0;         // Keycode 3
  buf[5] = 0;         // Keycode 4
  buf[6] = 0;         // Keycode 5
  buf[7] = 0;         // Keycode 6

  HidDev_Report( HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                HID_KEYBOARD_IN_RPT_LEN, buf );
}

static void hidUSBKbdSendReport_chinese( uint8 keycode )
{
  uint8 buf[HID_KEYBOARD_IN_RPT_LEN];

  buf[0] = 0x40;         // Modifier keys
  buf[1] = 0;         // Reserved
  buf[2] = keycode;   // Keycode 1
  buf[3] = 0;         // Keycode 2
  buf[4] = 0;         // Keycode 3
  buf[5] = 0;         // Keycode 4
  buf[6] = 0;         // Keycode 5
  buf[7] = 0;         // Keycode 6



  hidSendHidInReport(buf, USB_HID_KBD_EP, 8);
}
static void hidUSBKbdSendReport( uint8 keycode )
{
  uint8 buf[HID_KEYBOARD_IN_RPT_LEN];

  buf[0] = shift_key;         // Modifier keys
  buf[1] = 0;         // Reserved
  buf[2] = keycode;   // Keycode 1
  buf[3] = 0;         // Keycode 2
  buf[4] = 0;         // Keycode 3
  buf[5] = 0;         // Keycode 4
  buf[6] = 0;         // Keycode 5
  buf[7] = 0;         // Keycode 6

  hidSendHidInReport(buf, USB_HID_KBD_EP, 8);
}

/*********************************************************************
 * @fn      hidEmuKbdSendMouseReport
 *
 * @brief   Build and send a HID mouse report.
 *
 * @param   buttons - Mouse button code
 *
 * @return  none
 */
//static void hidEmuKbdSendMouseReport( uint8 buttons )
//{
//  uint8 buf[HID_MOUSE_IN_RPT_LEN];
//
//  buf[0] = buttons;   // Buttons
//  buf[1] = 0;         // X
//  buf[2] = 0;         // Y
//  buf[3] = 0;         // Wheel
//  buf[4] = 0;         // AC Pan
//
//  HidDev_Report( HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT,
//                 HID_MOUSE_IN_RPT_LEN, buf );
//}

/*********************************************************************
 * @fn      hidEmuKbdRcvReport
 *
 * @brief   Process an incoming HID keyboard report.
 *
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  status
 */
static uint8 hidEmuKbdRcvReport( uint8 len, uint8 *pData )
{
  // verify data length
  if ( len == HID_LED_OUT_RPT_LEN )
  {
    // set keyfob LEDs
    HalLedSet( HAL_LED_1, ((*pData & LED_CAPS_LOCK) == LED_CAPS_LOCK) );
    HalLedSet( HAL_LED_2, ((*pData & LED_NUM_LOCK) == LED_NUM_LOCK) );

    return SUCCESS;
  }
  else
  {
    return ATT_ERR_INVALID_VALUE_SIZE;
  }
}

/*********************************************************************
 * @fn      hidEmuKbdRptCB
 *
 * @brief   HID Dev report callback.
 *
 * @param   id - HID report ID.
 * @param   type - HID report type.
 * @param   uuid - attribute uuid.
 * @param   oper - operation:  read, write, etc.
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  GATT status code.
 */
static uint8 hidEmuKbdRptCB( uint8 id, uint8 type, uint16 uuid,
                             uint8 oper, uint8 *pLen, uint8 *pData )
{
  uint8 status = SUCCESS;

  // write
  if ( oper == HID_DEV_OPER_WRITE )
  {
    if ( uuid == REPORT_UUID )
    {
      // process write to LED output report; ignore others
      if ( type == HID_REPORT_TYPE_OUTPUT )
      {
        status = hidEmuKbdRcvReport( *pLen, pData );
      }
    }

    if ( status == SUCCESS )
    {
      status = HidKbd_SetParameter( id, type, uuid, *pLen, pData );
    }
  }
  // read
  else if ( oper == HID_DEV_OPER_READ )
  {
    status = HidKbd_GetParameter( id, type, uuid, pLen, pData );
  }
  // notifications enabled
  else if ( oper == HID_DEV_OPER_ENABLE )
  {
    if ( id == HID_RPT_ID_MOUSE_IN && type == HID_REPORT_TYPE_INPUT )
    {
      hidBootMouseEnabled = TRUE;
    }
  }
  // notifications disabled
  else if ( oper == HID_DEV_OPER_DISABLE )
  {
    if ( id == HID_RPT_ID_MOUSE_IN && type == HID_REPORT_TYPE_INPUT )
    {
      hidBootMouseEnabled = FALSE;
    }
  }

  return status;
}

/*********************************************************************
 * @fn      hidEmuKbdEvtCB
 *
 * @brief   HID Dev event callback.
 *
 * @param   evt - event ID.
 *
 * @return  HID response code.
 */
static void hidEmuKbdEvtCB( uint8 evt )
{
  // process enter/exit suspend or enter/exit boot mode

  return;
}
static uint16 crc16(uint16 crc, uint8 val)
{
  const uint16 poly = 0x1021;
  uint8 cnt;

  for (cnt = 0; cnt < 8; cnt++, val <<= 1)
  {
    uint8 msb = (crc & 0x8000) ? 1 : 0;

    crc <<= 1;
    if (val & 0x80)  crc |= 0x0001;
    if (msb)         crc ^= poly;
  }

  return crc;
}
//static uint16 calcCRC(void)
//{
//  uint32 addr;
//  uint16 crc = 0;
//  uint8 buf[4];
//
//  for (addr = (uint32)(firmware_addr+0x1000); addr < ((uint32)0x3F000+firmware_addr); addr += 4)
//  {
//    if (addr != (firmware_addr+0x1090))
//    {
//      Flash_FAST_ReadData(addr, 4, buf);
//      for (uint8 idx = 0; idx < 4; idx++)
//      {
//        crc = crc16(crc, buf[idx]);
//      }
//    }
//  }
//
//  // IAR note explains that poly must be run with value zero for each byte of crc.
//  crc = crc16(crc, 0);
//  crc = crc16(crc, 0);
//
//  return crc;
//}
/*********************************************************************
*********************************************************************/
