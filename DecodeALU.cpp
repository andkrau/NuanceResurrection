#include "basetypes.h"
#include "mpe.h"
#include "Handlers.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "NuonMemoryMap.h"
#include <stdlib.h>

const uint32 absFlags = 0;
const uint32 addScalarFlags = 0;
const uint32 addFlags = 0;
const uint32 addImmediateFlags = 0;
const uint32 addpFlags = 0;
const uint32 addsvFlags = 0;
const uint32 andFlags = 0;
const uint32 asFlags = 0;
const uint32 aslFlags = 0;
const uint32 asrFlags = 0;
const uint32 eorFlags = 0;
const uint32 eorImmediateFlags = 0;
const uint32 ftstFlags = 0;
const uint32 subFlags = 0;
const uint32 subScalarFlags = 0;
const uint32 subImmediateFlags = 0;
const uint32 subImmediateReverseFlags = 0;
const uint32 subpFlags = 0;
const uint32 subsvFlags = 0;
const uint32 cmpFlags = 0;
const uint32 bitsFlags = 0;
const uint32 btstFlags = 0;
const uint32 buttFlags = 0;
const uint32 lsFlags = 0;
const uint32 lsrFlags = 0;
const uint32 msbFlags = 0;
const uint32 orFlags = 0;
const uint32 orImmediateFlags = 0;
const uint32 rotFlags = 0;
const uint32 rolFlags = 0;
const uint32 rorFlags = 0;
const uint32 satFlags = 0;
const uint32 addwcFlags = 0;
const uint32 subwcFlags = 0;
const uint32 cmpwcFlags = 0;
const uint32 copyFlags = 0;
const uint32 negFlags = 0;

void MPE::DecodeInstruction_ALU16(uint8 *iPtr, InstructionCacheEntry *entry, uint32 *immExt)
{
  uint32 field_3E0, field_1F;

  entry->packetInfo |= PACKETINFO_ALU;
  field_3E0 = ((*iPtr & 0x03) << 3) | (*(iPtr + 1) >> 5);
  field_1F = *(iPtr + 1) & 0x1F;

  switch((*iPtr & 0x3C) >> 2)
  {
    case (0x00 >> 2):
      //add Si, Sk
      entry->packetInfo |= addScalarFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDScalar;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;

      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
      return;
    case (0x04 >> 2):
      //add #nnnn, Sk
      entry->packetInfo |= addImmediateFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDImmediate;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;

      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
      return;
    case (0x08 >> 2):
      //copy Si, Sk
      entry->packetInfo |= copyFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_COPY;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = 0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;

      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
      return;
    case (0x0C >> 2):
      //add_sv, sub_sv, neg sk or abs sk
      switch(*(iPtr + 1)  & 0x03)
      {
        case 0x00:
          //add_sv Vi, Vk
          entry->packetInfo |= addsvFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADD_SV;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (field_3E0 & 0x1C);
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = (field_1F & 0x1C);
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = (field_1F & 0x1C);

          entry->scalarInputDependencies[SLOT_ALU] = VECTOR_REG_DEPENDENCY_MASK(field_3E0) | VECTOR_REG_DEPENDENCY_MASK(field_1F);
          entry->scalarOutputDependencies[SLOT_ALU] = VECTOR_REG_DEPENDENCY_MASK(field_1F);
          return;
        case 0x01:
          //sub_sv Vi, Vk
          entry->packetInfo |= subsvFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUB_SV;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (field_3E0 & 0x1C);
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = (field_1F & 0x1C);
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = (field_1F & 0x1C);

          entry->scalarInputDependencies[SLOT_ALU] = VECTOR_REG_DEPENDENCY_MASK(field_3E0) | VECTOR_REG_DEPENDENCY_MASK(field_1F);
          entry->scalarOutputDependencies[SLOT_ALU] = VECTOR_REG_DEPENDENCY_MASK(field_1F);
          return;
        case 0x02:
          //neg Sk
          entry->packetInfo |= negFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBImmediateReverse;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E0;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = 0;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E0;

          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x03:
          //abs Sk
          entry->packetInfo |= absFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ABS;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E0;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E0;          

          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
      }
      break;
    case (0x10 >> 2):
      //sub Si, Sk
      entry->packetInfo |= subScalarFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBScalar;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
      return;
    case (0x14 >> 2):
      //sub #nnnn, Sk
      entry->packetInfo |= subImmediateFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBImmediate;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
      return;
    case (0x18 >> 2):
      //eor #nnnn, Sk
      entry->packetInfo |= eorImmediateFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_EORImmediate;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
      if(*immExt == 0)
      {
        entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_3E0 << 27)) >> 27; //#n
      }
      else
      {
        entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E0;
      }
      return;
    case (0x1C >> 2):
      //cmp Si, Sj
      entry->packetInfo |= cmpFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPScalar;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
      return;
    case (0x20 >> 2):
      //cmp #nnnn, Sj
      entry->packetInfo |= cmpFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPImmediate;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
      return;
    case (0x24 >> 2):
      //and Si, Sk
      entry->packetInfo |= andFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ANDScalar;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
      return;
    case (0x28 >> 2):
      //or Si, Sk
      entry->packetInfo |= orFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ORScalar;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
      return;
    case (0x2C >> 2):
      //eor Si, Sk
      entry->packetInfo |= eorFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_EORScalar;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
      return;
    case (0x30 >> 2):
      //asl #m, Sk
      entry->packetInfo |= aslFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ASL;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (32UL - field_1F);
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E0;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
      return;
    case (0x34 >> 2):
      //asr #m, Sk
      entry->packetInfo |= asrFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ASR;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E0;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
      return;
    case (0x38 >> 2):
      //lsr #m, Sk
      entry->packetInfo |= lsrFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_LSR;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_1F;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E0;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
      return;
    case (0x3C >> 2):
      //btst #m, Sj
      entry->packetInfo |= btstFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_BTST;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (1UL << field_1F);
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E0;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E0;
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E0);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
      return;
  }
}

void MPE::DecodeInstruction_ALU32(uint8 *iPtr, InstructionCacheEntry *entry, uint32 *immExt)
{
  uint32 field_3E00000, field_1F0000, field_1F;

  entry->packetInfo |= PACKETINFO_ALU;

  field_3E00000 = ((*iPtr & 0x03) << 3) | (*(iPtr + 1) >> 5);
  field_1F0000 = *(iPtr + 1) & 0x1F;
  field_1F = *(iPtr + 3) & 0x1F;

  switch(*(iPtr + 2) & 0x0F)
  {
    case 0x00:
      //add
      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //add Si, Sj, Sk
          entry->packetInfo |= addScalarFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x01:
          //add #nnnn, Sj, Sk
          entry->packetInfo |= addImmediateFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((*immExt << 5) | field_3E00000);
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x02:
          //add #nn, Sk
          entry->packetInfo |= addImmediateFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((field_1F << 5) | field_3E00000);
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x03:
          //add #n, >>#m, Sk
          entry->packetInfo |= addImmediateFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(field_1F)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] <<= (32 - field_1F);
          }
          return;
        case 0x04:
          //unused
          return;
        case 0x05:
          //add Si, >>#m, Sk
          entry->packetInfo |= addFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = (((int32)(field_1F << 27)) >> 27);
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if((int32)entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] >= 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDScalarShiftRightImmediate;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDScalarShiftLeftImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = -((int32)(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]));
          }
          return;
        case 0x06:
          //add_sv Vi, Vj, Vk
          entry->packetInfo |= addsvFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADD_SV;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000 & 0x1C;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000 & 0x1C;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F & 0x1C;
          entry->scalarInputDependencies[SLOT_ALU] = VECTOR_REG_DEPENDENCY_MASK(field_3E00000) | VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = VECTOR_REG_DEPENDENCY_MASK(field_1F);
          return;
        case 0x07:
          //add_p Vi, Vj, Vk
          entry->packetInfo |= addpFlags;          
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADD_P;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000 & 0x1C;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000 & 0x1C;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F & 0x1C;
          entry->scalarInputDependencies[SLOT_ALU] = PIXEL_REG_DEPENDENCY_MASK(field_3E00000) | PIXEL_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = PIXEL_REG_DEPENDENCY_MASK(field_1F);
          return;
      }
      return;
    case 0x01:
      //sub
      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //sub Si, Sj, Sk
          entry->packetInfo |= subScalarFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x01:
          //sub #nnnn, Sj, Sk
          entry->packetInfo |= subImmediateFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((*immExt << 5) | field_3E00000);
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x02:
          //sub #nn, Sk
          entry->packetInfo |= subImmediateFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((field_1F << 5) | field_3E00000);
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x03:
          //sub #n, >>#m, Sk
          entry->packetInfo |= subImmediateFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(field_1F)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] <<= (32 - field_1F);
          }
          return;
        case 0x04:
          //sub Si, #nnnnn, Sk
          entry->packetInfo |= subImmediateReverseFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBImmediateReverse;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = (*immExt << 5) | field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x05:
          //sub Si, >>#m, Sk
          entry->packetInfo |= subFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = ((int32)(field_1F << 27)) >> 27;;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(((int32)entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]) >= 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBScalarShiftRightImmediate;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBScalarShiftLeftImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = -((int32)(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]));
          }
          return;
        case 0x06:
          //sub_sv Vi, Vj, Vk
          entry->packetInfo |= subsvFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUB_SV;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000 & 0x1C;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000 & 0x1C;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F & 0x1C;
          entry->scalarInputDependencies[SLOT_ALU] = VECTOR_REG_DEPENDENCY_MASK(field_3E00000) | VECTOR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = VECTOR_REG_DEPENDENCY_MASK(field_1F);
          return;
        case 0x07:
          //sub_p Vi, Vj, Vk
          entry->packetInfo |= subpFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUB_P;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000 & 0x1C;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000 & 0x1C;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F & 0x1C;
          entry->scalarInputDependencies[SLOT_ALU] = PIXEL_REG_DEPENDENCY_MASK(field_3E00000) | PIXEL_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = PIXEL_REG_DEPENDENCY_MASK(field_1F);
          return;
      }
      return;
    case 0x02:
      //cmp
      entry->packetInfo |= cmpFlags;

      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //unused
          break;
        case 0x01:
          //unused
          break;
        case 0x02:
          //cmp #nn, Sq
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((field_1F << 5) | field_3E00000);
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x03:
          //cmp #n, >>#m, Sq
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(field_1F)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] <<= (32 - field_1F);
          }
          return;
        case 0x04:
          //cmp Si, #nnnn
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPImmediateReverse;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = (*immExt << 5) | field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x05:
          //cmp Si, >>#m, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPImmediateReverse;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = ((int32)(field_1F << 27)) >> 27;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(((int32)entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]) >= 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPScalarShiftRightImmediate;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPScalarShiftLeftImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = -((int32)(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]));
          }
          return;
        case 0x06:
        case 0x07:
          //unused
          return;
      }
      return;
    case 0x03:
      //and
      entry->packetInfo |= andFlags;

      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //and Si, Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ANDScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x01:
          //and #n, Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ANDImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(*immExt == 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_3E00000 << 27)) >> 27; //#n
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E00000;
          }
          return;
        case 0x02:
          //and #n, <>#m, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ANDImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_3E00000 << 27)) >> 27; //#n;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if((field_1F & 0x10) == 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = _lrotr(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)], field_1F);
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = _lrotl(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)], 32 - field_1F);
          }
          return;
        case 0x03:
          //and #n, >>Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ANDImmediateShiftScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E00000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(*immExt)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_1F;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_1F << 27)) >> 27;
          }
          return;
        case 0x04:
          //and Si, >>#m, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = ((int32)(field_1F << 27)) >> 27;;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(((int32)entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]) >= 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ANDScalarShiftRightImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] &= 0x3FUL;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ANDScalarShiftLeftImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = -((int32)(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)])) & 0x3FUL;
          }
          return;
        case 0x05:
          //and Si, >>Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ANDScalarShiftScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x06:
          //and Si, <>Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ANDScalarRotateScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x07:
          //unused
          return;
      }
      break;
    case 0x04:
      //ftst
      entry->packetInfo |= ftstFlags;

      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //ftst Si, Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_FTSTScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x01:
          //ftst #n, Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_FTSTImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(*immExt == 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_3E00000 << 27)) >> 27; //#n
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E00000;
          }
          return;
        case 0x02:
          //ftst #n, <>#m, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_FTSTImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_3E00000 << 27)) >> 27; //#n
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if((field_1F & 0x10) == 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = _lrotr(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)], field_1F);
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = _lrotl(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)], 32 - field_1F);
          }
          return;
        case 0x03:
          //ftst #n, >>Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_FTSTImmediateShiftScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000; //>>Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E00000; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(*immExt)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_1F;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_1F << 27)) >> 27;
          }
          return;
        case 0x04:
          //ftst Si, >>#m, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = ((int32)(field_1F << 27)) >> 27;  //>>#m
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(((int32)entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]) >= 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_FTSTScalarShiftRightImmediate;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_FTSTScalarShiftLeftImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = -((int32)(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)])) & 0x3FUL;
          }
          return;
        case 0x05:
          //ftst Si, >>Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_FTSTScalarShiftScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000; //>>Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x06:
          //ftst Si, <>Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_FTSTScalarRotateScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000; //<>Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x07:
          //unused
          return;
      }
      return;
    case 0x05:
      //or


      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //or Si, Sj, Sk
          entry->packetInfo |= orFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ORScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x01:
          //or #n, Sj, Sk
          entry->packetInfo |= orImmediateFlags;            
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ORImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(*immExt == 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_3E00000 << 27)) >> 27; //#n
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E00000;
          }
          return;
        case 0x02:
          //or #n, <>#m, Sk
          entry->packetInfo |= orImmediateFlags;            
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ORImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_3E00000 << 27)) >> 27; //#n;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if((field_1F & 0x10) == 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = _lrotr(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)], field_1F);
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = _lrotl(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)], 32 - field_1F);
          }
          return;
        case 0x03:
          //or #n, >>Sj, Sk
          entry->packetInfo |= orImmediateFlags;            
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ORImmediateShiftScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000; //>>Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E00000; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(*immExt)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_1F;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_1F << 27)) >> 27;
          }
          return;
        case 0x04:
          //or Si, >>#m, Sk
          entry->packetInfo |= orFlags;            
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = ((int32)(field_1F << 27)) >> 27;;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(((int32)entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]) >= 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ORScalarShiftRightImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] &= 0x3FUL;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ORScalarShiftLeftImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = -((int32)(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)])) & 0x3FUL;
          }
          return;
        case 0x05:
          //or Si, >>Sj, Sk
          entry->packetInfo |= orFlags;            
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ORScalarShiftScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x06:
          //or Si, <>Sj, Sk
          entry->packetInfo |= orFlags;            
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ORScalarRotateScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x07:
          //unused
          return;
      }
      return;
    case 0x06:

      //eor
      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //eor Si, Sj, Sk
          entry->packetInfo |= eorFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_EORScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x01:
          //eor #n, Sj, Sk
          entry->packetInfo |= eorImmediateFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_EORImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(*immExt == 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_3E00000 << 27)) >> 27; //#n
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E00000;
          }
          return;
        case 0x02:
          //eor #n, <>#m, Sk
          entry->packetInfo |= eorImmediateFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_EORImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_3E00000 << 27)) >> 27; //#n;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if((field_1F & 0x10) == 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = _lrotr(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)], field_1F);
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = _lrotl(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)], 32 - field_1F);
          }
          return;
        case 0x03:
          //eor #n, >>Sj, Sk
          entry->packetInfo |= eorImmediateFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_EORImmediateShiftScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000; //>>Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E00000; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(*immExt)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_1F;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = ((int32)(field_1F << 27)) >> 27;
          }
          return;
        case 0x04:
          //eor Si, >>#m, Sk
          entry->packetInfo |= eorFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = ((int32)(field_1F << 27)) >> 27;;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(((int32)entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]) >= 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_EORScalarShiftRightImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] &= 0x3FUL;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_EORScalarShiftLeftImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = -((int32)(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)])) & 0x3FUL;
          }
          return;
        case 0x05:
          //eor Si, >>Sj, Sk
          entry->packetInfo |= eorFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_EORScalarShiftScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x06:
          //eor Si, <>Sj, Sk
          entry->packetInfo |= eorFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_EORScalarRotateScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000) | SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x07:
          //unused
          return;
      }
      return;
    case 0x07:
      //as,asl,asr,lsr,ls,rot
      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //as >>Sj, Si, Sk
          entry->packetInfo |= asFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_AS;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_1F0000; //>>Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;  // Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x01:
          //asl #m, Si, Sk
          entry->packetInfo |= aslFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ASL;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = 32UL - field_1F; //#m
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;  // Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x02:
          //asr #m, Si, Sk
          entry->packetInfo |= asrFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ASR;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_1F; //>>#m
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;  // Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x03:
          //ls >>Sj, Si, Sk
          entry->packetInfo |= lsFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_LS;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_1F0000; //>>Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;  // Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x04:
          //lsr #m, Si, Sk
          entry->packetInfo |= lsrFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_LSR;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_1F; //#m
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;  // Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x05:
          //rot <>Sj, Si, Sk
          entry->packetInfo |= rotFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ROT;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_1F0000; //<>Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;  // Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          return;
        case 0x06:
          //rot <>#m, Si, Sk:
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ROT;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;  // Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVZ;
          if(field_1F & 0x10UL)
          {
            entry->packetInfo |= rolFlags;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = 32 - field_1F;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ROL;
            return;
          }
          else
          {
            entry->packetInfo |= rorFlags;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_1F;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ROR;
          }
          return;
        case 0x07:
          //unused
          return;
      }
      return;
    case 0x08:
      //bits,butt,sat,msb
      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //bits #n, >>Si, Sk
          entry->packetInfo |= bitsFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_BITSScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_INFO)] = field_1F;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = 0xFFFFFFFFUL >> (31 - field_1F); // mask ($FFFFFFFF >> (31 - #n))
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000; // >>Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E00000; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NZ;
          return;
        case 0x01:
          //bits #n, >>#m, Sk
          entry->packetInfo |= bitsFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_BITSImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_INFO)] = field_1F;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = 0xFFFFFFFFUL >> (31 - field_1F); // mask ($FFFFFFFF >> (31 - #n))
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000; // >>#m
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E00000; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NZ;
          return;
        case 0x02:
          //butt Si, Sj, Sk
          entry->packetInfo |= buttFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_BUTT;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000; // Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000; // Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = BUTTERFLY_REG_DEPENDENCY_MASK(field_1F);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x03:
          //sat #n, Si, Sk
          entry->packetInfo |= satFlags;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SAT;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F; // #n
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000; //Sk
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NZ;
          return;
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
          //unused
          return;
      }
      return;
    case 0x09:
      //msb Si, Sk
      entry->packetInfo |= msbFlags;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_MSB;
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_1F0000; //Si
      entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_3E00000; //Sk
      entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
      entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
      entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NZ;
      return;
    case 0x0C:
      //addwc
      entry->packetInfo |= addwcFlags;

      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //addwc Si, Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDWCScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x01:
          //addwc #nnnn, Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDWCImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x02:
          //addwc #nn, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDWCImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (field_1F << 5) | field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x03:
          //addwc #n, >>#m, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDWCImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(field_1F)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] >>= (32 - field_1F);
          }
          return;
        case 0x04:
          //unused
          return;
        case 0x05:
          //addwc Si, >>#m, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = ((int32)(field_1F << 27)) >> 27;;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(((int32)entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]) >= 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDWCScalarShiftRightImmediate;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_ADDWCScalarShiftLeftImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = -((int32)(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]));
          }
          return;
        case 0x06:
        case 0x07:
          //unused
          return;
      }
      return;
    case 0x0D:
      //subwc
      entry->packetInfo |= subwcFlags;

      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //subwc Si, Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBWCScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x01:
          //subwc #nnnn, Sj, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBWCImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x02:
          //subwc #nn, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBWCImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (field_1F << 5) | field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x03:
          //subwc #n, >>#m, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBWCImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(field_1F)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] >>= (32 - field_1F);
          }
          return;
        case 0x04:
          //subwc Si, #nnnnn, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBWCImmediateReverse;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = (*immExt << 5) | field_1F0000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x05:
          //subwc Si, >>#m, Sk
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = ((int32)(field_1F << 27)) >> 27;;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->scalarOutputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(((int32)entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]) >= 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBWCScalarShiftRightImmediate;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_SUBWCScalarShiftLeftImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = -((int32)(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]));
          }
          return;
        case 0x06:
        case 0x07:
          //unused
          return;
      }
      return;
    case 0x0E:
      //cmpwc
      entry->packetInfo |= cmpwcFlags;

      switch(*(iPtr + 3) >> 5)
      {
        case 0x00:
          //cmpwc Si, Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPWCScalar;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x01:
          //cmpwc #nnnn, Sj
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPWCImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (*immExt << 5) | field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x02:
          //cmpwc #nn, Sq (the SDK lists cmpwc #n, Sj too but its the same instruction)
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPWCImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = (field_1F << 5) | field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          return;
        case 0x03:
          //cmpwc #n, >>#m, Sq
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPWCImmediate;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(field_1F)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] >>= (32 - field_1F);
          }
          return;
        case 0x04:
          //cmpwc Si, #nn or cmpwc Si, #nnnn
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPWCImmediateReverse;
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000;
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = (*immExt << 5) | field_1F0000;
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000);
          return;
        case 0x05:
          //cmpwc Si, >>#m, Sq
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC1)] = field_3E00000; //Si
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = ((int32)(field_1F << 27)) >> 27; //#m
          entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_DEST)] = field_1F0000; //Sq
          entry->scalarInputDependencies[SLOT_ALU] = SCALAR_REG_DEPENDENCY_MASK(field_3E00000) | SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
          entry->miscInputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_C;
          entry->miscOutputDependencies[SLOT_ALU] = DEPENDENCY_FLAG_NVCZ;
          if(((int32)entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]) >= 0)
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPWCScalarShiftRightImmediate;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_HANDLER)] = Handler_CMPWCScalarShiftLeftImmediate;
            entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)] = -((int32)(entry->nuances[FIXED_FIELD(SLOT_ALU,FIELD_ALU_SRC2)]));
          }
          return;
        case 0x06:
        case 0x07:
          //unused
          return;
      }
      return;
  }
}