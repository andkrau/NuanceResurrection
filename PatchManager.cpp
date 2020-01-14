#include "PatchManager.h"
#include "BaseTypes.h"

void PatchManager::ApplyPatches()
{
  uint32 i;

  for(i = 0; i < numPatches; i++)
  {
    switch(patchData[i].patchType)
    {
      case PatchType_Rel8:
        *((uint8 *)(patchData[i].patchPtr)) = (uint8)(labelPointers[patchData[i].destLabel] - patchData[i].basePtr);
        break;
      case PatchType_Rel16:
        *((uint16 *)(patchData[i].patchPtr)) = (uint16)(labelPointers[patchData[i].destLabel] - patchData[i].basePtr);
        break;
      case PatchType_Rel32:
        *((uint32 *)(patchData[i].patchPtr)) = (uint32)(labelPointers[patchData[i].destLabel] - patchData[i].basePtr);
        break;
      case PatchType_Rel64:
        //*((uint64 *)(patchData[i].patchPtr)) = (uint64)(labelPointers[patchData[i].destLabel] - patchData[i].basePtr);
        break;
      case PatchType_Abs32:
        *((uint32 *)(patchData[i].patchPtr)) = (uint32)(labelPointers[patchData[i].destLabel]);
        break;
      case PatchType_Abs64:
        //*((uint64 *)(patchData[i].patchPtr)) = (uint32)(labelPointers[patchData[i].destLabel]);
        break;
    }
  }
}