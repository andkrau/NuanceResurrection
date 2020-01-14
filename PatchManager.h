#ifndef PATCH_MANAGER_H
#define PATCH_MANAGER_H

#include "BaseTypes.h"

#define MAX_PATCH_LABELS (64)
#define MAX_PATCH_ENTRIES (64)

enum PatchType
{
  PatchType_Rel8 = 0,
  PatchType_Rel16,
  PatchType_Rel32,
  PatchType_Rel64,
  PatchType_Abs32,
  PatchType_Abs64,
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
    numPatches = 0;
    numLabels = 0;
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

  void AddPatch(uint8 *patchPtr, PatchType patchType, uint8 *basePtr, uint32 destLabel)
  {
    patchData[numPatches].patchPtr = patchPtr;
    patchData[numPatches].patchType = patchType;
    patchData[numPatches].basePtr = basePtr;
    patchData[numPatches].destLabel = destLabel;
    numPatches++;
  }

  void SetLabelPointer(uint32 labelIndex, uint8 *ptr)
  {
    labelPointers[labelIndex] = ptr;
    numLabels++;
  }

  uint8 *GetLabelPointer(uint32 labelIndex)
  {
    return labelPointers[labelIndex];
  }
};

#endif