#include "SuperBlockConstants.h"

void PropagateConstants_PacketStart(SuperBlockConstants &constants)
{
}

void PropagateConstants_PacketEnd(SuperBlockConstants &constants)
{
  constants.CommitConstants();
}

SuperBlockConstants::SuperBlockConstants(MPE *pMPE, SuperBlock *sBlock)
{
  pSuperBlock = sBlock;
  mpe = pMPE;
  ClearConstants();
  FirstInstruction();
}

SuperBlockConstants::~SuperBlockConstants()
{
}

bool SuperBlockConstants::EvaluateBranchCondition(uint32 whichCondition, bool *branchResult)
{
  bool bIsConstant = false;
  uint32 flag1, flag2;

  switch(whichCondition & 0x1FUL)
  {
    case 0:
      if(IsMiscRegisterConstant(CONSTANT_REG_Z))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_Z) == 0);
      }
      break;
    case 1:
      if(IsMiscRegisterConstant(CONSTANT_REG_C0Z))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_C0Z) != 0);
      }
      break;
    case 2:
      if(IsMiscRegisterConstant(CONSTANT_REG_C1Z))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_C1Z) != 0);
      }
      break;
    case 3:
      if(IsMiscRegisterConstant(CONSTANT_REG_C))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_C) == 0);
      }
      break;
    case 4:
      if(IsMiscRegisterConstant(CONSTANT_REG_Z))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_Z) != 0);
      }
      break;
    case 5:
      if(IsMiscRegisterConstant(CONSTANT_REG_C))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_C) != 0);
      }
      break;
    case 6:
      if(IsMiscRegisterConstant(CONSTANT_REG_V))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_V) == 0);
      }
      break;
    case 7:
      if(IsMiscRegisterConstant(CONSTANT_REG_V))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_V) != 0);
      }
      break;
    case 8:
      //N.~V + ~N.V: both N and V must be constant
      if(IsMiscRegisterConstant(CONSTANT_REG_N) && IsMiscRegisterConstant(CONSTANT_REG_V))
      {
        bIsConstant = true;
        flag1 = GetMiscRegisterConstant(CONSTANT_REG_N);
        flag2 = GetMiscRegisterConstant(CONSTANT_REG_V);
        *branchResult = ((flag1 ^ flag2) != 0);
      }
      break;
    case 9:
      if(IsMiscRegisterConstant(CONSTANT_REG_MV))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_MV) == 0);
      }
      break;
    case 10:
      if(IsMiscRegisterConstant(CONSTANT_REG_MV))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_MV) != 0);
      }
      break;
    case 11:
      //!(C + Z): if either C and Z is constant and true, the condition is constant and the branch is not taken,
      //otherwise both C and Z must be constant
      if(IsMiscRegisterConstant(CONSTANT_REG_C))
      {
        flag1 = GetMiscRegisterConstant(CONSTANT_REG_C);
        if(flag1 != 0)
        {
          bIsConstant = true;
          *branchResult = false;
        }
        else
        {
          if(IsMiscRegisterConstant(CONSTANT_REG_Z))
          {
            if(GetMiscRegisterConstant(CONSTANT_REG_Z))
            {
              bIsConstant = true;
              *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_Z) == 0);
            }
          }
        }
      }
      else if(IsMiscRegisterConstant(CONSTANT_REG_Z))
      {
        flag1 = GetMiscRegisterConstant(CONSTANT_REG_Z);
        if(flag1 != 0)
        {
          bIsConstant = true;
          *branchResult = false;
        }
      }
      break;
    case 12:
      //Z + N.~V + ~N.V: if Z is constant and true, the condition is constant and the branch is taken,
      //otherwise both N and V must be constant
      if(IsMiscRegisterConstant(CONSTANT_REG_N) && IsMiscRegisterConstant(CONSTANT_REG_V))
      {
        bIsConstant = true;
        flag1 = GetMiscRegisterConstant(CONSTANT_REG_N);
        flag2 = GetMiscRegisterConstant(CONSTANT_REG_V);
        //N(~V) || (~N)V
        *branchResult = ((flag1 ^ flag2) != 0);
      }
      else if(IsMiscRegisterConstant(CONSTANT_REG_Z))
      {
        if(GetMiscRegisterConstant(CONSTANT_REG_Z))
        {
          bIsConstant = true;
          *branchResult = true;
        }
      }
      break;
    case 13:
      //C + Z: if either of C or Z is constant and true, the condition is constant and the branch is taken,
      //otherwise both flags must be constant
      if(IsMiscRegisterConstant(CONSTANT_REG_C))
      {
        if(GetMiscRegisterConstant(CONSTANT_REG_C))
        {
          bIsConstant = true;
          *branchResult = true;
        }
        else if(IsMiscRegisterConstant(CONSTANT_REG_Z))
        {
          bIsConstant = true;
          *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_Z) != 0);
        }
      }
      else if(IsMiscRegisterConstant(CONSTANT_REG_Z))
      {
        if(GetMiscRegisterConstant(CONSTANT_REG_Z))
        {
          bIsConstant = true;
          *branchResult = true;
        }
      }
      break;
    case 14:
      if(IsMiscRegisterConstant(CONSTANT_REG_N))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_N) == 0);
      }
      break;
    case 15:
      if(IsMiscRegisterConstant(CONSTANT_REG_N))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_N) != 0);
      }
      break;
    case 16:
      //~Z.(NV + ~N.~V)
      //If Z is constant and true, the condition is constant and the branch is not taken,
      //otherwise N, Z and V must all be constant
      if(IsMiscRegisterConstant(CONSTANT_REG_Z))
      {
        if(GetMiscRegisterConstant(CONSTANT_REG_Z) != 0)
        {
          bIsConstant = true;
          *branchResult = false;
          break;
        }
        else if(IsMiscRegisterConstant(CONSTANT_REG_N) && IsMiscRegisterConstant(CONSTANT_REG_V))
        {
          bIsConstant = true;
          flag1 = GetMiscRegisterConstant(CONSTANT_REG_N);
          flag2 = GetMiscRegisterConstant(CONSTANT_REG_V);
          //(~N)(~V) || NV
          *branchResult = ((flag1 ^ flag2) == 0);
        }
      }
      break;
    case 17:
      //T
      bIsConstant = true;
      *branchResult = true;
      break;
    case 18:
      if(IsMiscRegisterConstant(CONSTANT_REG_MODMI))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_MODMI) != 0);
      }
      break;
    case 19:
      if(IsMiscRegisterConstant(CONSTANT_REG_MODMI))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_MODMI) == 0);
      }
      break;
    case 20:
      //Both N and V must be constant
      if(IsMiscRegisterConstant(CONSTANT_REG_N) && IsMiscRegisterConstant(CONSTANT_REG_V))
      {
        bIsConstant = true;
        flag1 = GetMiscRegisterConstant(CONSTANT_REG_N);
        flag2 = GetMiscRegisterConstant(CONSTANT_REG_V);
        //(~N)(~V) || NV
        *branchResult = ((flag1 ^ flag2) == 0);
      }
    case 21:
      if(IsMiscRegisterConstant(CONSTANT_REG_MODGE))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_MODGE) != 0);
      }
      break;
    case 22:
      if(IsMiscRegisterConstant(CONSTANT_REG_MODGE))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_MODGE) == 0);
      }
      break;
    case 23:
       //The condition is always constant and the branch is never taken
       bIsConstant = true;
       *branchResult = false;
       break;
    case 24:
      if(IsMiscRegisterConstant(CONSTANT_REG_C0Z))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_C0Z) == 0);
      }
      break;
    case 25:
       bIsConstant = true;
       *branchResult = false;
       break;
    case 26:
       bIsConstant = true;
       *branchResult = false;
       break;
    case 27:
      if(IsMiscRegisterConstant(CONSTANT_REG_CP0))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_CP0) == 0);
      }
      break;
    case 28:
      if(IsMiscRegisterConstant(CONSTANT_REG_C1Z))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_C1Z) == 0);
      }
      break;
    case 29:
      if(IsMiscRegisterConstant(CONSTANT_REG_CP0))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_CP0) != 0);
      }
      break;
    case 30:
      if(IsMiscRegisterConstant(CONSTANT_REG_CP1))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_CP1) == 0);
      }
      break;
    case 31:
      if(IsMiscRegisterConstant(CONSTANT_REG_CP1))
      {
        bIsConstant = true;
        *branchResult = (GetMiscRegisterConstant(CONSTANT_REG_CP1) != 0);
      }
      break;
  }

  return bIsConstant;
}
