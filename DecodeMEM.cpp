#include "basetypes.h"
#include "mpe.h"
#include "Handlers.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "NuonMemoryMap.h"
#include "NuonEnvironment.h"

extern NuonEnvironment nuonEnv;

#define loadPixelBilinearXYFlags 0
#define loadPixelBilinearUVFlags 0
#define loadPixelLinearFlags 0
#define loadPixelAbsoluteFlags 0
#define storePixelBilinearXYFlags 0
#define storePixelBilinearUVFlags 0
#define storePixelLinearFlags 0
#define storePixelAbsoluteFlags 0
#define loadPixelZBilinearXYFlags 0
#define loadPixelZBilinearUVFlags 0
#define loadPixelZLinearFlags 0
#define loadPixelZAbsoluteFlags 0
#define storePixelZBilinearXYFlags 0
#define storePixelZBilinearUVFlags 0
#define storePixelZLinearFlags 0
#define storePixelZAbsoluteFlags 0
#define loadByteAbsoluteFlags 0
#define loadByteLinearFlags 0
#define loadByteBilinearFlags 0
#define loadWordBilinearFlags 0
#define loadWordAbsoluteFlags 0
#define loadWordLinearFlags 0
#define loadScalarAbsoluteFlags 0
#define loadScalarLinearFlags 0
#define loadScalarBilinearFlags 0
#define loadShortVectorAbsoluteFlags 0
#define loadShortVectorLinearFlags 0
#define loadShortVectorBilinearFlags 0
#define loadVectorAbsoluteFlags 0
#define loadVectorLinearFlags 0
#define loadVectorBilinearFlags 0
#define storeScalarAbsoluteFlags 0
#define storeScalarLinearFlags 0
#define storeScalarBilinearFlags 0
#define storeShortVectorAbsoluteFlags 0
#define storeShortVectorLinearFlags 0
#define storeShortVectorBilinearFlags 0
#define storeVectorAbsoluteFlags 0
#define storeVectorLinearFlags 0
#define storeVectorBilinearFlags 0
#define loadScalarControlRegisterFlags 0
#define loadVectorControlRegisterFlags 0
#define storeControlRegisterFlags 0
#define moveImmediateFlags 0
#define moveScalarFlags 0
#define moveVectorFlags 0
#define mirrorFlags 0
#define pushFlags 0
#define popFlags 0

void MPE::DecodeInstruction_MEM16(const uint8 * const iPtr, InstructionCacheEntry * const entry, const uint32 * const immExt)
{
  const uint32 field_3E0 = ((*iPtr & 0x03) << 3) | ((*(iPtr + 1) & 0xE0) >> 5);
  const uint32 field_1F = *(iPtr + 1) & 0x1F;
  bool bException = false;

  entry->packetInfo |= PACKETINFO_MEM;

  switch((*iPtr & 0x7C) >> 2)
  {
    case (0x48 >> 2):
      //ld_s (Si),Sk
      entry->packetInfo |= PACKETINFO_MEMORY_INDIRECT;
      entry->packetInfo |= loadScalarLinearFlags;
      if(nuonEnv.compilerOptions.bT3KCompilerHack)
      {
        entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
      }
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadScalarLinear;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_1F;

      entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_MEM_LOAD;
      break;
    case (0x4C >> 2):
      //st_s Sj,(Si)
      entry->packetInfo |= PACKETINFO_MEMORY_INDIRECT;
      entry->packetInfo |= storeScalarLinearFlags;
      if(nuonEnv.compilerOptions.bT3KCompilerHack)
      {
        entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
      }
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarLinear;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_3E0;

      entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E0) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_MEM] = DEPENDENCY_MASK_MEM_STORE;
      break;
    case (0x50 >> 2):
      //ld_s <label A> , Sk
      //address is a 5 bit offset in vectors from the base of the local control registers
      entry->packetInfo |= PACKETINFO_MEMORY_IO;
      entry->packetInfo |= loadScalarControlRegisterFlags;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadScalarControlRegisterAbsolute;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = (field_3E0 << 4) + MPE_CTRL_BASE;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_1F;
      entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscInputDependencies[SLOT_MEM] = GetControlRegisterInputDependencies(entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)],bException);

      switch(field_3E0)
      {
        case (0x40 >> 4):
          entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_FLAG_ALLFLAGS;
          break;
        case (0x80 >> 4):
          entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_RZ;
          break;
        case (0x1E0 >> 4):
          entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_RC0;
          break;
        case (0x1F0 >> 4):
          entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_RC1;
          break;
      }

      if(bException)
      {
        entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
      }
      break;
    case (0x54 >> 2):
      //st_s Sj, <label A>
      //address is a 5 bit offset in vectors from the base of the local control registers
      entry->packetInfo |= PACKETINFO_MEMORY_IO;
      entry->packetInfo |= storeControlRegisterFlags;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarControlRegisterAbsolute;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = (field_3E0 << 4) + MPE_CTRL_BASE;
      entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F);

      entry->miscOutputDependencies[SLOT_MEM] = GetControlRegisterOutputDependencies(entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)],bException);
      if(bException)
      {
        entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
      }
      break;
    case (0x58 >> 2):
      //mv_s Si, Sk
      entry->packetInfo |= moveScalarFlags;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_MV_SScalar;
      //src is 0x1F
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_1F;
      //dest is 0x3E0
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_3E0;
      entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      break;
    case (0x5C >> 2):
      //mv_s #n or mv_s #nnnn
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_MV_SImmediate;
      entry->packetInfo |= moveImmediateFlags;
      //src is 0x1F
      if(*immExt)
      {
        //#nnnn: no sign extend
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = ((*immExt & 0x7FFFFFF) << 5) | field_1F;
        //dest is 0x3E0
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_3E0;
      }
      else
      {
        //#n: sign extend
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = ((int32)(field_1F << 27)) >> 27;
        //dest is 0x3E0
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_3E0;
      }

      entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      break;
    case (0x60 >> 2):
      //mv_v Vi, Vk, or push/pop Vk
      if((*(iPtr + 1) & 0x60) == 0)
      {
        //mv_v Vi, Vk
        entry->packetInfo |= moveVectorFlags;
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_MV_V;
        //src is 0x1C
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_1F & 0x1CUL;
        //dest is 0x380
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_3E0 & 0x1CUL;

        entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F & 0x1CUL);
        entry->scalarOutputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_3E0 & 0x1CUL);
      }
      else
      {
        if(nuonEnv.compilerOptions.bT3KCompilerHack)
        {
          entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
        }

        if(*(iPtr + 1) & 0x20)
        {
          //push
          entry->packetInfo |= pushFlags;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_PushVector + (*iPtr & 0x03);
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_1F;
          if((*iPtr & 0x02) == 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] &= 0x1CUL;
          }

          switch(*iPtr & 0x03)
          {
            case 0:
              //push Vk
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F & 0x1CUL);
              break;
            case 1:
              //push Vk, rz
              entry->scalarInputDependencies[SLOT_MEM] = SHORT_VECTOR_REG_DEPENDENCY_MASK(field_1F & 0x1CUL);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_RZ;
              break;
            case 2:
              //push Sk, cc, rzi1, rz
              entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_RZ | DEPENDENCY_MASK_RZI1 | DEPENDENCY_FLAG_ALLFLAGS;
              break;
            case 3:
              //push Sk, cc, rzi2, rz
              entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_RZ | DEPENDENCY_MASK_RZI2 | DEPENDENCY_FLAG_ALLFLAGS;
              break;
          }
        }
        else
        {
          //pop
          entry->packetInfo |= popFlags;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_PopVector + (*iPtr & 0x03);
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_1F;
          if((*iPtr & 0x02) == 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] &= 0x1CUL;
          }

          switch(*iPtr & 0x03)
          {
            case 0:
              //pop Vk
              entry->scalarOutputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F & 0x1CUL);
              break;
            case 1:
              //pop Vk, rz
              entry->scalarOutputDependencies[SLOT_MEM] = SHORT_VECTOR_REG_DEPENDENCY_MASK(field_1F & 0x1CUL);
              entry->miscOutputDependencies[SLOT_MEM] = DEPENDENCY_MASK_RZ;
              break;
            case 2:
              //pop Sk, cc, rzi1, rz
              entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
              entry->miscOutputDependencies[SLOT_MEM] = DEPENDENCY_MASK_RZ | DEPENDENCY_MASK_RZI1 | DEPENDENCY_FLAG_ALLFLAGS;
              break;
            case 3:
              //pop Sk, cc, rzi2, rz
              entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
              entry->miscOutputDependencies[SLOT_MEM] = DEPENDENCY_MASK_RZ | DEPENDENCY_MASK_RZI2 | DEPENDENCY_FLAG_ALLFLAGS;
              break;
          }
        }
      }
      break;
  }
}

void MPE::DecodeInstruction_MEM32(const uint8 * const iPtr, InstructionCacheEntry * const entry, const uint32 * const immExt)
{
  const uint32 field_3E00000 = ((*iPtr & 0x03) << 3) | ((*(iPtr + 1) & 0xE0) >> 5);
  const uint32 field_1F0000 = *(iPtr + 1) & 0x1F;
  const uint32 field_1E0 = ((*(iPtr + 2) & 0x01) << 3) | ((*(iPtr + 3) & 0xE0) >> 5);
  const uint32 field_3F = *(iPtr + 3) & 0x3F;
  bool bException = false;

  entry->packetInfo |= PACKETINFO_MEM;

  switch(*(iPtr + 2) & 0x0F)
  {
    case 0x0:
    {
      //ld_b <label>, Sk
      entry->packetInfo |= loadByteAbsoluteFlags;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadByteAbsolute;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = (field_3F << 5) | field_3E00000;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_1F0000;
      entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);

      switch(*(iPtr + 3) >> 6)
      {
        case 0:
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTROM_BASE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
          return;
        case 1:
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTRAM_BASE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
          return;
      }
      break;
    }
    case 0x1:
    {
      //ld_w <label>, Sk
      entry->packetInfo |= loadWordAbsoluteFlags;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadWordAbsolute;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = ((field_3F << 5) | field_3E00000) << 1;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_1F0000;
      entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);

      switch(*(iPtr + 3) >> 6)
      {
        case 0:
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTROM_BASE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
          return;
        case 1:
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTRAM_BASE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
          return;
      }
      break;
    }
    case 0x2:
    {
      //ld_s <label B>, Sk
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = ((field_3F << 5) | field_3E00000) << 2;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_1F0000;
      entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);

      switch(*(iPtr + 3) >> 6)
      {
        case 0:
          entry->packetInfo |= loadScalarAbsoluteFlags;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadScalarAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTROM_BASE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
          return;
        case 1:
          entry->packetInfo |= loadScalarAbsoluteFlags;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadScalarAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTRAM_BASE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
          return;
        case 2:
          entry->packetInfo |= loadScalarControlRegisterFlags;
          entry->packetInfo |= PACKETINFO_MEMORY_IO;
          entry->miscInputDependencies[SLOT_MEM] = GetControlRegisterInputDependencies(entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)], bException);
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadScalarControlRegisterAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_CTRL_BASE;
          if(bException)
          {
            entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
          }
          return;
      }
      break;
    }
    case 0x3:
    {
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = (field_3F << 5) | field_3E00000;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_1F0000 & 0x1C;

      switch(*(iPtr + 1) & 0x03)
      {
        case 0:
        {
          //ld_sv <label>, Vk
          entry->packetInfo |= loadShortVectorAbsoluteFlags;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadShortVectorAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] <<= 3;
          entry->scalarOutputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000 & 0x1C);

          switch(*(iPtr + 3) >> 6)
          {
            case 0:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTROM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
              return;
            case 1:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTRAM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
              return;
          }
          break;
        }
        case 1:
        {
          //ld_v <label>, Vk
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] <<= 4;
          entry->scalarOutputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000 & 0x1C);

          switch(*(iPtr + 3) >> 6)
          {
            case 0:
              entry->packetInfo |= loadVectorAbsoluteFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadVectorAbsolute;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTROM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
              return;
            case 1:
              entry->packetInfo |= loadVectorAbsoluteFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadVectorAbsolute;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTRAM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
              return;
            case 2:
              entry->packetInfo |= loadVectorControlRegisterFlags;
              entry->packetInfo |= PACKETINFO_MEMORY_IO;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadVectorControlRegisterAbsolute;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_CTRL_BASE;
              return;
          }
          break;
        }
        case 2:
        {
          //ld_p <label>, Vk
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadPixelAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_INFO)] = MEM_INFO_LINEAR_ABSOLUTE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] <<= 1;
          entry->scalarOutputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000 & 0x1C);
          entry->packetInfo |= loadPixelAbsoluteFlags;

          switch(*(iPtr + 3) >> 6)
          {
            case 0:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTROM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
              return;
            case 1:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTRAM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
              return;
          }
          break;
        }
        case 3:
        {
          //ld_pz <label>, Vk
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadPixelZAbsolute;          
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_INFO)] = MEM_INFO_LINEAR_ABSOLUTE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] <<= 1;
          entry->scalarOutputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->packetInfo |= loadPixelZAbsoluteFlags;

          switch(*(iPtr + 3) >> 6)
          {
            case 0:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTROM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
              return;
            case 1:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] += MPE_DTRAM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] & MPE_VALID_MEMORY_MASK];
              return;
          }
          break;
        }
      }
    }
    break;
    case 0x4:
    {
      //load
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_3E00000;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_1F0000;

      switch(*(iPtr + 3) >> 4)
      {
        case 0x00:
        {
          switch(*(iPtr + 3) & 0x0F)
          {
            case 0x00:
              //ld_b (Si),Sk
              entry->packetInfo |= loadByteLinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadByteLinear;
              entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
              entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
              break;
            case 0x01:
              break;
            case 0x02:
              //ld_b (uv),Sk
              entry->packetInfo |= loadByteBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadByteBilinearUV;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
              break;
            case 0x03:
              //ld_b (xy),Sk
              entry->packetInfo |= loadByteBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadByteBilinearXY;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
              break;
            case 0x04:
              //ld_w (Si),Sk
              entry->packetInfo |= loadWordLinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadWordLinear;
              entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
              entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
              break;
            case 0x05:
              break;
            case 0x06:
              //ld_w (uv),Sk
              entry->packetInfo |= loadWordBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadWordBilinearUV;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
              break;
            case 0x07:
              //ld_w (xy),Sk
              entry->packetInfo |= loadWordBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadWordBilinearXY;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
              break;
            case 0x08:
              break;
            case 0x09:
              break;
            case 0x0A:
              //ld_s (uv),Sk
              entry->packetInfo |= loadScalarBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadScalarBilinearUV;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
              break;
            case 0x0B:
              //ld_s (xy),Sk
              entry->packetInfo |= loadScalarBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadScalarBilinearXY;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
              //entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
              break;
            case 0x0C:
              //ld_sv (Si),Vk
              entry->packetInfo |= loadShortVectorLinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadShortVectorLinear;
              entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
              entry->scalarOutputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
              break;
            case 0x0D:
              break;
            case 0x0E:
              //ld_sv (uv),Vk
              entry->packetInfo |= loadShortVectorBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadShortVectorBilinearUV;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              entry->scalarOutputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000 & 0x1C);
              break;
            case 0x0F:
              //ld_sv (xy),Vk
              entry->packetInfo |= loadShortVectorBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadShortVectorBilinearXY;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              entry->scalarOutputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000 & 0x1C);
              break;
          }
          break;
        }
        case 0x01:
        {
          entry->scalarOutputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
          switch(*(iPtr + 3) & 0x0F)
          {
            case 0x00:
              //ld_v (Si), Vk
              entry->packetInfo |= loadVectorLinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadVectorLinear;
              entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
              entry->miscInputDependencies[SLOT_MEM] = 0xFFFFFFFF;
              break;
            case 0x01:
            case 0x02:
              //ld_v (uv), Vk
              entry->packetInfo |= loadVectorBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadVectorBilinearUV;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              break;
            case 0x03:
              //ld_v (xy), Vk
              entry->packetInfo |= loadVectorBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadVectorBilinearXY;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              break;
            case 0x04:
              //ld_p (Si), Vk
              entry->packetInfo |= loadPixelLinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadPixelLinear;
              entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
              entry->miscInputDependencies[SLOT_MEM] = 0xFFFFFFFF;
              break;
            case 0x05:
              break;
            case 0x06:
              //ld_p (uv), Vk
              entry->packetInfo |= loadPixelBilinearUVFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadPixelBilinearUV;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              break;
            case 0x07:
              //ld_p (xy), Vk
              entry->packetInfo |= loadPixelBilinearXYFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadPixelBilinearXY;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              break;
            case 0x08:
              //ld_pz (Si), Vk
              entry->packetInfo |= loadPixelZLinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadPixelZLinear;
              entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
              entry->miscInputDependencies[SLOT_MEM] = 0xFFFFFFFF;
              break;
            case 0x09:
              break;
            case 0x0A:
              //ld_pz (uv), Vk
              entry->packetInfo |= loadPixelZBilinearUVFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadPixelZBilinearUV;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              break;
            case 0x0B:
              //ld_pz (xy), Vk
              entry->packetInfo |= loadPixelZBilinearXYFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_LoadPixelZBilinearXY;
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              break;
            case 0x0C:
              break;
            case 0x0D:
              break;
            case 0x0E:
              break;
            case 0x0F:
              break;
          }
          break;
        }
      }
      break;
    }
    case 0x5:
    {
      //mirror Sj, Sk
      entry->packetInfo |= mirrorFlags;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_Mirror;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_1F0000;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_3E00000;
      entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
      entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
      break;
    }
    case 0x6:
    {
      //mv_s #nnn, Sk
      entry->packetInfo |= moveImmediateFlags;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_MV_SImmediate;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = (((int32)((int8)((*(iPtr + 3) & 0x7F) << 1))) << 4) | (*(iPtr + 1) & 0x1F);
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_3E00000;
      entry->scalarOutputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
      break;
    }
    case 0x7:
    case 0x8:
    case 0x9:
    case 0xA:
    {
      //st_s Sj, <label B>
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = ((field_3F << 5) | field_3E00000) << 2;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_1F0000;
      entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
      switch(*(iPtr + 3) >> 6)
      {
        case 0:
          entry->packetInfo |= storeScalarAbsoluteFlags;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTROM_BASE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
          return;
        case 1:
          entry->packetInfo |= storeScalarAbsoluteFlags;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTRAM_BASE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
          return;
        case 2:
          entry->packetInfo |= PACKETINFO_MEMORY_IO;
          entry->packetInfo |= storeControlRegisterFlags;
          entry->miscOutputDependencies[SLOT_MEM] = GetControlRegisterOutputDependencies(entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)], bException);
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarControlRegisterAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_CTRL_BASE;
          if(bException)
          {
            entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
          }
          return;
      }
      break;
    }
    case 0xB:
    {
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = ((field_3F << 5) | field_3E00000);
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_1F0000 & 0x1C;

      switch(*(iPtr + 1) & 0x03)
      {
        case 0:
        {
          //st_sv Vj, <label>
          entry->packetInfo |= storeShortVectorAbsoluteFlags;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreShortVectorAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] <<= 3;
          entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000 & 0x1C);
          switch(*(iPtr + 3) >> 6)
          {
            case 0:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTROM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
              return;
            case 1:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTRAM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
              return;
          }
          break;
        }
        case 1:
        {
          //st_v Vj, <label>
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreVectorAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] <<= 4;
          entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000 & 0x1C);
          switch(*(iPtr + 3) >> 6)
          {
            case 0:
              entry->packetInfo |= storeVectorAbsoluteFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTROM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
              return;
            case 1:
              entry->packetInfo |= storeVectorAbsoluteFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTRAM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
              return;
            case 2:
              entry->packetInfo |= PACKETINFO_MEMORY_IO;
              entry->packetInfo |= storeControlRegisterFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreVectorControlRegisterAbsolute;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_CTRL_BASE;
              //the only vector control registers are commxmit and commrecv, both of which can cause exceptions, thus never compile
              entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
              return;
          }
          break;
        }
        case 2:
        {
          //st_p Vj, <label>
          entry->packetInfo |= storePixelAbsoluteFlags;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StorePixelAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_INFO)] = MEM_INFO_LINEAR_ABSOLUTE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] <<= 1;
          entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000 & 0x1C);
          switch(*(iPtr + 3) >> 6)
          {
            case 0:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTROM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
              return;
            case 1:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTRAM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
              return;
          }
          break;
        }
        case 3:
        {
          //st_pz Vj, <label>
          entry->packetInfo |= storePixelZAbsoluteFlags;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StorePixelZAbsolute;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_INFO)] = MEM_INFO_LINEAR_ABSOLUTE;
          entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] <<= 1;
          entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000 & 0x1C);
          //entry->packetInfo |= PACKETINFO_NEVERCOMPILE;

          switch(*(iPtr + 3) >> 6)
          {
            case 0:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTROM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
              return;
            case 1:
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTRAM_BASE;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
              return;
          }
          break;
        }
      }
    }
    break;
    case 0xC:
      //store
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = field_1F0000;
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = field_3E00000;
      switch(*(iPtr + 3) >> 4)
      {
        case 0x00:
          switch(*(iPtr + 3) & 0x0F)
          {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x09:
              break;
            case 0x0A:
              //st_s Sj, (uv)
              entry->packetInfo |= storeScalarBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarBilinearUV;
              entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              //entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
              break;
            case 0x0B:
              //st_s Sj, (xy)
              entry->packetInfo |= storeScalarBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarBilinearXY;
              entry->scalarInputDependencies[SLOT_MEM] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              //entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
              break;
            case 0x0C:
              //st_sv Vj, (Si)
              entry->packetInfo |= storeShortVectorLinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreShortVectorLinear;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
              break;
            case 0x0D:
              break;
            case 0x0E:
              //st_sv Vj, (uv)
              entry->packetInfo |= storeShortVectorBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreShortVectorBilinearUV;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              break;
            case 0x0F:
              //st_sv Vj, (xy)
              entry->packetInfo |= storeShortVectorBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreShortVectorBilinearXY;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              break;
          }
          break;
        case 0x01:
          switch(*(iPtr + 3) & 0x0F)
          {
            case 0x00:
              //st_v Vj, (Sk)
              entry->packetInfo |= storeVectorLinearFlags;
              entry->packetInfo |= PACKETINFO_MEMORY_INDIRECT;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreVectorLinear;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
              break;
            case 0x01:
            case 0x02:
              //st_v Vj, (uv)
              entry->packetInfo |= storeVectorBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreVectorBilinearUV;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              //entry->packetInfo |= PACKETINFO_NEVERCOMPILE; //!! why was this needed, all games seem to work/look okay
              break;
            case 0x03:
              //st_v Vj, (xy)
              entry->packetInfo |= storeVectorBilinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreVectorBilinearXY;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              //entry->packetInfo |= PACKETINFO_NEVERCOMPILE; //!! why was this needed, all games seem to work/look okay
              break;
            case 0x04:
              //st_p Vj, (Si)
              entry->packetInfo |= storePixelLinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StorePixelLinear;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
              break;
            case 0x05:
              break;
            case 0x06:
              //st_p Vj, (uv)
              entry->packetInfo |= storePixelBilinearUVFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StorePixelBilinearUV;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              break;
            case 0x07:
              //st_p Vj, (xy)
              entry->packetInfo |= storePixelBilinearXYFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StorePixelBilinearXY;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              break;
            case 0x08:
              //st_pz Vj, (Si)
              entry->packetInfo |= storePixelZLinearFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StorePixelZLinear;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
              break;
            case 0x09:
              break;
            case 0x0A:
              //st_pz Vj, (uv)
              entry->packetInfo |= storePixelZBilinearUVFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StorePixelZBilinearUV;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_RU | DEPENDENCY_MASK_RV;
              break;
            case 0x0B:
              //st_pz Vj, (xy)
              entry->packetInfo |= storePixelZBilinearXYFlags;
              entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StorePixelZBilinearXY;
              entry->scalarInputDependencies[SLOT_MEM] = VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
              entry->miscInputDependencies[SLOT_MEM] = DEPENDENCY_MASK_XYCTL | DEPENDENCY_MASK_RX | DEPENDENCY_MASK_RY;
              break;
            case 0x0C:
              break;
            case 0x0D:
              break;
            case 0x0E:
              break;
            case 0x0F:
              break;
          }
          break;
      }
      break;
    case 0xD:
      break;
    case 0xE:
      //st_s #nn, <label C> (fall through)
    case 0xF:
    {
      //st_s #nn, <label C>
      entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_FROM)] = ((*immExt & 0x7FFFFE0UL) << 5) | ((field_3F & 0x1F) << 5) | field_1F0000;
      if(*immExt == 0)
      {
        entry->packetInfo |= PACKETINFO_MEMORY_IO;
        entry->packetInfo |= storeControlRegisterFlags;
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarControlRegisterImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = (((field_1E0 << 5) | field_3E00000) << 4);
        entry->miscOutputDependencies[SLOT_MEM] = GetControlRegisterOutputDependencies(entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)], bException);
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_CTRL_BASE;
        if(bException)
        {
          entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
        }
        return;
      }
      else
      {
        //st_s #nnnn, <label D>
        entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] = (((*immExt & 0x04UL) << 9) | (field_1E0 << 7) | (field_3E00000 << 2) | (*immExt & 0x03UL)) << 2;
        switch((*immExt & 0x18UL) >> 3)
        {
          case 0:
            entry->packetInfo |= storeControlRegisterFlags;
            entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarImmediate;
            entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTROM_BASE;
            entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
            return;
          case 1:
            entry->packetInfo |= storeControlRegisterFlags;
            entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarImmediate;
            entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_DTRAM_BASE;
            entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_POINTER)] = (size_t)&dtrom[entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] & MPE_VALID_MEMORY_MASK];
            return;
          case 2:
            entry->packetInfo |= PACKETINFO_MEMORY_IO;
            entry->packetInfo |= storeControlRegisterFlags;
            entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_HANDLER)] = Handler_StoreScalarControlRegisterImmediate;
            entry->miscOutputDependencies[SLOT_MEM] = GetControlRegisterOutputDependencies(entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)], bException);
            entry->nuances[FIXED_FIELD(SLOT_MEM,FIELD_MEM_TO)] += MPE_CTRL_BASE;
            if(bException)
            {
              entry->packetInfo |= PACKETINFO_NEVERCOMPILE;
            }
            return;
        }
      }
      break;
    }
  }
}

uint32 MPE::GetControlRegisterInputDependencies(const uint32 address, bool &bException) const
{
  bException = false;

  switch((address & 0x1FF0) >> 4)
  {
    case (0x00 >> 4):
      //mpectl
    case (0x10 >> 4):
      //excepsrc
      bException = true;
      return 0;
    case (0x20 >> 4):
      //excepclr
      return 0;
    case (0x30 >> 4):
      //excephalten
      bException = true;
      return 0;
    case (0x40 >> 4):
      //cc
      return DEPENDENCY_FLAG_ALLFLAGS;
    case (0x50 >> 4):
      //pcfetch
      bException = true;
      return 0;
    case (0x60 >> 4):
      //pcroute
      bException = true;
      return 0;
    case (0x70 >> 4):
      //pcexec
      bException = true;
      return 0;
    case (0x80 >> 4):
      //rz
      return DEPENDENCY_MASK_RZ;
    case (0x90 >> 4):
      //rzi1
      return DEPENDENCY_MASK_RZI1;
    case (0xA0 >> 4):
      //rzi2
      return DEPENDENCY_MASK_RZI2;
    case (0xB0 >> 4):
      //intvec1
      return 0;
    case (0xC0 >> 4):
      //intvec2
      return 0;
    case (0xD0 >> 4):
      //intsrc
      bException = true;
      return 0;
    case (0xE0 >> 4):
      //intclr
    case (0xF0 >> 4):
      //intctl
    case (0x100 >> 4):
      //inten1
    case (0x110 >> 4):
      //inten1set
    case (0x130 >> 4):
      //inten2sel
      bException = true;
      return 0;
    case (0x1E0 >> 4):
      //rc0
      return DEPENDENCY_MASK_RC0;
    case (0x1F0 >> 4):
      //rc1
      return DEPENDENCY_MASK_RC1;
    case (0x200 >> 4):
      //rx
      return DEPENDENCY_MASK_RX;
    case (0x210 >> 4):
      //ry
      return DEPENDENCY_MASK_RY;
    case (0x220 >> 4):
      //xyrange
      return DEPENDENCY_MASK_XYRANGE;
    case (0x240 >> 4):
      //xyctl
      return DEPENDENCY_MASK_XYCTL;
    case (0x250 >> 4):
      //ru
      return DEPENDENCY_MASK_RU;
    case (0x260 >> 4):
      //rv
      return DEPENDENCY_MASK_RV;
    case (0x270 >> 4):
      //uvrange
      return DEPENDENCY_MASK_UVRANGE;
    case (0x290 >> 4):
      //uvctl
      return DEPENDENCY_MASK_UVCTL;
    case (0x2C0 >> 4):
      //svshift
      return DEPENDENCY_MASK_SVSHIFT;  
    case (0x2D0 >> 4):
      //acshift
      return DEPENDENCY_MASK_ACSHIFT;
    case (0x500 >> 4):
      //odmactl
    case (0x510 >> 4):
      //odmacptr
    case (0x600 >> 4):
      //mdmactl
    case (0x610 >> 4):
      //mdmacptr
    case (0x7E0 >> 4):
      //comminfo
    case (0x7F0 >> 4):
      //commctl
    case (0x800 >> 4):
      //commxmit0 to commxmit3
    case (0x810 >> 4):
      //commrecv0 to commrecv3
      bException = true;
      return 0;
    default:
      bException = true;
      return 0;
  }
}

uint32 MPE::GetControlRegisterOutputDependencies(const uint32 address, bool &bException) const
{
  bException = false;

  switch((address & 0x1FF0) >> 4)
  {
    case (0x00 >> 4):
      //mpectl
    case (0x10 >> 4):
      //excepsrc
      bException = true;
      return 0;
    case (0x20 >> 4):
      //excepclr
      return 0;
    case (0x30 >> 4):
      //excephalten
      bException = true;
      return 0;
    case (0x40 >> 4):
      //cc
      return DEPENDENCY_FLAG_ALLFLAGS;
    case (0x50 >> 4):
      //pcfetch
      bException = true;
      return 0;
    case (0x60 >> 4):
      //pcroute
      bException = true;
      return 0;
    case (0x70 >> 4):
      //pcexec
      bException = true;
      return 0;
    case (0x80 >> 4):
      //rz
      return DEPENDENCY_MASK_RZ;
    case (0x90 >> 4):
      //rzi1
      return DEPENDENCY_MASK_RZI1;
    case (0xA0 >> 4):
      //rzi2
      return DEPENDENCY_MASK_RZI2;
    case (0xB0 >> 4):
      //intvec1
      return 0;
    case (0xC0 >> 4):
      //intvec2
      return 0;
    case (0xD0 >> 4):
      //intsrc
      bException = true;
      return 0;
    case (0xE0 >> 4):
      //intclr
      return 0;
    case (0xF0 >> 4):
      //intctl
    case (0x100 >> 4):
      //inten1
    case (0x110 >> 4):
      //inten1set
    case (0x130 >> 4):
      //inten2sel
      bException = true;
      return 0;
    case (0x1E0 >> 4):
      //rc0
      return DEPENDENCY_MASK_RC0 | DEPENDENCY_FLAG_C0Z;
    case (0x1F0 >> 4):
      //rc1
      return DEPENDENCY_MASK_RC1 | DEPENDENCY_FLAG_C1Z;
    case (0x200 >> 4):
      //rx
      return DEPENDENCY_MASK_RX;
    case (0x210 >> 4):
      //ry
      return DEPENDENCY_MASK_RY;
    case (0x220 >> 4):
      //xyrange
      return DEPENDENCY_MASK_XYRANGE;
    case (0x240 >> 4):
      //xyctl
      return DEPENDENCY_MASK_XYCTL;
    case (0x250 >> 4):
      //ru
      return DEPENDENCY_MASK_RU;
    case (0x260 >> 4):
      //rv
      return DEPENDENCY_MASK_RV;
    case (0x270 >> 4):
      //uvrange
      return DEPENDENCY_MASK_UVRANGE;
    case (0x290 >> 4):
      //uvctl
      return DEPENDENCY_MASK_UVCTL;
    case (0x2C0 >> 4):
      //svshift
      return DEPENDENCY_MASK_SVSHIFT;  
    case (0x2D0 >> 4):
      //acshift
      return DEPENDENCY_MASK_ACSHIFT;
    case (0x500 >> 4):
      //odmactl
    case (0x510 >> 4):
      //odmacptr
    case (0x600 >> 4):
      //mdmactl
    case (0x610 >> 4):
      //mdmacptr
    case (0x7E0 >> 4):
      //comminfo
    case (0x7F0 >> 4):
      //commctl
    case (0x800 >> 4):
      //commxmit0 to commxmit3
    case (0x810 >> 4):
      //commrecv0 to commrecv3
      bException = true;
      return 0;
    case (0xFF0 >> 4):
      //configa: writing to this register acts as a syscall
      bException = true;
      return 0;
    default:
      bException = true;
      return 0;
  }
}
