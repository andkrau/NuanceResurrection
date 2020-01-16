#ifndef OVERLAYMANAGER_H
#define OVERLAYMANAGER_H

#include "basetypes.h"

class OverlayManager
{
public:

  OverlayManager();
  
  uint32 GetOverlaysInUse() const
  {
    return numOverlays;
  }

  uint32 GetOverlayMask() const
  {
    return (currentOverlayIndex << 13);
  }

  uint32 GetOverlayIndex(const uint32 index) const
  {
    return currentOverlayIndex;
  }

  void SetOverlayIndex(const uint32 index)
  {
    currentOverlayIndex = index;
  }

  void SetOverlayLength(const uint32 len)
  {
    overlayLength = len;
  }

  uint32 Hash(const uint32 *buffer);
  uint32 FindOverlay(const uint32 * const buffer, bool &bInvalidate);

private:
  uint32 numOverlays;
  uint32 currentOverlayIndex;
  uint32 crctab[256];
  uint32 overlayHash[128];
  uint32 overlayLength;
};

#endif