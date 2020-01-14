#include "basetypes.h"
#include "mpe.h"
#include "Handlers.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"

const uint32 addmFlags = 0;
const uint32 submFlags = 0;
const uint32 dotpFlags = 0;
const uint32 mulFlags = 0;
const uint32 mulsvFlags = 0;
const uint32 mulpFlags = 0;

void MPE::DecodeInstruction_MUL16(uint8 *iPtr, InstructionCacheEntry *entry, uint32 *immExt)
{
  uint32 field_3E0 = ((*iPtr & 0x03) << 3) | (*(iPtr + 1) >> 5);
  uint32 field_1F = *(iPtr + 1) & 0x1F;

  entry->packetInfo |= PACKETINFO_MUL;
  
  if(*iPtr & 0x04)
  {
    //mul_sv Vi, Vk, >>svshift, Vk
    entry->packetInfo |= mulsvFlags;
    entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_SVVectorShiftSvshift;
    entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_1F & 0x1C;
    entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = (field_3E0 & 0x1C);
    entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = (field_3E0 & 0x1C);
    entry->scalarInputDependencies[SLOT_MUL] = VECTOR_REG_DEPENDENCY_MASK(field_3E0 & 0x1C) | VECTOR_REG_DEPENDENCY_MASK(field_1F & 0x1C);
    entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_SVSHIFT;
    entry->scalarOutputDependencies[SLOT_MUL] = VECTOR_REG_DEPENDENCY_MASK(field_3E0 & 0x1C);
  }
  else
  {
    //mul Si, Sk, >>acshift, Sk
    entry->packetInfo |= mulFlags;
    entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MULScalarShiftAcshift;
    entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_1F;
    entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E0;
    entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_3E0;
    entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E0) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
    entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_ACSHIFT;
    entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
    entry->miscOutputDependencies[SLOT_MUL] = DEPENDENCY_FLAG_MV;
  }
}

void MPE::DecodeInstruction_MUL32(uint8 *iPtr, InstructionCacheEntry *entry, uint32 *immExt)
{
  uint32 field_3E00000 = ((*iPtr & 0x03) << 3) | ((*(iPtr + 1) & 0xE0) >> 5);
  uint32 field_1F0000 = *(iPtr + 1) & 0x1F;
  uint32 field_7F = *(iPtr + 3) & 0x7F;
  uint32 field_60 = (*(iPtr + 3) & 0x60) >> 5;
  uint32 field_1F = *(iPtr + 3) & 0x1F;

  entry->packetInfo |= PACKETINFO_MUL;

  switch(*(iPtr + 2) & 0x0F)
  {
    case 0x00:
      //mul Si, Sj, >>acshift, Sk or mul Si, Sk, >>#m, Sk
      entry->packetInfo |= mulFlags;
      if(*(iPtr + 3) & 0x80)
      {
        //mul Si, Sk, >>#m, Sk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_1F0000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_3E00000;

        if((field_7F & 0x40) == 0)
        {
          entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MULScalarShiftRightImmediate;
          entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_7F;
        }
        else
        {
          entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MULScalarShiftLeftImmediate;
          entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = 128 - field_7F;
        }

        entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
        entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
      }
      else
      {
        //mul Si, Sj, >>acshift, Sk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MULScalarShiftAcshift;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_1F0000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F;
        entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_ACSHIFT;
        entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      }
      entry->miscOutputDependencies[SLOT_MUL] = DEPENDENCY_FLAG_MV;
      return;
    case 0x01:
      //mul Si, Sk, >>Sq, Sk or mul #n, Sj, >>acshift, Sk
      entry->packetInfo |= mulFlags;
      if(*(iPtr + 3) & 0x80)
      {
        //mul #n, Sj, >>acshift, Sk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MULImmediateShiftAcshift;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_1F0000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F;
        entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_ACSHIFT;
        entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      }
      else
      {
        //mul Si, Sk, >>Sq, Sk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MULScalarShiftScalar;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_1F0000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_3E00000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_1F;
        entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
        entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
      }
      entry->miscOutputDependencies[SLOT_MUL] = DEPENDENCY_FLAG_MV;
      return;
    case 0x02:
      //mul #n, Sk, >>#m, Sk or mul #n, Sk, >>Sq, Sk
      entry->packetInfo |= mulFlags;
      if(*(iPtr + 3) & 0x80)
      {
        //mul #n, Sk, >>Sq, Sk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MULImmediateShiftScalar;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_1F0000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_3E00000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_1F;
        entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
        entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
      }
      else
      {
        //mul #n, Sk, >>#m, Sk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_1F0000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_3E00000;
        if((field_7F & 0x40) == 0)
        {
          entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MULImmediateShiftRightImmediate;
          entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_7F;
        }
        else
        {
          entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MULImmediateShiftLeftImmediate;
          entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = 128 - field_7F;
        }
        entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
        entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
      }
      entry->miscOutputDependencies[SLOT_MUL] = DEPENDENCY_FLAG_MV;
      return;
    case 0x03:
      //no instructions
      break;
    case 0x04:
      entry->packetInfo |= mulsvFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_3E00000;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_1F0000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F & 0x1CUL;
      entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
      entry->scalarOutputDependencies[SLOT_MUL] = VECTOR_REG_DEPENDENCY_MASK(field_1F);

      if(*(iPtr + 3) & 0x80)
      {
        //mul_sv Si, Vj, >>m, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_SVScalarShiftImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_60;
      }
      else
      {
        //mul_sv Si, Vj, >>svshift, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_SVScalarShiftSvshift;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_SVSHIFT;
      }
      return;
    case 0x05:
      entry->packetInfo |= mulsvFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F & 0x1CUL;
      entry->scalarInputDependencies[SLOT_MUL] = VECTOR_REG_DEPENDENCY_MASK(field_3E00000);
      entry->scalarOutputDependencies[SLOT_MUL] = VECTOR_REG_DEPENDENCY_MASK(field_1F);

      if(*(iPtr + 3) & 0x80)
      {
        //mul_sv ru, Vj, >>m, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_SVRuShiftImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_60;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_RU | DEPENDENCY_MASK_UVCTL;
      }
      else
      {
        //mul_sv ru, Vj, >>svshift, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_SVRuShiftSvshift;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_RU | DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_SVSHIFT;
      }
      return;
    case 0x06:
      entry->packetInfo |= mulsvFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F & 0x1CUL;
      entry->scalarInputDependencies[SLOT_MUL] = VECTOR_REG_DEPENDENCY_MASK(field_3E00000);
      entry->scalarOutputDependencies[SLOT_MUL] = VECTOR_REG_DEPENDENCY_MASK(field_1F);

      if(*(iPtr + 3) & 0x80)
      {
        //mul_sv rv, Vj, >>#m, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_SVRvShiftImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_60;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_RV | DEPENDENCY_MASK_UVCTL;
      }
      else
      {
        //mul_sv rv, Vj, >>svshift, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_SVRvShiftSvshift;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_RV | DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_SVSHIFT;
      }
      return;
    case 0x07:
      entry->packetInfo |= mulsvFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_1F0000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F & 0x1CUL;
      entry->scalarInputDependencies[SLOT_MUL] = VECTOR_REG_DEPENDENCY_MASK(field_3E00000) | VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
      entry->scalarOutputDependencies[SLOT_MUL] = VECTOR_REG_DEPENDENCY_MASK(field_1F);

      if(*(iPtr + 3) & 0x80)
      {
        //mul_sv Vi, Vj, >>m, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_SVVectorShiftImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_60;
      }
      else
      {
        //mul_sv Vi, Vj, >>svshift, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_SVVectorShiftSvshift;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_SVSHIFT;
      }
      return;
    case 0x08:
      //mul_p Si, Vj, >>svshift, Vk or mul_p Si, Vj, >>m, Vk
      entry->packetInfo |= mulpFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_3E00000;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_1F0000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F & 0x1CUL;
      entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | PIXEL_REG_DEPENDENCY_MASK(field_1F0000);
      entry->scalarOutputDependencies[SLOT_MUL] = PIXEL_REG_DEPENDENCY_MASK(field_1F);

      if(*(iPtr + 3) & 0x80)
      {
        //mul_p Si, Vj, >>#m, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_PScalarShiftImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_60;
      }
      else
      {
        //mul_p Si, Vj, >>svshift, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_PScalarShiftSvshift;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_SVSHIFT;
      }
      return;
    case 0x09:
      //mul_p ru, Vj, >>svshift, Vk or mul_p ru, Vj, >>#m, Vk
      entry->packetInfo |= mulpFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F & 0x1CUL;
      entry->scalarInputDependencies[SLOT_MUL] = PIXEL_REG_DEPENDENCY_MASK(field_3E00000);
      entry->scalarOutputDependencies[SLOT_MUL] = PIXEL_REG_DEPENDENCY_MASK(field_1F);

      if(*(iPtr + 3) & 0x80)
      {
        //mul_p ru, Vj, >>m, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_PRuShiftImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_60;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_RU | DEPENDENCY_MASK_UVCTL;
      }
      else
      {
        //mul_p ru, Vj, >>svshift, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_PRuShiftSvshift;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_RU | DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_SVSHIFT;
      }
      return;
    case 0x0A:
      //mul_p rv, Vj, >>svshift, Vk or mul_p rv, Vj, >>m, Vk
      entry->packetInfo |= mulpFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F & 0x1CUL;
      entry->scalarInputDependencies[SLOT_MUL] = PIXEL_REG_DEPENDENCY_MASK(field_3E00000);
      entry->scalarOutputDependencies[SLOT_MUL] = PIXEL_REG_DEPENDENCY_MASK(field_1F);

      if(*(iPtr + 3) & 0x80)
      {
        //mul_p rv, Vj, >>m, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_PRvShiftImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_60;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_RV | DEPENDENCY_MASK_UVCTL;
      }
      else
      {
        //mul_p rv, Vj, >>svshift, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_PRvShiftSvshift;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_RV | DEPENDENCY_MASK_UVCTL | DEPENDENCY_MASK_SVSHIFT;
      }
      return;
    case 0x0B:
      //mul_p Vi, Vj, >>svshift, Vk or mul_p Vi, Vj, >>m, Vk
      entry->packetInfo |= mulpFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_1F0000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_3E00000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F & 0x1CUL;
      entry->scalarInputDependencies[SLOT_MUL] = PIXEL_REG_DEPENDENCY_MASK(field_3E00000) | PIXEL_REG_DEPENDENCY_MASK(field_1F0000);
      entry->scalarOutputDependencies[SLOT_MUL] = PIXEL_REG_DEPENDENCY_MASK(field_1F);

      if(*(iPtr + 3) & 0x80)
      {
        //mul_p Vi, Vj, >>m, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_PVectorShiftImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_60;
      }
      else
      {
        //mul_p Vi, Vj, >>svshift, Vk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_MUL_PVectorShiftSvshift;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_SVSHIFT;
      }
      return;
    case 0x0C:
      //dotp Si, Vj, >>svshift, Sk or dotp Si, Vj, >>#m, Sk
      entry->packetInfo |= dotpFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_3E00000;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_1F0000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
      entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_1F);

      if(*(iPtr + 3) & 0x80)
      {
        //dotp Si, Vj, >>#m, Sk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_DOTPScalarShiftImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_60;
      }
      else
      {
        //dotp Si, Vj, >>svshift, Sk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_DOTPScalarShiftSvshift;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_SVSHIFT;
      }
      return;
    case 0x0D:
      //dotp Vi, Vj, >>svshift, Sk or dotp Vi, Vj, >>#m, Sk
      entry->packetInfo |= dotpFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_3E00000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_1F0000 & 0x1CUL;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_MUL] = VECTOR_REG_DEPENDENCY_MASK(field_3E00000 & 0x1CUL) | VECTOR_REG_DEPENDENCY_MASK(field_1F0000 & 0x1CUL);
      entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_1F);

      if(*(iPtr + 3) & 0x80)
      {
        //dotp Vi, Vj, >>#m, Sk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_DOTPVectorShiftImmediate;
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_INFO)] = field_60;
      }
      else
      {
        //dotp Vi, Vj, >>svshift, Sk
        entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_DOTPVectorShiftSvshift;
        entry->miscInputDependencies[SLOT_MUL] = DEPENDENCY_MASK_SVSHIFT;
      }
      return;
    case 0x0E:
      //addm
      entry->packetInfo |= addmFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_ADDM;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_3E00000;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_1F0000;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
      entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      return;
    case 0x0F:
      //subm
      entry->packetInfo |= submFlags;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_HANDLER)] = Handler_SUBM;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC1)] = field_3E00000;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_SRC2)] = field_1F0000;
      entry->nuances[FIXED_FIELD(SLOT_MUL,FIELD_MUL_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
      entry->scalarOutputDependencies[SLOT_MUL] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      return;
  }
}
