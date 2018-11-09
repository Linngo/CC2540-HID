#include "hal_types.h"  
void  FlashReadID(void);
void SPI_Flash_Init(void);
void Flash_ReadData(uint32 addr, uint8 *data);
void Flash_FAST_ReadData(uint32 addr, uint32 rsize, uint8 *data);
uint8 Flash_ReadStatus(void);
void Flash_WriteByte(uint32 addr, uint8 data);
void Flash_ChipErase(void);
void Flash_SectorErase(uint32 addr);
void Flash_Sector_Erase(uint32 addr);
uint8 Flash_ReadWip(void);
void Flash_WriteStatus(uint8 data);
uint8 SPI_Flash_SendByte(uint8 byte);