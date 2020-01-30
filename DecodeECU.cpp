#include "mpe.h"
#include "Handlers.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "NuonMemoryMap.h"

//Table to map 16/48 bit condition code enumeration to 32/64 bit sequence
//uint32 CC16To32[32] = {0,4,8,12,16,20,24,28,1,5,9,13,17,21,17,29,2,6,10,14,18,22,17,30,3,7,11,15,19,17,27,31};

#define braConditionalFlags 0
#define braConditionalNOPFlags 0
#define braAlwaysFlags 0
#define braAlwaysNOPFlags 0
#define jmpAlwaysIndirectFlags 0
#define jmpAlwaysIndirectNOPFlags 0
#define jmpConditionalIndirectFlags 0
#define jmpConditionalIndirectNOPFlags 0
#define jsrAlwaysFlags 0
#define jsrAlwaysNOPFlags 0
#define jsrAlwaysIndirectFlags 0
#define jsrAlwaysIndirectNOPFlags 0
#define jsrConditionalFlags 0
#define jsrConditionalNOPFlags 0
#define jsrConditionalIndirectFlags 0
#define jsrConditionalIndirectNOPFlags 0
#define rtsAlwaysFlags 0
#define rtsAlwaysNOPFlags 0
#define rtsConditionalFlags 0
#define rtsConditionalNOPFlags 0
#define rti1ConditionalFlags 0
#define rti1ConditionalNOPFlags 0
#define rti2ConditionalFlags 0
#define rti2ConditionalNOPFlags 0

static const uint32 flagDependencies[32] = {
  //ECU_CONDITION_NE (0x00UL)
  DEPENDENCY_FLAG_Z,
  //ECU_CONDITION_C0EQ (0x01UL)
  DEPENDENCY_FLAG_C0Z,
  //ECU_CONDITION_C1EQ (0x02UL)
  DEPENDENCY_FLAG_C1Z,
  //ECU_CONDITION_CC (0x03UL)
  DEPENDENCY_FLAG_C,
  //ECU_CONDITION_EQ (0x04UL)
  DEPENDENCY_FLAG_Z,
  //ECU_CONDITION_CS (0x05UL)
  DEPENDENCY_FLAG_C,
  //ECU_CONDITION_VC (0x06UL)
  DEPENDENCY_FLAG_V,
  //ECU_CONDITION_VS (0x07UL)
  DEPENDENCY_FLAG_V,
  //ECU_CONDITION_LT (0x08UL)
  DEPENDENCY_FLAG_N | DEPENDENCY_FLAG_V,
  //ECU_CONDITION_MVC (0x09UL)
  DEPENDENCY_FLAG_MV,
  //ECU_CONDITION_MVS (0x0AUL)
  DEPENDENCY_FLAG_MV,
  //ECU_CONDITION_HI (0x0BUL)
  DEPENDENCY_FLAG_C | DEPENDENCY_FLAG_Z,
  //ECU_CONDITION_LE (0x0CUL)
  DEPENDENCY_FLAG_N | DEPENDENCY_FLAG_V | DEPENDENCY_FLAG_Z,
  //ECU_CONDITION_LS (0x0DUL)
  DEPENDENCY_FLAG_C | DEPENDENCY_FLAG_Z,
  //ECU_CONDITION_PL (0x0EUL)
  DEPENDENCY_FLAG_N,
  //ECU_CONDITION_MI (0x0FUL)
  DEPENDENCY_FLAG_N,
  //ECU_CONDITION_GT (0x10UL)
  DEPENDENCY_FLAG_N | DEPENDENCY_FLAG_V | DEPENDENCY_FLAG_Z,
  //ECU_CONDITION_T (0x11UL)
  0,
  //ECU_CONDITION_MODMI (0x12UL)
  DEPENDENCY_FLAG_MODMI,
  //ECU_CONDITION_MODPL (0x13UL)
  DEPENDENCY_FLAG_MODMI,
  //ECU_CONDITION_GE (0x14UL)
  DEPENDENCY_FLAG_N | DEPENDENCY_FLAG_V,
  //ECU_CONDITION_MODGE (0x15UL)
  DEPENDENCY_FLAG_MODGE,
  //ECU_CONDITION_MODLT (0x16UL)
  DEPENDENCY_FLAG_MODGE,
  //NOT USED (0x17)
  0,
  //ECU_CONDITION_C0NE (0x18UL)
  DEPENDENCY_FLAG_C0Z,
  //NOT USED (0x19)
  0,
  //NOT USED (0x1A)
  0,
  //ECU_CONDITION_CF0LO (0x1BUL)
  DEPENDENCY_FLAG_CP0,
  //ECU_CONDITION_C1NE (0x1CUL)
  DEPENDENCY_FLAG_C1Z,
  //ECU_CONDITION_CF0HI (0x1DUL)
  DEPENDENCY_FLAG_CP0,
  //ECU_CONDITION_CF1LO (0x1EUL)
  DEPENDENCY_FLAG_CP1,
  //ECU_CONDITION_CF1HI (0x1FUL)
  DEPENDENCY_FLAG_CP1,
};

void MPE::DecodeInstruction_ECU16(const uint8 * const iPtr, InstructionCacheEntry * const entry, const uint32 * const immExt)
{
  const uint32 field_300 = *iPtr & 0x03UL;
  const uint32 field_380 = ((*iPtr & 0x03) << 1) | (*(iPtr + 1) >> 7);
  const uint32 field_3E0 = ((*iPtr & 0x03) << 3) | (*(iPtr + 1) >> 5);
  const uint32 field_7F = *(iPtr + 1) & 0x7FUL;
  const uint32 field_FF = *(iPtr + 1) & 0xFFUL;

  entry->packetInfo |= PACKETINFO_ECU;

  if(*iPtr & 0x0C)
  {
    //BRA
    if((*iPtr & 0x04) == 0)
    {
      //BRA cc

      //condition is 0x380 or 0x30380, offset is 07F or 0x7FFFFFF8007F
      
      if(*immExt == 0)
      {
        //BRA 16

        //can convert condition code sequencing using a simple multiply by four
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_CONDITION)] = field_380 << 2;
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] = ((int32)(field_7F << 25)) >> (25 - 1);
      }
      else
      {
        //BRA 48 (LBRA)
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_CONDITION)] = (field_380 << 2) | (*immExt & 0x03UL);
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] = (((*immExt & 0x07FFFFF8UL) << 4)| field_7F) << 1;
      }

      if(entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_CONDITION)] == ECU_CONDITION_T)
      {
        if((*immExt & 0x04UL) >> 2)
        {
          //BRA T, NOP
          entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAAlways_NOP;
          entry->packetInfo |= (PACKETINFO_BRANCH_ALWAYS | PACKETINFO_BRANCH_NOP);
          entry->packetInfo |= braAlwaysNOPFlags;
        }
        else
        {
          //BRA T
          entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAAlways;
          entry->packetInfo |= PACKETINFO_BRANCH_ALWAYS;
          entry->packetInfo |= braAlwaysFlags;
        }
      }
      else
      {
        if((*immExt & 0x04UL) >> 2)
        {
          //BRA CC, NOP
          entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAConditional_NOP;
          entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_NOP);
          entry->packetInfo |= braConditionalNOPFlags;
        }
        else
        {
          //BRA CC
          entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAConditional;
          entry->packetInfo |= PACKETINFO_BRANCH_CONDITIONAL;
          entry->packetInfo |= braConditionalFlags;
        }
      }
    }
    else
    {
      //BRA always

      //offset is 0x3FF
      entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_CONDITION)] = ECU_CONDITION_T;
      const int32 offset = ((int32)(field_300 << 30)) >> (30 - 8);
      entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] = (offset | field_FF) << 1;
      entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAAlways;
      entry->packetInfo |= PACKETINFO_BRANCH_ALWAYS;
      entry->packetInfo |= braAlwaysFlags;
    }

    entry->miscInputDependencies[SLOT_ECU] = flagDependencies[entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_CONDITION)]]; 
    //add packet address to offset
    entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] += entry->pcexec;
  }
  else
  {
    //RTI, RTS or HALT
    if(*(iPtr + 1) != 0x01)
    {
      //RTI or RTS

      //The first three bits of the instruction determine the register to use as the target address

      //condition is 0x03E0
      entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_CONDITION)] = field_3E0;
      const bool bImplicitNOP = ((*(iPtr + 1) & 0x01) != 0);

      switch((*(iPtr + 1) & 0x07) >> 1)
      {
        case 0:
          entry->miscInputDependencies[SLOT_ECU] = (flagDependencies[field_3E0] | DEPENDENCY_MASK_RZ);

          if(field_3E0 == ECU_CONDITION_T)
          {
            if(bImplicitNOP)
            {
              entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_RTSAlways_NOP;
              entry->packetInfo |= (PACKETINFO_BRANCH_ALWAYS | PACKETINFO_BRANCH_NOP);
              entry->packetInfo |= rtsAlwaysNOPFlags;
            }
            else
            {
              entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_RTSAlways;
              entry->packetInfo |= PACKETINFO_BRANCH_ALWAYS;
              entry->packetInfo |= rtsAlwaysFlags;
            }
          }
          else
          {
            if(bImplicitNOP)
            {
              entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_RTSConditional_NOP;
              entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_NOP);
              entry->packetInfo |= rtsConditionalNOPFlags;
            }
            else
            {
              entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_RTSConditional;
              entry->packetInfo |= PACKETINFO_BRANCH_CONDITIONAL;
              entry->packetInfo |= rtsConditionalFlags;
            }
          }
          break;
        case 1:
          entry->miscInputDependencies[SLOT_ECU] = flagDependencies[field_3E0] | DEPENDENCY_MASK_RZI1;
          if(bImplicitNOP)
          {
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_RTI1Conditional_NOP;
            entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_NOP);
            entry->packetInfo |= rti1ConditionalNOPFlags;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_RTI1Conditional;
            entry->packetInfo |= PACKETINFO_BRANCH_CONDITIONAL;
            entry->packetInfo |= rti1ConditionalFlags;
          }
          break;
        case 2:
          entry->miscInputDependencies[SLOT_ECU] = flagDependencies[field_3E0] | DEPENDENCY_MASK_RZI2;
          if(bImplicitNOP)
          {
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_RTI2Conditional_NOP;
            entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_NOP);
            entry->packetInfo |= rti2ConditionalNOPFlags;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_RTI2Conditional;
            entry->packetInfo |= PACKETINFO_BRANCH_CONDITIONAL;
            entry->packetInfo |= rti2ConditionalFlags;
          }
          break;
      }
    }
    else
    {
      //HALT
      entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_Halt;
      entry->packetInfo |= (PACKETINFO_NEVERCOMPILE | PACKETINFO_EXCEPTION);    
    }
  }

  entry->ecuConditionCode = entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_CONDITION)];
}

void MPE::DecodeInstruction_ECU32(const uint8 * const iPtr, InstructionCacheEntry *const entry, const uint32 * const immExt)
{
  const uint32 field_3E00000 = ((*iPtr & 0x03) << 3) | (*(iPtr + 1) >> 5);
  const uint32 field_1F0000 = *(iPtr + 1) & 0x1FUL;
  const uint32 field_100 = *(iPtr + 2) & 0x01UL;
  const uint32 field_FF = *(iPtr + 3) & 0xFFUL;
  const bool bImplicitNOP = (((*(iPtr + 2) & 0x08) >> 3) != 0);

  entry->packetInfo |= PACKETINFO_ECU;

  entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_CONDITION)] = field_3E00000;
  entry->miscInputDependencies[SLOT_ECU] = flagDependencies[field_3E00000];

  if((*(iPtr + 2) & 0x06) == 0)
  {
    //bra 32 (lbra)

    //address is 0x1F01FF, pcexec relative: lower bits are stored first!

    //offset = ((signed __int32)((signed __int8)(field_100 << 7))) << 6;
    int32 offset = ((int32)(field_100 << 31)) >> 18;
    offset |= ((field_FF << 5) | field_1F0000);
    entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] = entry->pcexec + (offset << 1);

    if(field_3E00000 == ECU_CONDITION_T)
    {
      if(bImplicitNOP)
      {
        //BRA T, NOP
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAAlways_NOP;
        entry->packetInfo |= (PACKETINFO_BRANCH_ALWAYS | PACKETINFO_BRANCH_NOP);
        entry->packetInfo |= braAlwaysNOPFlags;
      }
      else
      {
        //BRA T
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAAlways;
        entry->packetInfo |= PACKETINFO_BRANCH_ALWAYS;
        entry->packetInfo |= braAlwaysFlags;
      }
    }
    else
    {
      if(bImplicitNOP)
      {
        //BRA CC, NOP
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAConditional_NOP;
        entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_NOP);
        entry->packetInfo |= braConditionalNOPFlags;
      }
      else
      {
        //BRA CC
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAConditional;
        entry->packetInfo |= PACKETINFO_BRANCH_CONDITIONAL;
        entry->packetInfo |= braConditionalFlags;
      }
    }
  }
  else
  {
    //jmp or jsr

    switch(*(iPtr + 2) & 0x07)
    {
      case 1:
      case 2:
      case 3:
        //jmp cc, <label>

        //address is 0x1F00FF: if immExt == 0, base relative to bit 8: if bit 8 is clear, IROM, else IRAM
        //if immExt is non-zero, instruction is 64 bit and the address is absolute
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] = (((*immExt & 0x7FFFE00) << 4) | (((size_t)(*(iPtr + 3) & 0xFF)) << 5) | ((size_t)(*(iPtr + 1) & 0x1F))) << 1;

        if(*immExt == 0)
        {
          if(*(iPtr + 2) & 0x01)
          {
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] += MPE_IRAM_BASE;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] += MPE_IROM_BASE;
          }
        }

        if(field_3E00000 == ECU_CONDITION_T)
        {
          if(bImplicitNOP)
          {
            //JMP T, NOP
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAAlways_NOP;
            entry->packetInfo |= (PACKETINFO_BRANCH_ALWAYS | PACKETINFO_BRANCH_NOP);
            entry->packetInfo |= braAlwaysNOPFlags;
          }
          else
          {
            //JMP T
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAAlways;
            entry->packetInfo |= PACKETINFO_BRANCH_ALWAYS;
            entry->packetInfo |= braAlwaysFlags;
          }
        }
        else
        {
          if(bImplicitNOP)
          {
            //JMP CC, NOP
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAConditional_NOP;
            entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_NOP);
            entry->packetInfo |= braConditionalNOPFlags;
          }
          else
          {
            //JMP CC
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_BRAConditional;
            entry->packetInfo |= PACKETINFO_BRANCH_CONDITIONAL;
            entry->packetInfo |= braConditionalFlags;
          }
        }
        break;
      case 4:
      case 5:
        //jsr cc, <label>
        //address is 0x1F00FF: if immExt == 0, base relative to bit 8: if bit 8 is clear, IROM, else IRAM
        //if immExt is non-zero, instruction is 64 bit and the address is absolute
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] = (((*immExt & 0x7FFFE00) << 4) | (((size_t)(*(iPtr + 3) & 0xFF)) << 5) | ((size_t)(*(iPtr + 1) & 0x1F))) << 1;
        entry->miscOutputDependencies[SLOT_ECU] = DEPENDENCY_MASK_RZ;

        if(*immExt == 0)
        {
          if(*(iPtr + 2) & 0x01)
          {
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] += MPE_IRAM_BASE;
          }
          else
          {
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] += MPE_IROM_BASE;
          }
        }

        if(field_3E00000 == ECU_CONDITION_T)
        {
          if(bImplicitNOP)
          {
            //JSR T, NOP
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JSRAlways_NOP;
            entry->packetInfo |= (PACKETINFO_BRANCH_ALWAYS | PACKETINFO_BRANCH_NOP);
            entry->packetInfo |= jsrAlwaysNOPFlags;
          }
          else
          {
            //JSR T
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JSRAlways;
            entry->packetInfo |= (PACKETINFO_BRANCH_ALWAYS | PACKETINFO_NEEDS_PCFETCHNEXT);
            entry->packetInfo |= jsrAlwaysFlags;
          }
        }
        else
        {
          if(bImplicitNOP)
          {
            //JSR CC, NOP
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JSRConditional_NOP;
            entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_NOP);
            entry->packetInfo |= jsrConditionalNOPFlags;
          }
          else
          {
            //JSR CC
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JSRConditional;
            entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_NEEDS_PCFETCHNEXT);
            entry->packetInfo |= jsrConditionalFlags;
          }
        }
       break;
      case 6:
        //address is register indirect
        //jmp cc, (Si)
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] = field_1F0000;
        entry->scalarInputDependencies[SLOT_ECU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);

        if(field_3E00000 == ECU_CONDITION_T)
        {
          if(bImplicitNOP)
          {
            //JMP T, (Si), NOP
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JMPAlwaysIndirect_NOP;
            entry->packetInfo |= (PACKETINFO_BRANCH_ALWAYS | PACKETINFO_BRANCH_NOP);
            entry->packetInfo |= jmpAlwaysIndirectNOPFlags;
          }
          else
          {
            //JMP T, (Si)
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JMPAlwaysIndirect;
            entry->packetInfo |= PACKETINFO_BRANCH_ALWAYS;
            entry->packetInfo |= jmpAlwaysIndirectFlags;
          }
        }
        else
        {
          if(bImplicitNOP)
          {
            //JMP CC, (Si), NOP
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JMPConditionalIndirect_NOP;
            entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_NOP);
            entry->packetInfo |= jmpConditionalIndirectNOPFlags;
          }
          else
          {
            //JMP CC, (Si)
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JMPConditionalIndirect;
            entry->packetInfo |= PACKETINFO_BRANCH_CONDITIONAL;
            entry->packetInfo |= jmpConditionalIndirectFlags;
          }
        }
        break;
      case 7:
        //address is register indirect
        //jsr cc, (Si)
        entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_ADDRESS)] = field_1F0000;
        entry->scalarInputDependencies[SLOT_ECU] = SCALAR_REG_DEPENDENCY_MASK(field_1F0000);
        entry->miscOutputDependencies[SLOT_ECU] = DEPENDENCY_MASK_RZ;

        if(field_3E00000 == ECU_CONDITION_T)
        {
          if(bImplicitNOP)
          {
            //JSR T, (Si),  NOP
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JSRAlwaysIndirect_NOP;
            entry->packetInfo |= (PACKETINFO_BRANCH_ALWAYS | PACKETINFO_BRANCH_NOP);
            entry->packetInfo |= jsrAlwaysIndirectNOPFlags;     
          }
          else
          {
            //JSR T, (Si)
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JSRAlwaysIndirect;
            entry->packetInfo |= (PACKETINFO_BRANCH_ALWAYS | PACKETINFO_NEEDS_PCFETCHNEXT);
            entry->packetInfo |= jsrAlwaysIndirectFlags;        
          }
        }
        else
        {
          if(bImplicitNOP)
          {
            //JSR CC, (Si), NOP
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JSRConditionalIndirect_NOP;
            entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_BRANCH_NOP);
            entry->packetInfo |= jsrConditionalIndirectNOPFlags;
          }
          else
          {
            //JSR CC, (Si)
            entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_HANDLER)] = Handler_JSRConditionalIndirect;
            entry->packetInfo |= (PACKETINFO_BRANCH_CONDITIONAL | PACKETINFO_NEEDS_PCFETCHNEXT);
            entry->packetInfo |= jsrConditionalIndirectFlags;
          }
        }
        break;
    }
  }

  entry->ecuConditionCode = entry->nuances[FIXED_FIELD(SLOT_ECU,FIELD_ECU_CONDITION)];
}
