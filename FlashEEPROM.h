#ifndef FLASHEEPROM_H
#define FLASHEEPROM_H

#include "basetypes.h"
#include <stdio.h>

//16 Mbit = 2 MByte
#define DEFAULT_EEPROM_SIZE (2 * 1024 * 1024)
#define DEFAULT_NUM_SECTORS (39)

///Device ID = AT49BV162A/163A
#define EEPROM_DEVICE_ID (0xC0L)
///Manufacturer = Atmel
#define EEPROM_MANUFACTURER_CODE (0x1FL)

#define EEPROM_STATE_READ (1)
#define EEPROM_STATE_BUS_CYCLE_2 (2)
#define EEPROM_STATE_BUS_CYCLE_3 (3)
#define EEPROM_STATE_BUS_CYCLE_4 (4)
#define EEPROM_STATE_BUS_CYCLE_5 (5)
#define EEPROM_STATE_BUS_CYCLE_6 (6)

enum class eFlashMode
{
  FlashMode_Read,
  FlashMode_CFI,
  FlashMode_SinglePulseProgram,
  FlashMode_Program,
  FlashMode_ProductID,
  FlashMode_Erase,
  FlashMode_Error
};

class FlashEEPROM
{
public:
  FlashEEPROM()
  {
    eepromSize = DEFAULT_EEPROM_SIZE;
    init_nuon_mem(eeprom, DEFAULT_EEPROM_SIZE);
    //Device ID: AT49BV162A/163A
    eepromDeviceID = 0xC0;
    //Manufacturer Code : Atmel
    eepromManufacturerCode = 0x1F;
    Reset();
    LoadFromFile("FlashEEPROM.bin");
  }

  ~FlashEEPROM()
  {
    SaveToFile("FlashEEPROM.bin");
  }

  uint8 *GetBasePointer()
  {
    return eeprom;
  }

  void Reset()
  {
    mode = eFlashMode::FlashMode_Read;
    errorMode = eFlashMode::FlashMode_Read;
    state = EEPROM_STATE_READ;
    errorState = EEPROM_STATE_READ;
    for(uint32 i = 0; i < DEFAULT_NUM_SECTORS; i++)
    {
      sectorLockStatus[i] = false;
    }
  }

  //Command functions
  void WriteData(uint32 address, uint32 data);
  void ReadData(uint32 address, uint32 *buffer);
  uint32 ReadStatus();

  //Sector functions
  bool IsSectorLocked(uint32 address);
  void EraseChip();
  void EraseSector(uint32 address);
  void LockSector(uint32 address);

  //File functions
  void LoadFromFile(const char * const fileName);
  void SaveToFile(const char * const fileName);

private:
  uint32 eepromSize;
  uint8 eeprom[DEFAULT_EEPROM_SIZE];
  uint32 state;
  uint32 errorState;
  uint32 eepromManufacturerCode;
  uint32 eepromDeviceID;
  eFlashMode mode;
  eFlashMode errorMode;
  bool sectorLockStatus[DEFAULT_NUM_SECTORS];
};

#endif
