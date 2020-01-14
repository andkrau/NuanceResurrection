#include "basetypes.h"
#include "mpe.h"
#include "Handlers.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"

const uint32 decOnlyFlags = 0;
const uint32 addrFlags = 0;
const uint32 addrOnlyFlags = 0;
const uint32 mvrFlags = 0;
const uint32 mvrOnlyFlags = 0;
const uint32 rangeFlags = 0;
const uint32 rangeOnlyFlags = 0;
const uint32 moduloFlags = 0;
const uint32 moduloOnlyFlags = 0;

void MPE::DecodeInstruction_RCU16(uint8 *iPtr, InstructionCacheEntry *entry, uint32 *immExt)
{
  uint32 rangeDependency;
  uint8 decInfo;

  entry->packetInfo |= PACKETINFO_RCU;

  //copy dec control bits
  decInfo = *(iPtr + 1) & 0x03;

  entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_INFO)] = decInfo;

  switch(decInfo)
  {
    case 0:
      //shouldnt happen
      break;
    case 1:
      //RC1 only
      entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_DECRc1;
      entry->miscInputDependencies[SLOT_RCU] |= (DEPENDENCY_MASK_RC1 | DEPENDENCY_FLAG_C1Z);
      entry->miscOutputDependencies[SLOT_RCU] |= (DEPENDENCY_MASK_RC1 | DEPENDENCY_FLAG_C1Z);
      break;
    case 2:
      //RC0 only
      entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_DECRc0;
      entry->miscInputDependencies[SLOT_RCU] |= (DEPENDENCY_MASK_RC0 | DEPENDENCY_FLAG_C0Z);
      entry->miscOutputDependencies[SLOT_RCU] |= (DEPENDENCY_MASK_RC0 | DEPENDENCY_FLAG_C0Z);
      break;
    case 3:
      //both RC1 and RC0
      entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_DECBoth;
      entry->miscInputDependencies[SLOT_RCU] |= (DEPENDENCY_MASK_RC0 | DEPENDENCY_MASK_RC1 | DEPENDENCY_FLAG_C0Z | DEPENDENCY_FLAG_C1Z);
      entry->miscOutputDependencies[SLOT_RCU] |= (DEPENDENCY_MASK_RC0 | DEPENDENCY_MASK_RC1 | DEPENDENCY_FLAG_C0Z | DEPENDENCY_FLAG_C1Z);
      break;
  }

  if((*iPtr == 0xF9) && (*(iPtr + 1) & 0x04))
  {
    entry->packetInfo |= decOnlyFlags;
    //DEC RC0, DEC RC1 or both
    *immExt = 0;
    return;
  }

  //src is 0x03E0
  entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)] = ((*iPtr & 0x03) << 3) | (*(iPtr + 1) >> 5);
  //dest is 0x0018
  entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_DEST)] = (*(iPtr + 1) >> 3) & 0x03;

  //calculate remaining bits
  if((*iPtr & 0x7C) == 0x74)
  {
    if(*(iPtr + 1) & 0x04)
    {
      //addr #(n<<16), RI or addr #nnnn, RI
      if(*immExt)
      {
        entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)] = ((*immExt & ~0xFFFF) << 5) | (entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)] << 16) | (*immExt & 0xFFFF);
      }
      else
      {
        //convert five bit immediate to signed 16.16 fixed point
        entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)] = ((int32)(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)] << 27)) >> 11;
      }

      if(decInfo == 0)
      {
        entry->packetInfo |= addrOnlyFlags;
        entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_ADDRImmediateOnly;
      }
      else
      {
        entry->packetInfo |= addrFlags;
        entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_ADDRImmediate;
      }

      entry->miscInputDependencies[SLOT_RCU] |= INDEX_REG_DEPENDENCY_MASK(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_DEST)]);
      entry->miscOutputDependencies[SLOT_RCU] |= INDEX_REG_DEPENDENCY_MASK(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_DEST)]);
    }
    else
    {
      //addr Si, RI

      if(decInfo == 0)
      {
        entry->packetInfo |= addrOnlyFlags;
        entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_ADDRScalarOnly;
      }
      else
      {
        entry->packetInfo |= addrFlags;
        entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_ADDRScalar;
      }

      entry->scalarInputDependencies[SLOT_RCU] |= SCALAR_REG_DEPENDENCY_MASK(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)]);
      entry->miscInputDependencies[SLOT_RCU] |= INDEX_REG_DEPENDENCY_MASK(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_DEST)]);
      entry->miscOutputDependencies[SLOT_RCU] |= INDEX_REG_DEPENDENCY_MASK(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_DEST)]);
    }
  }
  else
  {
    if(*immExt)
    {
      //mvr #nnnn, RI
      entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)] = ((*immExt & ~0xFFFF) << 5) | (entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)] << 16) | (*immExt & 0xFFFF);

      if(decInfo == 0)
      {
        entry->packetInfo |= mvrOnlyFlags;
        entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_MVRImmediateOnly;
      }
      else
      {
        entry->packetInfo |= mvrFlags;
        entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_MVRImmediate;
      }

      entry->miscOutputDependencies[SLOT_RCU] |= INDEX_REG_DEPENDENCY_MASK(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_DEST)]);
    }
    else if((*(iPtr + 1) & 0x04) == 0)
    {
      //mvr Sj, RI
      if(decInfo == 0)
      {
        entry->packetInfo |= mvrOnlyFlags;
        entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_MVRScalarOnly;
      }
      else
      {
        entry->packetInfo |= mvrFlags;
        entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_MVRScalar;
      }

      entry->scalarInputDependencies[SLOT_RCU] |= SCALAR_REG_DEPENDENCY_MASK(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)]);
      entry->miscOutputDependencies[SLOT_RCU] |= INDEX_REG_DEPENDENCY_MASK(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_DEST)]);
    }
    else
    {
      //modulo RI or range RI
      entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)] = entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_DEST)];
      switch(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_DEST)])
      {
        case 0:
          rangeDependency = DEPENDENCY_MASK_XYRANGE;
          break;
        case 1:
          rangeDependency = DEPENDENCY_MASK_XYRANGE;
          break;
        case 2:
          rangeDependency = DEPENDENCY_MASK_UVRANGE;
          break;
        case 3:
          rangeDependency = DEPENDENCY_MASK_UVRANGE;
          break;
      }
      entry->miscInputDependencies[SLOT_RCU] |= (INDEX_REG_DEPENDENCY_MASK(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_SRC)]) | rangeDependency);
      
      if(*iPtr & 0x02)
      {
        //Range
        entry->miscOutputDependencies[SLOT_RCU] |= (DEPENDENCY_FLAG_MODMI | DEPENDENCY_FLAG_MODGE);

        if(decInfo == 0)
        {
          entry->packetInfo |= rangeOnlyFlags;
          entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_RangeOnly;
        }
        else
        {
          entry->packetInfo |= rangeFlags;
          entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_Range;
        }
      }
      else
      {
        //Modulo
        entry->miscOutputDependencies[SLOT_RCU] |= (DEPENDENCY_FLAG_MODMI | DEPENDENCY_FLAG_MODGE | INDEX_REG_DEPENDENCY_MASK(entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_DEST)]));
        if(decInfo == 0)
        {
          entry->packetInfo |= moduloOnlyFlags;
          entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_ModuloOnly;
        }
        else
        {
          entry->packetInfo |= moduloFlags;
          entry->nuances[FIXED_FIELD(SLOT_RCU,FIELD_RCU_HANDLER)] = Handler_Modulo;
        }
      }
    }
  }
}
