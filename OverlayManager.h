#ifndef OVERLAYMANAGER_H
#define OVERLAYMANAGER_H

#include "basetypes.h"

class OverlayManager
{
public:

  OverlayManager();
  
  uint32 GetOverlaysInUse()
  {
    return numOverlays;
  }

  uint32 GetOverlayMask()
  {
    return (currentOverlayIndex << 13);
  }

  uint32 GetOverlayIndex(uint32 index)
  {
    return currentOverlayIndex;
  }

  void SetOverlayIndex(uint32 index)
  {
    currentOverlayIndex = index;
  }

  void SetOverlayLength(uint32 len)
  {
    overlayLength = len;
  }

  uint32 Hash(uint32 *buffer);
  uint32 FindOverlay(uint32 *buffer, bool &bInvalidate);

private:
  uint32 numOverlays;
  uint32 currentOverlayIndex;
  uint32 crctab[256];
  uint32 overlayHash[128];
  uint32 overlayLength;
};

#endif