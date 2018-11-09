
#ifndef _DISPLAY_EPD_W21_H_
#define _DISPLAY_EPD_W21_H_

//#define  EPDDOT152
#define  EPDDOT200

extern void driver_delay_xms(unsigned long xms);


void EPD_Dis_Part(unsigned char xStart,unsigned char xEnd,unsigned long yStart,unsigned long yEnd,unsigned char *DisBuffer,unsigned char Label);
void EPD_Dis_Full(unsigned char *DisBuffer,unsigned char Label);
void EPD_init_Full(void);
void EPD_init_Part(void);
void Epaper_First_line(uint8 line,uint32 flash_address);
void Epaper_line(uint8 line);
void Epaper_Update(void);
#endif
/***********************************************************
						end file
***********************************************************/


