#include "spi.h"
#include  "hal_adc.h"





#define FLASH_WRITE_EN 		0x06
#define FLASH_WRITE_DISABLE 0x04
#define FLASH_READ_ID		0x9F
#define FLASH_READ_STATUS   0x05
#define FLASH_WRITE_STATUS  0x01
#define FLASH_READ_DATA		0x03
#define FLASH_FastRead		0x0B
#define FLASH_SE			0x20
#define FLASH_BE			0xD8
#define FLASH_CE			0x60
#define FLASH_PP  			0x02
#define FLASH_DP			0xB9



#define Dummy_Byte	 0xFF


void Select_Flash(void)
{
  	P1_2 = 0;
}

void NotSelect_Flash(void)
{
 	P1_2 = 1;
}


/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte 
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
 
uint8 SPI_Flash_SendByte(uint8 byte)
{

  uint8 tmp=0;	
 
  while ((U0CSR & 0x01) == 1 ); 
  U0CSR &= ~0x02;
  U0DBUF = byte;
  while ((U0CSR & 0x02) == 0 ); 
  tmp =U0DBUF;


  return tmp;

}




/*******************************************************************************
* Function Name  : SPI_FLASH_ReadByte
* Description    : Reads a byte from the SPI Flash.
*                  This function must be used only if the Start_Read_Sequence
*                  function has been previously called.
* Input          : None
* Output         : None
* Return         : Byte Read from the SPI Flash.
*******************************************************************************/
uint8 SPI_Flash_ReadByte(void)
{
  return (SPI_Flash_SendByte(Dummy_Byte));
}


uint8 Flash_ReadStatus(void)
{
	uint8 data = 0;
	Select_Flash();	

  	SPI_Flash_SendByte(FLASH_READ_STATUS);


  	data = SPI_Flash_ReadByte();
  	NotSelect_Flash();
	return data;
}
void Flash_WriteEN(void)
{
   	Select_Flash();	
  	SPI_Flash_SendByte(FLASH_WRITE_EN);
  	NotSelect_Flash();
}
void Flash_WriteStatus(uint8 data)
{
	Flash_WriteEN();
	Select_Flash();	

  	SPI_Flash_SendByte(FLASH_WRITE_STATUS);

  	SPI_Flash_SendByte(data);
  	NotSelect_Flash();

	while((Flash_ReadStatus()&0x01) == 0x01)
	{
	   //for(i=0; i<10;i++) ;
	}

	
}

void Flash_WriteByte(uint32 addr, uint8 data)
{
//	uint8 i = 0;
	uint8 status = 0;

	while((Flash_ReadStatus()&0x01) == 0x01)
	{
	   //for(i=0; i<10;i++) ;
	}
	
	status = Flash_ReadStatus();
	if((status&0x7C) != 0x00)
		{
			Flash_WriteStatus(0);
		}

	Flash_WriteEN();
	Select_Flash();
  	SPI_Flash_SendByte(FLASH_PP);
	SPI_Flash_SendByte((addr>>16)&0xff);
	SPI_Flash_SendByte((addr>>8)&0xff);
	SPI_Flash_SendByte(addr&0xff);
	SPI_Flash_SendByte(data);
	NotSelect_Flash();
	while((Flash_ReadStatus()&0x01) == 0x01)
	{
	   //for(i=0; i<10;i++) ;
	}
	//Flash_WriteEN();
	//Flash_WriteStatus(status);
}
void Flash_ChipErase(void)
{
	//uint8 i = 0;
	Flash_WriteEN();
	Select_Flash();
  	SPI_Flash_SendByte(FLASH_CE);

	NotSelect_Flash();
	while((Flash_ReadStatus()&0x01) == 0x01)
	{
	  // for(i=0; i<100;i++) ;
	}	
}
extern void ttttttttt(void);
void Flash_SectorErase(uint32 addr)
{
	//uint8 i = 0;
	//if((addr != 0x23000) && (addr != 0x25000))
		
	Flash_WriteEN();
	Select_Flash();
  	SPI_Flash_SendByte(FLASH_SE);
	SPI_Flash_SendByte((addr>>16)&0xff);
	SPI_Flash_SendByte((addr>>8)&0xff);
	SPI_Flash_SendByte(addr&0xff);
	NotSelect_Flash();
	while((Flash_ReadStatus()&0x01) == 0x01)
	{
	  // for(i=0; i<100;i++) ;
	}
		

}

void Flash_Sector_Erase(uint32 addr)
{
	//uint8 i = 0;
		//if((addr != 0x23000) && (addr != 0x25000))
			//{
	while((Flash_ReadStatus()&0x01) == 0x01)
	{
	   while(1);
	}
	Flash_WriteEN();
	Select_Flash();
  	SPI_Flash_SendByte(FLASH_SE);
	SPI_Flash_SendByte((addr>>16)&0xff);
	SPI_Flash_SendByte((addr>>8)&0xff);
	SPI_Flash_SendByte(addr&0xff);
	NotSelect_Flash();
	//while((Flash_ReadStatus()&0x01) == 0x01)
	//{
	  // for(i=0; i<100;i++) ;
	//}	
			//}
		//else
			//{
			//ttttttttt();
			//}
}

uint8 Flash_ReadWip(void)
{
	
	if((Flash_ReadStatus()&0x01) == 0x01)
		{
			return 1;
		}
	else
		{
			return 0;
		}

}


void Flash_ReadData(uint32 addr, uint8 *data)
{
	Select_Flash();	

  	SPI_Flash_SendByte(FLASH_READ_DATA);
	SPI_Flash_SendByte((addr>>16)&0xff);
	SPI_Flash_SendByte((addr>>8)&0xff);
	SPI_Flash_SendByte(addr&0xff);
  	data[0] = SPI_Flash_ReadByte();
  	NotSelect_Flash();
}
void Flash_FAST_ReadData(uint32 addr, uint32 rsize, uint8 *data)
{
	uint32 i;
	Select_Flash();	

  	SPI_Flash_SendByte(FLASH_FastRead);
	SPI_Flash_SendByte((addr>>16)&0xff);
	SPI_Flash_SendByte((addr>>8)&0xff);
	SPI_Flash_SendByte(addr&0xff);
	SPI_Flash_SendByte(Dummy_Byte);
	for(i=0; i<rsize; i++)
  		data[i] = SPI_Flash_ReadByte();
  	NotSelect_Flash();
}


/*********************SPI初始化************************************/
void SPI_Flash_Init(void)     
{  

		PERCFG |= 0x01;        // map USART0 to its alternative 1 location. P1_2: SSN, P13: SCK, P1_4: MOSI, P1_5: MISO
		P1SEL |= 0x38;        // P1_3,4,5 are peripherals
		P1SEL &= ~0x4;        // P1_2 is GPIO (SSN)
		P1DIR |= 0x04;        // SSN is set as output,复位为0是输入模式

		

		U0CSR = 0x40;           //允许接收 
		U0BAUD = 00;
		U0GCR  = 0xF0;    //cp0l=0,cpha=0,msb first , baud_e=15

} 
uint8 bm=0,zzw =0;

void  FlashReadID(void)
{

		//while(1)
  
                {
		Select_Flash();

		SPI_Flash_SendByte(0x90);
		SPI_Flash_SendByte(0x00);
		SPI_Flash_SendByte(0x00);
		SPI_Flash_SendByte(0x00);
		zzw=SPI_Flash_ReadByte();
		 
			bm=SPI_Flash_ReadByte();
                        if((zzw != 0xE0) || (bm != 0x14))
                          {
                              //while(1);
                           }
                      //  zzw=0;
                     //   bm=0;
			
	  	       NotSelect_Flash();
                }
                
	  	       
	
}
