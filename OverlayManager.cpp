#include "basetypes.h"
#include "byteswap.h"
#include "OverlayManager.h"

#define QUOTIENT (0x04C11DB7UL)

OverlayManager::OverlayManager()
{
  numOverlays = 0;
  currentOverlayIndex = 0;

  for (uint32 i = 0; i < 256; i++)
  {
    uint32 crc = i << 24;
    for (uint32 j = 0; j < 8; j++)
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

  for(uint32 i = 0; i < 128; i++)
  {
    overlayHash[i] = 0;
  }

  overlayLength = 0;
}

uint32 OverlayManager::Hash(const uint32 *data)
{
  const uint32 * const e = (uint32 *)(data + overlayLength);

  uint32 result = ~*data++;
    
  while(data < e)
  {
    result = crctab[result & 0xff] ^ result >> 8;
    result = crctab[result & 0xff] ^ result >> 8;
    result = crctab[result & 0xff] ^ result >> 8;
    result = crctab[result & 0xff] ^ result >> 8;
    result ^= *data;
    data++;
    //data += 128;
  }
    
  return ~result;
}

uint32 OverlayManager::FindOverlay(const uint32 * const buffer, bool &bInvalidate)
{
  static uint32 replace = 0;
  
  bInvalidate = false;

  const uint32 hash = Hash(buffer);

  uint32 i;
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
