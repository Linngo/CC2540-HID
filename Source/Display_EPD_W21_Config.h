
#ifndef _DISPLAY_EPD_W21_CONFIG_H_
#define _DISPLAY_EPD_W21_CONFIG_H_

	#define xDot 200
	#define yDot 200

	unsigned char GDOControl[]={0x01,(yDot-1)%256,(yDot-1)/256,0x00}; //for 1.54inch
	unsigned char softstart[]={0x0c,0xd7,0xd6,0x9d};
	unsigned char Rambypass[] = {0x21,0x8f};		// Display update
	unsigned char MAsequency[] = {0x22,0xf0};		// clock 
	unsigned char GDVol[] = {0x03,0x66};	// Gate voltage +18V/-18V
	unsigned char SDVol[] = {0x04,0x04};	// Source voltage +12V/-12V
	unsigned char VCOMVol[] = {0x2c,0x9a};	// VCOM 7c
	unsigned char BOOSTERFB[] = {0xf0,0x1f};	// Source voltage +15V/-15V
	unsigned char DummyLine[] = {0x3a,0x1a};	// 4 dummy line per gate
	unsigned char Gatetime[] = {0x3b,0x08};	// 2us per line
	unsigned char BorderWavefrom[] = {0x3c,0x33};	// Border
	unsigned char analogblock[] = {0x74,0x54};	// analog block control
	unsigned char digitalblock[] = {0x7E,0x3B};	//digital block control  
	unsigned char Driveroutput[] = {0x01,0x97,0x00,0x00};	// Driver output control control
	unsigned char RamDataEntryMode[] = {0x11,0x01};	// Ram data entry mode
	unsigned char choiceRam[] = {0x21,0x40};	// choice Ram
	unsigned char Ramxadress[] = {0x44,0x00,0x12};	// Ram-X address start
	unsigned char Ramyadress[] = {0x45,0x97,0x00,0x00,0x00};	// Ram-y address start
	unsigned char Setborder[] = {0x3c,0x02};	// set border 

	unsigned char SetRamX[] = {0x4E,0x00};	// Set RamX 
	unsigned char SetRamY[] = {0x4F,0x97,0x00};	// Set RamY

#endif
/***********************************************************
						end file
***********************************************************/


