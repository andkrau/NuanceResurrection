#include "basetypes.h"
#include <cstdio>
#include "FlashEEPROM.h"

void FlashEEPROM::WriteData(uint32 address, uint32 data1)
{
  const uint32 commandAddress = (address >> 1) & 0x7FF;
  data1 >>= 24;

  switch(mode)
  {
    case eFlashMode::FlashMode_Read:
      switch(state)
      {
        case EEPROM_STATE_READ:
          if(((commandAddress & 0xFF) == 0x55) && (data1 == 0x98))
          {
            mode = eFlashMode::FlashMode_CFI;
            errorMode = eFlashMode::FlashMode_CFI;
            state = EEPROM_STATE_READ;
            errorState = EEPROM_STATE_READ;
          }
          else
          {
            if((commandAddress == 0x555) && (data1 == 0xAA))
            {
              state = EEPROM_STATE_BUS_CYCLE_2;
            }
            else
            {
              mode = errorMode;
              state = errorState;
            }
          }
          break;
        case EEPROM_STATE_BUS_CYCLE_2:
          if((commandAddress == 0x2AA) && (data1 == 0x55))
          {
            state = EEPROM_STATE_BUS_CYCLE_3;
          }
          else
          {
            mode = errorMode;
            state = errorState;
          }
          break;
        case EEPROM_STATE_BUS_CYCLE_3:
          if(commandAddress == 0x555)
          {
            switch(data1)
            {
              case 0x80:
                //Next three bus cycles needed to trigger sector erase, chip erase or sector lockdown
                mode = eFlashMode::FlashMode_Erase;
                state = EEPROM_STATE_BUS_CYCLE_4;
                break;
              case 0x90:
                //Enter ProductID mode
                mode = eFlashMode::FlashMode_ProductID;
                errorMode = eFlashMode::FlashMode_ProductID;
                state = EEPROM_STATE_READ;
                errorState = EEPROM_STATE_READ;
                break;
              case 0xA0:
                //Next bus cycle expected to be write address and data
                mode = eFlashMode::FlashMode_Program;
                state = EEPROM_STATE_BUS_CYCLE_4;
                break;
              default:
                mode = errorMode;
                state = errorState;
                break;
            }
          }
          else
          {
            mode = errorMode;
            state = errorState;
          }
          break;
      }
      break;
    case eFlashMode::FlashMode_CFI:
      if(data1 == 0xF0)
      {
        mode = eFlashMode::FlashMode_Read;
        errorMode = eFlashMode::FlashMode_Read;
        state = EEPROM_STATE_READ;
        errorState = EEPROM_STATE_READ;
      }     
      break;
    case eFlashMode::FlashMode_ProductID:
      if(data1 == 0xF0)
      {
        mode = eFlashMode::FlashMode_Read;
        errorMode = eFlashMode::FlashMode_Read;
        state = EEPROM_STATE_READ;
        errorState = EEPROM_STATE_READ;
      }
      break;
    case eFlashMode::FlashMode_Program:
      if(!IsSectorLocked(address))
      {
        //Cannot turn 0's to 1's, only 1's to 0's (0e op 0w: 0e, 0e op 1w: 0e, 1e op 0w: 0e, 1e op 1w: 1e)
        eeprom[address] &= data1;
      }
      mode = eFlashMode::FlashMode_Read;
      errorMode = eFlashMode::FlashMode_Read;
      state = EEPROM_STATE_READ;
      errorState = EEPROM_STATE_READ;
      break;
    case eFlashMode::FlashMode_Erase:
    {
      switch(state)
      {
        case EEPROM_STATE_BUS_CYCLE_4:
          if((commandAddress == 0x555) && (data1 == 0xAA))
          {
            state = EEPROM_STATE_BUS_CYCLE_5;
          }
          else
          {
            mode = errorMode;
            state = errorState;
          }
          break;
        case EEPROM_STATE_BUS_CYCLE_5:
          if((commandAddress == 0x2AA) && (data1 == 0x55))
          {
            state = EEPROM_STATE_BUS_CYCLE_6;
          }
          else
          {
            mode = errorMode;
            state = errorState;
          }
          break;
        case EEPROM_STATE_BUS_CYCLE_6:
        {
          switch(data1)
          {
            case 0x10:
              //Erase Chip
              if(commandAddress == 0x555)
              {
                EraseChip();
              }
              mode = eFlashMode::FlashMode_Read;
              errorMode = eFlashMode::FlashMode_Read;
              state = EEPROM_STATE_READ;
              errorState = EEPROM_STATE_READ;              
              break;
            case 0x30:
              //Erase Sector
              EraseSector(address);
              mode = eFlashMode::FlashMode_Read;
              errorMode = eFlashMode::FlashMode_Read;
              state = EEPROM_STATE_READ;
              errorState = EEPROM_STATE_READ;              
              break;
            case 0x60:
              //Sector Lockdown
              LockSector(address);
              mode = eFlashMode::FlashMode_Read;
              errorMode = eFlashMode::FlashMode_Read;
              state = EEPROM_STATE_READ;
              errorState = EEPROM_STATE_READ;
              break;
            default:
              break;
          }
          break;
        }
      }
    }
    break;
    case eFlashMode::FlashMode_Error:
    case eFlashMode::FlashMode_SinglePulseProgram:
      assert(!"not implemented");
      break;
  }
}

void FlashEEPROM::ReadData(uint32 address, uint32 *buffer)
{
  switch(mode)
  {
    case eFlashMode::FlashMode_Read:
      buffer[0] = 
        (((uint32)(eeprom[ address      & (DEFAULT_EEPROM_SIZE - 1)])) << 24) |
        (((uint32)(eeprom[(address + 1) & (DEFAULT_EEPROM_SIZE - 1)])) << 16) |
        (((uint32)(eeprom[(address + 2) & (DEFAULT_EEPROM_SIZE - 1)])) << 8) |
        (((uint32)(eeprom[(address + 3) & (DEFAULT_EEPROM_SIZE - 1)])));
      break;
    case eFlashMode::FlashMode_ProductID:
      if(address & 0x02)
      {
        buffer[0] = (eepromDeviceID << 24);
      }
      else
      {
        buffer[0] = (eepromManufacturerCode << 24);
      }
      break;
    case eFlashMode::FlashMode_CFI:
      break;
    default:
      break;
  }
}

uint32 FlashEEPROM::ReadStatus()
{
  return 0;
}

bool FlashEEPROM::IsSectorLocked(uint32 address)
{
  const uint32 i = (address < 0x10000) ?
    //Bottom boot sector (8Kbyte)
    (address >> 13) :
    //normal sector (64Kbyte)
    (address >> 16);
  
  return sectorLockStatus[i];
}

void FlashEEPROM::EraseChip()
{
  for(uint32 i = 0; i < DEFAULT_NUM_SECTORS; i++)
  {
    if(i < 8)
    {
      //Bottom boot sector (8 Kbytes)
      EraseSector(i << 13);
    }
    else
    {
      //Normal sector (64 Kbytes)
      EraseSector(i << 16);
    }
  }
}

void FlashEEPROM::EraseSector(uint32 address)
{
  const uint32 sectorSize = (address < 0x10000) ? 8192 : 65536;
  
  address &= ~(sectorSize - 1);

  if(!IsSectorLocked(address))
  {
    uint8 *pEEPROM = &eeprom[address];
    for(uint32 i = 0; i < sectorSize; i++)
    {
      //Erase turns 0's to 1's
      *pEEPROM++ = 0xFF;
    }
  }
}

void FlashEEPROM::LockSector(uint32 address)
{
  const uint32 i = (address < 0x10000) ?
    //Bottom boot sector (8Kbyte)
    (address >> 13) :
    //normal sector (64Kbyte)
    (address >> 16);
  
  sectorLockStatus[i] = true;
}

void FlashEEPROM::LoadFromFile(const char * const fileName)
{
  FILE *inFile = fopen(fileName,"rb");

  if(inFile)
  {
    fseek(inFile,0,SEEK_END);
    uint32 fileLength = ftell(inFile) + 1;
    if(fileLength > DEFAULT_EEPROM_SIZE)
    {
      fileLength = DEFAULT_EEPROM_SIZE;
    }

    fseek(inFile,0,SEEK_SET);
    for(uint32 i = 0; i < fileLength; i++)
    {
      eeprom[i] = (uint8)(fgetc(inFile));
    }
    fclose(inFile);
  }
}

void FlashEEPROM::SaveToFile(const char * const fileName)
{
  FILE *outFile = fopen(fileName,"wb");

  if(outFile)
  {
    for(uint32 i = 0; i < DEFAULT_EEPROM_SIZE; i++)
    {
      fputc((int)(eeprom[i]),outFile);
    }
    fclose(outFile);
  }
}
