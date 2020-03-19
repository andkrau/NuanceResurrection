#include "basetypes.h"
#include "PatchManager.h"

void PatchManager::ApplyPatches()
{
  for(uint32 i = 0; i < numPatches; i++)
  {
    switch(patchData[i].patchType)
    {
      case PatchType::PatchType_Rel8:
        *((uint8 *)(patchData[i].patchPtr)) = (uint8)(labelPointers[patchData[i].destLabel] - patchData[i].basePtr);
        break;
      case PatchType::PatchType_Rel16:
        *((uint16 *)(patchData[i].patchPtr)) = (uint16)(labelPointers[patchData[i].destLabel] - patchData[i].basePtr);
        break;
      case PatchType::PatchType_Rel32:
        *((uint32 *)(patchData[i].patchPtr)) = (uint32)(labelPointers[patchData[i].destLabel] - patchData[i].basePtr);
        break;
      case PatchType::PatchType_Rel64:
        //*((uint64 *)(patchData[i].patchPtr)) = (uint64)(labelPointers[patchData[i].destLabel] - patchData[i].basePtr);
        break;
      case PatchType::PatchType_Abs32:
        *((uint32 *)(patchData[i].patchPtr)) = (uint32)(labelPointers[patchData[i].destLabel]); //!! 64bit prob?
        break;
      case PatchType::PatchType_Abs64:
        //*((uint64 *)(patchData[i].patchPtr)) = (uint32)(labelPointers[patchData[i].destLabel]);
        break;
    }
  }
}
