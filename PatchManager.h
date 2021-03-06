#ifndef PATCH_MANAGER_H
#define PATCH_MANAGER_H

#include "basetypes.h"
#include <memory.h>

#define MAX_PATCH_LABELS (64)
#define MAX_PATCH_ENTRIES (64)

enum class PatchType
{
  PatchType_Rel8 = 0,
  PatchType_Rel16,
  PatchType_Rel32,
  PatchType_Rel64,
  PatchType_Abs32,
  PatchType_Abs64
};

struct PatchInfo
{
  uint8 *patchPtr;
  uint8 *basePtr;
  uint32 destLabel;
  PatchType patchType;
};

class PatchManager
{
public:
  PatchInfo patchData[MAX_PATCH_ENTRIES];
  uint8 *labelPointers[MAX_PATCH_LABELS];
  uint32 numPatches;
  uint32 numLabels;

  PatchManager()
  {
    memset(this,0,sizeof(PatchManager));
  }

  void ApplyPatches();

  void ClearPatches()
  {
    numPatches = 0;
  }

  void ClearLabels()
  {
    numLabels = 0;
  }

  void Reset()
  {
    numPatches = 0;
    numLabels = 0;
  }

  void AddPatch(uint8 * const patchPtr, const PatchType patchType, uint8 * const basePtr, const uint32 destLabel)
  {
    assert(numPatches < MAX_PATCH_ENTRIES);

    patchData[numPatches].patchPtr = patchPtr;
    patchData[numPatches].patchType = patchType;
    patchData[numPatches].basePtr = basePtr;
    patchData[numPatches].destLabel = destLabel;
    numPatches++;
  }

  void SetLabelPointer(const uint32 labelIndex, uint8 * const ptr)
  {
    assert(labelIndex < MAX_PATCH_LABELS);

    labelPointers[labelIndex] = ptr;
    numLabels++;
  }

  uint8 *GetLabelPointer(const uint32 labelIndex) const
  {
    assert(labelIndex < MAX_PATCH_LABELS);

    return labelPointers[labelIndex];
  }
};

#endif
