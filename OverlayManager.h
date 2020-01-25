#ifndef OVERLAYMANAGER_H
#define OVERLAYMANAGER_H

#include "basetypes.h"
#include "external\MurmurHash3.h"

class OverlayManager
{
public:

  OverlayManager()
  {
    numOverlays = 0;
    currentOverlayIndex = 0;

    //#define QUOTIENT (0x04C11DB7UL)
    /*for (uint32 i = 0; i < 256; i++)
    {
      uint32 crc = i << 24;
      for (uint32 j = 0; j < 8; j++)
      {
        if(crc & 0x80000000)
          crc = (crc << 1) ^ QUOTIENT;
        else
          crc = crc << 1;
      }

      //If host machine is little endian, convert crc to big endian

      SwapScalarBytes(&crc);
      crctab[i] = crc;
    }*/

    for(uint32 i = 0; i < 128; i++)
      overlayHash[i] = 0;

    overlayLength = 0;

    replaceOverlayCounter = 0;
  }

  uint32 GetOverlaysInUse() const
  {
    return numOverlays;
  }

  uint32 GetOverlayMask() const
  {
    return (currentOverlayIndex << 13);
  }

  void SetOverlayLength(const uint32 len)
  {
    overlayLength = len;
  }

  uint32 FindOverlay(const uint32* const buffer, bool& bInvalidate)
  {
    bInvalidate = false;

    const uint32 hash = Hash(buffer);

    for(uint32 i = 0; i < numOverlays; i++)
    {
      if(overlayHash[i] == hash)
      {
        currentOverlayIndex = i;
        return i;
      }
    }

    bInvalidate = true;

    uint32 i;
    if(numOverlays < 128)
    {
      i = numOverlays++;
    }
    else
    {
      i = replaceOverlayCounter++;

      if(replaceOverlayCounter >= numOverlays)
        replaceOverlayCounter = 0;
    }

    currentOverlayIndex = i;
    overlayHash[i] = hash;

    return i;
  }

private:
  uint32 Hash(const uint32* data) const
  {
    uint32 result;
    MurmurHash3_x86_32((void*)data, overlayLength * 4, 0xdeadbeef, &result); //!! overkill to use this HQ hash?? // can be 4k or 8k * 4 bytes of data!!!
    return (result == 0) ? 1 : result; // remove 0, as this is used for init'ing the tables

    /*const uint32 * const e = data + overlayLength;

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
    
    return ~result;*/
  }

  uint32 numOverlays;
  uint32 currentOverlayIndex;
  //uint32 crctab[256];
  uint32 replaceOverlayCounter;
  uint32 overlayHash[128];
  uint32 overlayLength;
};

#endif
