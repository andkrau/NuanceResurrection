#include "basetypes.h"
#include <cstdlib>
#include "Handlers.h"
#include "InstructionCache.h"
#include "mpe.h"
#include "PropagateConstants.h"
#include "PropagateConstants_MEM.h"
#include "SuperBlockConstants.h"

#define ALLOW_ALU_PROPAGATION true
#define DISABLE_ADD_IMMEDIATE_PROPAGATION_MR_WAR true // otherwise Merlin Racing renders corruptly on beginning of races

void PropagateConstants_ABS(SuperBlockConstants &constants)
{
  const uint32 srcIndex = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(srcIndex) && ALLOW_ALU_PROPAGATION)
  {
    const int32 src = constants.GetScalarRegisterConstant(srcIndex);
    int32 destValue = src;
    uint32 flagValues = 0;
    if(src < 0)
    {
      constants.SetMiscRegisterConstant(CONSTANT_REG_C, 1);
      flagValues = CC_ALU_CARRY;
      if(src == (int32)0x80000000)
      {
        flagValues |= (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
      }
      destValue = -destValue;
    }
    else
    {
      if(!src)
      {
        flagValues = CC_ALU_ZERO;
      }
    }

    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY | CC_ALU_ZERO;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(srcIndex);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_BITSImmediate(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST];

  if(constants.IsScalarRegisterConstant(destIndex) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const int32 destValue = src1 & (constants.GetScalarRegisterConstant(destIndex) >> src2);
    uint32 flagValues;
    if(!destValue)
    {
      flagValues = CC_ALU_ZERO;
    }
    else
    {
      flagValues = (destValue >> 28) & CC_ALU_NEGATIVE;
    }

    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(destIndex);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_BITSScalar(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST];

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_BITSImmediate;
    constants.nuance->fields[FIELD_ALU_SRC2] = constants.GetScalarRegisterConstant(src2Index) & 0x1FUL;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_BITSImmediate(constants);
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_BTST(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_N, 0);
  constants.SetMiscRegisterConstant(CONSTANT_REG_V, 0);

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const int32 destValue = src1 & constants.GetScalarRegisterConstant(src2Index);
    uint32 flagValues = 0;
    if(!destValue)
    {
      flagValues = CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreMiscRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = CONSTANT_REG_DISCARD;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_ZERO;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreMiscRegisterConstant(constants);
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_BUTT(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_V);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
  constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
  constants.ClearScalarRegisterConstant(destIndex);    
  constants.ClearScalarRegisterConstant(destIndex + 1);    
  constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
}

void PropagateConstants_COPY(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V, 0);

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const int32 destValue = constants.GetScalarRegisterConstant(src2Index);

    uint32 flagValues;
    if(!destValue)
    {
      flagValues = CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      flagValues = ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_MSB(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    int32 n = constants.GetScalarRegisterConstant(src1Index);
    uint32 sigbits;
    if((n == 0) || (n == -1))
    {
      sigbits = 0;
    }
    else
    {
      //n = n if positive, n = ~n if negative
      n = (n ^ (n >> 31));

      //fold n into itself to get a new value where all bits below the
      //most significant one bit have also been set to one.

      n |= (n >> 1);
      n |= (n >> 2);
      n |= (n >> 4);
      n |= (n >> 8);
      n |= (n >> 16);

      //get the ones count

      n -= ((n >> 1) & 0x55555555);
      n = (((n >> 2) & 0x33333333) + (n & 0x33333333));
      n = (((n >> 4) + n) & 0x0f0f0f0f);
      n += (n >> 8);
      n += (n >> 16);

      //return the ones count... if n was originally 0 or -1 then the ones count
      //will be zero which is exactly what we want
      sigbits = ((uint32)n) & 0x1FUL;
    }

    uint32 flagValues = 0;
    if(!sigbits)
    {
      flagValues = CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = sigbits;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_ZERO;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src1Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_SAT(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    int32 mask = (0x01UL << constants.nuance->fields[FIELD_ALU_SRC2]) - 1;
    const int32 n = constants.GetScalarRegisterConstant(src1Index);

    //initial mask is largest negative number using 'bits' bits, minus one to get
    //largest positive signed number using 'bits' bits

    //NOTE: the bits parameter will be one less than the number of bits desired
    //so you do not have to subtract 1 from the shift amount to get the correct
    //mask

    uint32 destValue;
    if(n > mask)
    {
      destValue = mask;
    }
    else
    {
      //inverting mask gives smallest negative number possible using 'bits' bits
      mask = ~mask;

      if(n < mask)
      {
        destValue = mask;
      }
      else
      {
        destValue = n;
      }
    }

    uint32 flagValues;
    if(!destValue)
    {
      flagValues = CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      flagValues = ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src1Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_ASL(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);
  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    // carry = bit 31 of source
    uint32 flagValues = ((src2 >> 30) & CC_ALU_CARRY);
    uint32 destValue = src2 << constants.nuance->fields[FIELD_ALU_SRC1];

    //AS <immediate>,<scalar>,<scalar> will be transformed into ASL.  The AS
    //instruction allows left shifts of 32 so this handler must support this
    //scenario even though ASL cannot shift by 32.
    if(!destValue || (constants.nuance->fields[FIELD_ALU_SRC1] == 32))
    {
      destValue = 0;
      flagValues |= CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      flagValues |= ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_CARRY | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_ASR(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);
  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    // carry = bit 31 of source
    uint32 flagValues = ((src2 << 1) & CC_ALU_CARRY);
    const uint32 destValue = ((int32)src2) >> constants.nuance->fields[FIELD_ALU_SRC1];

    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      flagValues |= ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_CARRY | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_AS(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);
  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index) & 0x3FUL;
    const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
    if(src1 & 0x20)
    {
      //shift left
      constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ASL;
      constants.nuance->fields[FIELD_ALU_SRC1] = 64UL - src1;
      constants.ClearScalarInputDependency(src1Index);
      constants.SetScalarInputDependency(src2Index);
      PropagateConstants_ASL(constants);    
    }
    else
    {
      constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ASR;
      constants.nuance->fields[FIELD_ALU_SRC1] = src1;
      constants.ClearScalarInputDependency(src1Index);
      constants.SetScalarInputDependency(src2Index);
      PropagateConstants_ASR(constants);
    }
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_LSR(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);
  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    // carry = bit 31 of source
    uint32 flagValues = ((src2 << 1) & CC_ALU_CARRY);
    const uint32 destValue = src2 >> constants.nuance->fields[FIELD_ALU_SRC1];

    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      flagValues |= ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_CARRY | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_LS(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);
  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index) & 0x3FUL;
    const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
    if(src1 & 0x20)
    {
      //shift left
      constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ASL;
      constants.nuance->fields[FIELD_ALU_SRC1] = 64UL - src1;
      constants.ClearScalarInputDependency(src1Index);
      constants.SetScalarInputDependency(src2Index);
      PropagateConstants_ASL(constants);    
    }
    else
    {
      constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_LSR;
      constants.nuance->fields[FIELD_ALU_SRC1] = src1;
      constants.ClearScalarInputDependency(src1Index);
      constants.SetScalarInputDependency(src2Index);
      PropagateConstants_LSR(constants);    
    }
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_ROL(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);
  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    const uint32 destValue = _lrotl(src2, constants.nuance->fields[FIELD_ALU_SRC1]);
    uint32 flagValues;
    if(!destValue)
    {
      flagValues = CC_ALU_ZERO;
    }
    else
    {
      flagValues = ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_ROR(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);
  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    const uint32 destValue = _lrotr(src2, constants.nuance->fields[FIELD_ALU_SRC1]);
    uint32 flagValues;
    if(!destValue)
    {
      flagValues = CC_ALU_ZERO;
    }
    else
    {
      flagValues = ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_ROT(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);
  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index) & 0x3FUL;
    const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
    if(src1 & 0x20)
    {
      constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ROL;
      constants.nuance->fields[FIELD_ALU_SRC1] = 64 - src1;
      constants.ClearScalarInputDependency(src1Index);
      constants.SetScalarInputDependency(src2Index);
      PropagateConstants_ROL(constants);    
    }
    else
    {
      constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ROR;
      constants.nuance->fields[FIELD_ALU_SRC1] = src1;
      constants.ClearScalarInputDependency(src1Index);
      constants.SetScalarInputDependency(src2Index);
      PropagateConstants_ROR(constants);    
    }
  }
  else
  {
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);    
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  }
}

void PropagateConstants_ADD_P(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  constants.ClearPixelRegisterConstant(destIndex);    
}

void PropagateConstants_SUB_P(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1]; 
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2]; 
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if((src1Index == src2Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.SetScalarRegisterConstant(destIndex    , 0);    
    constants.SetScalarRegisterConstant(destIndex + 1, 0);    
    constants.SetScalarRegisterConstant(destIndex + 2, 0);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearPixelRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ADD_SV(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
  constants.ClearShortVectorRegisterConstant(destIndex);    
}

void PropagateConstants_SUB_SV(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1]; 
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2]; 
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if((src1Index == src2Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.SetScalarRegisterConstant(destIndex    , 0);    
    constants.SetScalarRegisterConstant(destIndex + 1, 0);    
    constants.SetScalarRegisterConstant(destIndex + 2, 0);    
    constants.SetScalarRegisterConstant(destIndex + 3, 0);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearShortVectorRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ADDImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION && !DISABLE_ADD_IMMEDIATE_PROPAGATION_MR_WAR)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    uint32 destValue;
    uint32 flagValues = _addcarry_u32(0, src1, src2, &destValue) ? CC_ALU_CARRY : 0;
    if((~(src1 ^ src2)) & (src1 ^ destValue) & 0x80000000u)
    {
      flagValues |= CC_ALU_OVERFLOW;
    }
    if(destValue & 0x80000000u)
    {
      flagValues |= CC_ALU_NEGATIVE;
    }
    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);
  }
}

void PropagateConstants_ADDScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ADDImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_ADDImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ADDScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ADDImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = ((int32)src1) >> src2;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ADDImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ADDScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ADDImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 << src2; // do not cast to u64 before shift!
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ADDImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_SUBImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    uint32 destValue;
    uint32 flagValues = _subborrow_u32(0, src2, src1, &destValue) ? CC_ALU_CARRY : 0;
    if((src1 ^ src2) & (src2 ^ destValue) & 0x80000000u)
    {
      flagValues |= CC_ALU_OVERFLOW;
    }
    if(destValue & 0x80000000u)
    {
      flagValues |= CC_ALU_NEGATIVE;
    }
    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);
  }
}

void PropagateConstants_SUBImmediateReverse(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    uint32 destValue;
    uint32 flagValues = _subborrow_u32(0, src2, src1, &destValue) ? CC_ALU_CARRY : 0;
    if((src1 ^ src2) & (src2 ^ destValue) & 0x80000000u)
    {
      flagValues |= CC_ALU_OVERFLOW;
    }
    if(destValue & 0x80000000u)
    {
      flagValues |= CC_ALU_NEGATIVE;
    }
    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src1Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);
  }
}

void PropagateConstants_SUBScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if((src1Index == src2Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = 0;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = CC_ALU_ZERO;
    constants.ClearScalarInputDependency(src2Index);
    constants.ClearScalarInputDependency(src1Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);      
  }
  else if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_SUBImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_SUBImmediate(constants);    
  }
  else if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_SUBImmediateReverse;
    constants.nuance->fields[FIELD_ALU_SRC2] = src2;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    PropagateConstants_SUBImmediateReverse(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_SUBScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_SUBImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = ((int32)src1) >> src2;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_SUBImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_SUBScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_SUBImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 << src2; // do not cast to u64 before shift!
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_SUBImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_CMPImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    uint32 destValue;
    uint32 flagValues = _subborrow_u32(0, src2, src1, &destValue) ? CC_ALU_CARRY : 0;
    if((src1 ^ src2) & (src2 ^ destValue) & 0x80000000u)
    {
      flagValues |= CC_ALU_OVERFLOW;
    }
    if(destValue & 0x80000000u)
    {
      flagValues |= CC_ALU_NEGATIVE;
    }
    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreMiscRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = CONSTANT_REG_DISCARD;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreMiscRegisterConstant(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
  }
}

void PropagateConstants_CMPImmediateReverse(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    uint32 destValue;
    uint32 flagValues = _subborrow_u32(0, src2, src1, &destValue) ? CC_ALU_CARRY : 0;
    if((src1 ^ src2) & (src2 ^ destValue) & 0x80000000u)
    {
      flagValues |= CC_ALU_OVERFLOW;
    }
    if(destValue & 0x80000000u)
    {
      flagValues |= CC_ALU_NEGATIVE;
    }
    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreMiscRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = CONSTANT_REG_DISCARD;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src1Index);
    PropagateConstants_StoreMiscRegisterConstant(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
  }
}

void PropagateConstants_CMPScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_CMPImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_CMPImmediate(constants);    
  }
  else if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_CMPImmediateReverse;
    constants.nuance->fields[FIELD_ALU_SRC2] = src2;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    PropagateConstants_CMPImmediateReverse(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}

void PropagateConstants_CMPScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_CMPImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = ((int32)src1) >> src2;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_CMPImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}

void PropagateConstants_CMPScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_CMPImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 << src2; // do not cast to u64 before shift!
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_CMPImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}

void PropagateConstants_ANDImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);
  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    const uint32 destValue = src1 & src2;
    uint32 flagValues;
    if(!destValue)
    {
      flagValues = CC_ALU_ZERO;
    }
    else
    {
      flagValues = ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ANDScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ANDImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_ANDImmediate(constants);    
  }
  else if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ANDImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_ALU_SRC2] = src1Index;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    PropagateConstants_ANDImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ANDImmediateShiftScalar(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
          uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = (((int32)(constants.GetScalarRegisterConstant(src2Index) << 26)) >> 26) & 0x3FUL;
    if(src2 & 0x20)
    {
      src1 = (src1 << (64UL - src2));
    }
    else
    {
      src1 = (src1 >> src2);
    }

    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ANDImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ANDImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}
void PropagateConstants_ANDScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ANDImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 >> (constants.nuance->fields[FIELD_ALU_SRC2] & 0x3FUL);
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ANDImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);
  }
}

void PropagateConstants_ANDScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ANDImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 << (constants.nuance->fields[FIELD_ALU_SRC2] & 0x3FUL); // do not cast to u64 before shift!
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ANDImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);
  }
}

void PropagateConstants_ANDScalarShiftScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ANDImmediateShiftScalar;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ANDImmediateShiftScalar(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
    constants.ClearScalarRegisterConstant(destIndex);
  }
}

void PropagateConstants_ANDScalarRotateScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
          uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    const uint32 src2 = (((int32)(constants.GetScalarRegisterConstant(src2Index) << 26)) >> 26) & 0x3FUL;

    if(src2 & 0x20)
    {
      src1 = _lrotl(src1, 64UL - src2);
    }
    else
    {
      src1 = _lrotr(src1, src2);
    }

    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ANDImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ANDImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_FTSTImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    const uint32 destValue = src1 & src2;
    uint32 flagValues;
    if(!destValue)
    {
      flagValues = CC_ALU_ZERO;
    }
    else
    {
      flagValues = ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreMiscRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = CONSTANT_REG_DISCARD;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreMiscRegisterConstant(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}

void PropagateConstants_FTSTScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_FTSTImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_FTSTImmediate(constants);    
  }
  else if(constants.IsScalarRegisterConstant(src2Index))
  {
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_FTSTImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_ALU_SRC2] = src1Index;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    PropagateConstants_FTSTImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
  }
}

void PropagateConstants_FTSTImmediateShiftScalar(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
          uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = (((int32)(constants.GetScalarRegisterConstant(src2Index) << 26)) >> 26) & 0x3FUL;
    if(src2 & 0x20)
    {
      src1 = (src1 << (64UL - src2));
    }
    else
    {
      src1 = (src1 >> src2);
    }

    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_FTSTImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_FTSTImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}

void PropagateConstants_FTSTScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_FTSTImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 >> (constants.nuance->fields[FIELD_ALU_SRC2] & 0x3FUL);
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_FTSTImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
  }
}

void PropagateConstants_FTSTScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_FTSTImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 << (constants.nuance->fields[FIELD_ALU_SRC2] & 0x3FUL); // do not cast to u64 before shift!
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_FTSTImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
  }
}

void PropagateConstants_FTSTScalarShiftScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_FTSTImmediateShiftScalar;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_FTSTImmediateShiftScalar(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);
  }
}

void PropagateConstants_FTSTScalarRotateScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 
          uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    const uint32 src2 = (((int32)(constants.GetScalarRegisterConstant(src2Index) << 26)) >> 26) & 0x3FUL;

    if(src2 & 0x20)
    {
      src1 = _lrotl(src1, 64UL - src2);
    }
    else
    {
      src1 = _lrotr(src1, src2);
    }

    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_FTSTImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_FTSTImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}

void PropagateConstants_ORImmediate(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    const uint32 destValue = src1 | src2;
    uint32 flagValues;
    if(!destValue)
    {
      flagValues = CC_ALU_ZERO;
    }
    else
    {
      flagValues = ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ORScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_ORImmediate(constants);    
  }
  else if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_ALU_SRC2] = src1Index;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    PropagateConstants_ORImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ORImmediateShiftScalar(SuperBlockConstants &constants)
{
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
          uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = (((int32)(constants.GetScalarRegisterConstant(src2Index) << 26)) >> 26) & 0x3FUL;
    if(src2 & 0x20)
    {
      src1 = (src1 << (64UL - src2));
    }
    else
    {
      src1 = (src1 >> src2);
    }
    
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ORImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ORScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 >> (constants.nuance->fields[FIELD_ALU_SRC2] & 0x3FUL);
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ORImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ORScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 << (constants.nuance->fields[FIELD_ALU_SRC2] & 0x3FUL);
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ORImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ORScalarShiftScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST];

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ORImmediateShiftScalar;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ORImmediateShiftScalar(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ORScalarRotateScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
          uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    const uint32 src2 = (((int32)(constants.GetScalarRegisterConstant(src2Index) << 26)) >> 26) & 0x3FUL;

    if(src2 & 0x20)
    {
      src1 = _lrotl(src1, 64UL - src2);
    }
    else
    {
      src1 = _lrotr(src1, src2);
    }

    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ORImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_EORImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    const uint32 destValue = src1 ^ src2;
    uint32 flagValues;
    if(!destValue)
    {
      flagValues = CC_ALU_ZERO;
    }
    else
    {
      flagValues = ((destValue >> 28) & CC_ALU_NEGATIVE);
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_EORScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if((src1Index == src2Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = 0;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = CC_ALU_ZERO;
    constants.ClearScalarInputDependency(src2Index);
    constants.ClearScalarInputDependency(src1Index);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_EORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_EORImmediate(constants);    
  }
  else if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_EORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_ALU_SRC2] = src1Index;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    PropagateConstants_EORImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_EORImmediateShiftScalar(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
          uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = (((int32)(constants.GetScalarRegisterConstant(src2Index) << 26)) >> 26) & 0x3FUL;
    if(src2 & 0x20)
    {
      src1 = (src1 << (64UL - src2));
    }
    else
    {
      src1 = (src1 >> src2);
    }

    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_EORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_EORImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_EORScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_EORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 >> (constants.nuance->fields[FIELD_ALU_SRC2] & 0x3FUL);
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_EORImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_EORScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_EORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 << (constants.nuance->fields[FIELD_ALU_SRC2] & 0x3FUL);
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_EORImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_EORScalarShiftScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_EORImmediateShiftScalar;
    constants.nuance->fields[FIELD_ALU_SRC1] = constants.GetScalarRegisterConstant(src1Index);
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_EORImmediateShiftScalar(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_EORScalarRotateScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST];

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src1Index) && constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
          uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    const uint32 src2 = (((int32)(constants.GetScalarRegisterConstant(src2Index) << 26)) >> 26) & 0x3FUL;

    if(src2 & 0x20)
    {
      src1 = _lrotl(src1, 64UL - src2);
    }
    else
    {
      src1 = _lrotr(src1, src2);
    }

    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_EORImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_EORImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ADDWCImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  constants.SetMiscRegisterConstant(CONSTANT_REG_V,0);

  if(constants.IsScalarRegisterConstant(src2Index) && constants.IsMiscRegisterConstant(CONSTANT_REG_C) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    uint32 destValue;
    uint32 flagValues = _addcarry_u32(constants.GetMiscRegisterConstant(CONSTANT_REG_C), src1, src2, &destValue) ? CC_ALU_CARRY : 0;
    if((~(src1 ^ src2)) & (src1 ^ destValue) & 0x80000000u)
    {
      flagValues |= CC_ALU_OVERFLOW;
    }
    if(destValue & 0x80000000u)
    {
      flagValues |= CC_ALU_NEGATIVE;
    }
    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    constants.ClearMiscInputDependency(DEPENDENCY_FLAG_C);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ADDWCScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ADDWCImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_ADDWCImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ADDWCScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ADDWCImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = ((int32)src1) >> src2;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ADDWCImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_ADDWCScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_ADDWCImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 << src2; // do not cast to u64 before shift!
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_ADDWCImmediate(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}
void PropagateConstants_SUBWCImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src2Index) && constants.IsMiscRegisterConstant(CONSTANT_REG_C) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    uint32 destValue;
    uint32 flagValues = _subborrow_u32(constants.GetMiscRegisterConstant(CONSTANT_REG_C), src2, src1, &destValue) ? CC_ALU_CARRY : 0;
    if((src1 ^ src2) & (src2 ^ destValue) & 0x80000000u)
    {
      flagValues |= CC_ALU_OVERFLOW;
    }
    if(destValue & 0x80000000u)
    {
      flagValues |= CC_ALU_NEGATIVE;
    }
    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    constants.ClearMiscInputDependency(DEPENDENCY_FLAG_C);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_SUBWCImmediateReverse(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && constants.IsMiscRegisterConstant(CONSTANT_REG_C) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    uint32 destValue;
    uint32 flagValues = _subborrow_u32(constants.GetMiscRegisterConstant(CONSTANT_REG_C), src2, src1, &destValue) ? CC_ALU_CARRY : 0;
    if((src1 ^ src2) & (src2 ^ destValue) & 0x80000000u)
    {
      flagValues |= CC_ALU_OVERFLOW;
    }
    if(destValue & 0x80000000u)
    {
      flagValues |= CC_ALU_NEGATIVE;
    }
    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreScalarRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = destIndex;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src1Index);
    constants.ClearMiscInputDependency(DEPENDENCY_FLAG_C);
    PropagateConstants_StoreScalarRegisterConstant(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_SUBWCScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_SUBWCImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_SUBWCImmediate(constants);    
  }
  else if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_SUBWCImmediateReverse;
    constants.nuance->fields[FIELD_ALU_SRC2] = src2;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    PropagateConstants_SUBWCImmediateReverse(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_SUBWCScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_SUBWCImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = ((int32)src1) >> src2;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_SUBWCImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_SUBWCScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_SUBWCImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 << src2; // do not cast to u64 before shift!
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_SUBWCImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
    constants.ClearScalarRegisterConstant(destIndex);    
  }
}

void PropagateConstants_CMPWCImmediate(SuperBlockConstants &constants)
{
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];

  if(constants.IsScalarRegisterConstant(src2Index) && constants.IsMiscRegisterConstant(CONSTANT_REG_C) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.nuance->fields[FIELD_ALU_SRC1];
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    uint32 destValue;
    uint32 flagValues = _subborrow_u32(constants.GetMiscRegisterConstant(CONSTANT_REG_C), src2, src1, &destValue) ? CC_ALU_CARRY : 0;
    if((src1 ^ src2) & (src2 ^ destValue) & 0x80000000u)
    {
      flagValues |= CC_ALU_OVERFLOW;
    }
    if(destValue & 0x80000000u)
    {
      flagValues |= CC_ALU_NEGATIVE;
    }
    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreMiscRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = CONSTANT_REG_DISCARD;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src2Index);
    constants.ClearMiscInputDependency(DEPENDENCY_FLAG_C);
    PropagateConstants_StoreMiscRegisterConstant(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}

void PropagateConstants_CMPWCImmediateReverse(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];

  if(constants.IsScalarRegisterConstant(src1Index) && constants.IsMiscRegisterConstant(CONSTANT_REG_C) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    uint32 destValue;
    uint32 flagValues = _subborrow_u32(constants.GetMiscRegisterConstant(CONSTANT_REG_C), src2, src1, &destValue) ? CC_ALU_CARRY : 0;
    if((src1 ^ src2) & (src2 ^ destValue) & 0x80000000u)
    {
      flagValues |= CC_ALU_OVERFLOW;
    }
    if(destValue & 0x80000000u)
    {
      flagValues |= CC_ALU_NEGATIVE;
    }
    if(!destValue)
    {
      flagValues |= CC_ALU_ZERO;
    }
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_StoreMiscRegisterConstant;
    constants.nuance->fields[FIELD_CONSTANT_ADDRESS] = CONSTANT_REG_DISCARD;
    constants.nuance->fields[FIELD_CONSTANT_VALUE] = destValue;
    constants.nuance->fields[FIELD_CONSTANT_FLAGMASK] = CC_ALU_NEGATIVE | CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_CARRY;
    constants.nuance->fields[FIELD_CONSTANT_FLAGVALUES] = flagValues;
    constants.ClearScalarInputDependency(src1Index);
    constants.ClearMiscInputDependency(DEPENDENCY_FLAG_C);
    PropagateConstants_StoreMiscRegisterConstant(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}

void PropagateConstants_CMPWCScalar(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];
  const uint32 src2Index = constants.nuance->fields[FIELD_ALU_SRC2];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_CMPWCImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(src2Index);
    PropagateConstants_CMPWCImmediate(constants);    
  }
  else if(constants.IsScalarRegisterConstant(src2Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.GetScalarRegisterConstant(src2Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_CMPWCImmediateReverse;
    constants.nuance->fields[FIELD_ALU_SRC2] = src2;
    constants.ClearScalarInputDependency(src2Index);
    constants.SetScalarInputDependency(src1Index);
    PropagateConstants_CMPWCImmediateReverse(constants);    
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}

void PropagateConstants_CMPWCScalarShiftRightImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_CONSTANT_HANDLER] = Handler_CMPWCImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = ((int32)src1) >> src2;
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_CMPWCImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}

void PropagateConstants_CMPWCScalarShiftLeftImmediate(SuperBlockConstants &constants)
{
  const uint32 src1Index = constants.nuance->fields[FIELD_ALU_SRC1];

  if(constants.IsScalarRegisterConstant(src1Index) && ALLOW_ALU_PROPAGATION)
  {
    const uint32 src2 = constants.nuance->fields[FIELD_ALU_SRC2];
    const uint32 destIndex = constants.nuance->fields[FIELD_ALU_DEST]; 
    const uint32 src1 = constants.GetScalarRegisterConstant(src1Index);
    constants.nuance->fields[FIELD_ALU_HANDLER] = Handler_CMPWCImmediate;
    constants.nuance->fields[FIELD_ALU_SRC1] = src1 << src2; // do not cast to u64 before shift!
    constants.nuance->fields[FIELD_ALU_SRC2] = destIndex;
    constants.ClearScalarInputDependency(src1Index);
    constants.SetScalarInputDependency(destIndex);
    PropagateConstants_CMPWCImmediate(constants);
  }
  else
  {
    constants.status.status = PROPAGATE_CONSTANTS_STATUS_ALU_OK;
    constants.ClearMiscRegisterConstant(CONSTANT_REG_N);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_V);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_C);    
    constants.ClearMiscRegisterConstant(CONSTANT_REG_Z);    
  }
}
