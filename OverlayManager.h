#ifndef OVERLAYMANAGER_H
#define OVERLAYMANAGER_H

#include "basetypes.h"
//#include "external\MurmurHash3.h"
#define XXH_INLINE_ALL
#include "external\xxhash.h"
#include <list>

class OverlayManager
{
public:

  OverlayManager()
  {
    numOverlays = 0;

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

    overlayLength = 0;
    overlayShift = 0;
    maxOverlays = 0;
    overlayBuffer = nullptr;
  }

  uint32 GetOverlaysInUse() const
  {
    return numOverlays;
  }

  void SetOverlayBufferAndLength(const uint32* const buffer, const uint32 len)
  {
    overlayBuffer = buffer;
    overlayLength = len;

    overlayShift = (overlayLength == 4096) ? 12 : ((overlayLength == 8192) ? 13 : 0);
    maxOverlays = (overlayLength == 4096) ? 256 : ((overlayLength == 8192) ? 128 : 0);

    assert(overlayShift != 0);
  }

  // returns the overlay mask
  uint32 FindOverlay(bool& bInvalidate)
  {
    bInvalidate = false;

    const uint64 hash = Hash(overlayBuffer,overlayLength);

    for(std::list<std::pair<uint32,uint64>>::iterator it = overlayLRU.begin(); it != overlayLRU.end(); ++it)
      if(it->second == hash)
      {
        const uint32 idx = it->first;
        overlayLRU.erase(it);
        overlayLRU.push_front(std::pair<uint32,uint64>(idx,hash));

        //!! this assumes that there are no hash collisions!
        //!!  should add debug code that also stores full MPE IRAM region (overlayBuffer -> overlayLength) in addition to be able to do a full comparison in here
        return idx << overlayShift;
      }

    bInvalidate = true;

    uint32 idx;
    if(numOverlays < maxOverlays) // enough entries left? simply increase counter
    {
      idx = numOverlays++;
    }
    else // need to remove last element from the list (=least recently used) 
    {
      idx = overlayLRU.back().first;
      overlayLRU.pop_back();
    }
    overlayLRU.push_front(std::pair<uint32,uint64>(idx,hash)); // push new element to beginning (=most recently used)

    return idx << overlayShift;
  }

private:
  static uint64 Hash(const uint32* data, const uint32 length)
  {
    const uint64 result
    //MurmurHash3_x86_32((void*)data, length, 0xdeadbeef, &result);
    = XXH3_64bits((void*)data, length);
    return (result == 0) ? 1 : result; // must remove 0, as this is used for init'ing the tables

    /*const uint32 * const e = (uint8*)data + length;

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
  uint32 maxOverlays; // dependent on overlayLength, so at the moment 128 or 256, more is not possible for Aries 2
  uint8  overlayShift;// dependent on overlayLength, so at the moment 12 or 13

  std::list<std::pair<uint32,uint64>> overlayLRU; // stores the idx of the overlay, and the 64bit-hash of that buffer

  uint32 overlayLength; // 4096 or 8192, depending on which MPE of an Aries 2
  const uint32* overlayBuffer;

  //uint32 crctab[256];
};

#endif
