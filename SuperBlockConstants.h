#ifndef SUPERBLOCKCONSTANTS_H
#define SUPERBLOCKCONSTANTS_H

#include "basetypes.h"
#include "SuperBlock.h"
#include "InstructionCache.h"
#include "InstructionDependencies.h"
#include "PropagateConstants.h"
#include "mpe.h"

#define CONSTANT_REG_RC0 (0)
#define CONSTANT_REG_RC1 (1)
#define CONSTANT_REG_RX (2)
#define CONSTANT_REG_RY (3)
#define CONSTANT_REG_RU (4)
#define CONSTANT_REG_RV (5)
#define CONSTANT_REG_RZ (6)
#define CONSTANT_REG_RZI1 (7)
#define CONSTANT_REG_RZI2 (8)
#define CONSTANT_REG_XYCTL (9)
#define CONSTANT_REG_UVCTL (10)
#define CONSTANT_REG_XYRANGE (11)
#define CONSTANT_REG_UVRANGE (12)
#define CONSTANT_REG_ACSHIFT (13)
#define CONSTANT_REG_SVSHIFT (14)
#define CONSTANT_REG_C (15)
#define CONSTANT_REG_Z (16)
#define CONSTANT_REG_V (17)
#define CONSTANT_REG_N (18)
#define CONSTANT_REG_MV (19)
#define CONSTANT_REG_C0Z (20)
#define CONSTANT_REG_C1Z (21)
#define CONSTANT_REG_MODGE (22)
#define CONSTANT_REG_MODMI (23)
#define CONSTANT_REG_CP0 (24)
#define CONSTANT_REG_CP1 (25)
#define CONSTANT_REG_DISCARD (32)
#define CONSTANT_REG_ALLFLAGS (0xFFFFFFFFUL)
#define CONSTANT_REG_ONLYFLAGS (0xFFFF8000UL)

void PropagateConstants_PacketEnd(SuperBlockConstants &constants);
void PropagateConstants_PacketStart(SuperBlockConstants &constants);

extern const PropagateConstantsHandler ConstantHandlers[];

struct PropagateConstantsStatusStruct
{
  uint32 status;
  uint32 info[5];
};

class SuperBlockConstants
{
public:
  SuperBlockConstants(MPE *pMPE, SuperBlock *sBlock);
  ~SuperBlockConstants();

  void PropagateConstants()
  {
    (ConstantHandlers[pCurrentInstructionEntry->instruction.fields[0]])(*this);
  }
  void FirstInstruction()
  {
    bConstantPropagated = false;
    currentInstructionIndex = 0;
    pCurrentInstructionEntry = pSuperBlock->instructions;
    nuance = &(pSuperBlock->instructions[0].instruction);
  }
  bool NextInstruction()
  {
    currentInstructionIndex++;
    pCurrentInstructionEntry++;
    nuance = &(pCurrentInstructionEntry->instruction);
    return currentInstructionIndex < pSuperBlock->numInstructions;
  }
  void ClearInstructionFlags(const uint32 mask)
  {
    pCurrentInstructionEntry->flags &= (~mask);
  }
  void SetInstructionFlags(const uint32 mask)
  {
    pCurrentInstructionEntry->flags |= mask;
  }
  void ClearAllScalarOutputDependencies()
  {
    pCurrentInstructionEntry->scalarOutputDependencies = 0;
  }
  void ClearAllScalarInputDependencies()
  {
    pCurrentInstructionEntry->scalarInputDependencies = 0;
  }
  void ClearScalarInputDependency(const uint32 index)
  {
    pCurrentInstructionEntry->scalarInputDependencies &= ~(1UL << index);
  }
  void ClearPixelInputDependency(const uint32 index)
  {
    pCurrentInstructionEntry->scalarInputDependencies &= ~(0x7UL << index);
  }
  void ClearShortVectorInputDependency(const uint32 index)
  {
    pCurrentInstructionEntry->scalarInputDependencies &= ~(0xFUL << index);
  }
  void ClearVectorInputDependency(const uint32 index)
  {
    pCurrentInstructionEntry->scalarInputDependencies &= ~(0xFUL << index);
  }
  void ClearAllMiscOutputDependencies()
  {
    pCurrentInstructionEntry->miscOutputDependencies = 0;
  }
  void ClearAllMiscInputDependencies()
  {
    pCurrentInstructionEntry->miscInputDependencies = 0;
  }
  void ClearMiscInputDependency(const uint32 index)
  {
    pCurrentInstructionEntry->miscInputDependencies &= ~(1UL << index);
  }
  void SetScalarInputDependency(const uint32 index)
  {
    pCurrentInstructionEntry->scalarInputDependencies |= (1UL << index);
  }
  void SetPixelInputDependency(const uint32 index)
  {
    pCurrentInstructionEntry->scalarInputDependencies |= (0x7UL << index);
  }
  void SetShortVectorInputDependency(const uint32 index)
  {
    pCurrentInstructionEntry->scalarInputDependencies |= (0xFUL << index);
  }
  void SetVectorInputDependency(const uint32 index)
  {
    pCurrentInstructionEntry->scalarInputDependencies |= (0xFUL << index);
  }
  void SetMiscInputDependency(const uint32 index)
  {
    pCurrentInstructionEntry->miscInputDependencies |= (1UL << index);
  }
  void ClearScalarRegisterConstant(const uint32 index)
  {
    tempScalarRegisterConstantsStatus &= ~(0x1UL << index);
  }
  void ClearPixelRegisterConstant(const uint32 index)
  {
    tempScalarRegisterConstantsStatus &= ~(0x7UL << index);
  }
  void ClearShortVectorRegisterConstant(const uint32 index)
  {
    tempScalarRegisterConstantsStatus &= ~(0xFUL << index);
  }
  void ClearVectorRegisterConstant(const uint32 index)
  {
    tempScalarRegisterConstantsStatus &= ~(0xFUL << index);
  }
  void ClearMiscRegisterConstant(const uint32 index)
  {
    if(index != CONSTANT_REG_ALLFLAGS)
    {
      tempMiscRegisterConstantsStatus &= ~(1UL << index);
    }
    else
    {
      tempMiscRegisterConstantsStatus &= ~DEPENDENCY_FLAG_ALLFLAGS;
    }
  }
  void SetScalarRegisterConstant(const uint32 index, const uint32 value)
  {
    tempScalarRegisterConstantsStatus |= (1UL << index);
    tempScalarRegisterConstants[index] = value;
  }
  void SetMiscRegisterConstant(const uint32 index, const uint32 value)
  {
    tempMiscRegisterConstantsStatus |= (1UL << index);
    tempMiscRegisterConstants[index] = value;
  }
  void ClearConstants()
  {
    scalarRegisterConstantsStatus = 0;
    miscRegisterConstantsStatus = 0;
    tempScalarRegisterConstantsStatus = 0;
    tempMiscRegisterConstantsStatus = 0;
  }
  void CommitConstants()
  {
    scalarRegisterConstantsStatus = tempScalarRegisterConstantsStatus;
    miscRegisterConstantsStatus = tempMiscRegisterConstantsStatus;
    if(scalarRegisterConstantsStatus)
    {
      for(uint32 i = 0; i < 32; i++)
      {
        scalarRegisterConstants[i] = tempScalarRegisterConstants[i];
      }
    }
    if(miscRegisterConstantsStatus)
    {
      for(uint32 i = 0; i < 32; i++)
      {
        miscRegisterConstants[i] = tempMiscRegisterConstants[i];
      }
    }
  }
  bool IsScalarRegisterConstant(const uint32 index) const
  {
    return (scalarRegisterConstantsStatus & (1UL << index)) != 0;
  }
  bool IsMiscRegisterConstant(const uint32 index) const
  {
    return (miscRegisterConstantsStatus & (1UL << index)) != 0;
  }
  uint32 GetScalarRegisterConstant(const uint32 index) const
  {
    return scalarRegisterConstants[index];
  }
  uint32 GetMiscRegisterConstant(const uint32 index) const
  {
    return miscRegisterConstants[index];
  }

  bool EvaluateBranchCondition(const uint32 whichCondition, bool * const branchResult);

  MPE *mpe;
  Nuance *nuance;
  InstructionEntry *pCurrentInstructionEntry;
  PropagateConstantsStatusStruct status;
  bool bConstantPropagated;

protected:
  uint32 currentInstructionIndex;
  uint32 tempScalarRegisterConstants[32];
  uint32 tempMiscRegisterConstants[32];
  uint32 scalarRegisterConstants[32];
  uint32 miscRegisterConstants[32];
  uint32 scalarRegisterConstantsStatus;
  uint32 miscRegisterConstantsStatus;
  uint32 tempScalarRegisterConstantsStatus;
  uint32 tempMiscRegisterConstantsStatus;
  SuperBlock *pSuperBlock;
};

#endif
