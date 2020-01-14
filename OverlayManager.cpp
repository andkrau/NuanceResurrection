#include "basetypes.h"
#include "byteswap.h"
#include "OverlayManager.h"

#define QUOTIENT (0x04C11DB7UL)

OverlayManager::OverlayManager()
{
  uint32 i, j, crc;

  numOverlays = 0;
  currentOverlayIndex = 0;

  for (i = 0; i < 256; i++)
  {
    crc = i << 24;
    for (j = 0; j < 8; j++)
    {
      if(crc & 0x80000000)
      {
        crc = (crc << 1) ^ QUOTIENT;
      }
      else
      {
        crc = crc << 1;
      }
    }

    //If host machine is little endian, convert crc to big endian

    SwapScalarBytes(&crc);
    crctab[i] = crc;
  }

  for(i = 0; i < 128; i++)
  {
    overlayHash[i] = 0;
  }

  overlayLength = 0;
}

uint32 OverlayManager::Hash(uint32 *data)
{
  uint32 result;
  uint32 *e = (uint32 *)(data + overlayLength);

  result = ~*data++;
    
  while(data < e)
  {
    result = crctab[result & 0xff] ^ result >> 8;
    result = crctab[result & 0xff] ^ result >> 8;
    result = crctab[result & 0xff] ^ result >> 8;
    result = crctab[result & 0xff] ^ result >> 8;
    result ^= *data;
    data += 1;
    //data += 128;
  }
    
  return ~result;
}

uint32 OverlayManager::FindOverlay(uint32 *buffer, bool &bInvalidate)
{
  uint32 i, hash;

  static uint32 replace = 0;
  
  bInvalidate = false;

  hash = Hash(buffer);

  for(i = 0; i < numOverlays; i++)
  {
    if(overlayHash[i] == hash)
    {
      currentOverlayIndex = i;
      return i;
    }
  }

  bInvalidate = true;

  if(numOverlays < 128)
  {
    i = numOverlays++;
  }
  else
  {
    i = replace++;

    if(replace >= numOverlays)
    {
      replace = 0;
    }
  }

  currentOverlayIndex = i;
  overlayHash[i] = hash;
  return i;
}
