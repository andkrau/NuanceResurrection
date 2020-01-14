#include "mpe.h"

bool MPE::TestConditionCode(uint32 whichCondition)
{
  //This sequencing is correct for 32/64 bit ECU instructions.  The decode
  //handler for 16/48 bit ECU instructions converts the extracted condition
  //to this sequence.  VMLabs must die.  Mission accomplished!

  switch(whichCondition & 0x1FUL)
  {
    case 0:
      //ne
      return (tempCC & CC_ALU_ZERO) == 0;
    case 1:
      //c0z
      return (tempCC & CC_COUNTER0_ZERO);
    case 2:
      //c1z
      return (tempCC & CC_COUNTER1_ZERO);
    case 3:
      //cc
      return (tempCC & CC_ALU_CARRY) == 0;
    case 4:
      //eq
      return (tempCC & CC_ALU_ZERO);
    case 5:
      //cs
      return (tempCC & CC_ALU_CARRY);
    case 6:
      //vc
      return (tempCC & CC_ALU_OVERFLOW) == 0;
    case 7:
      //vs
      return (tempCC & CC_ALU_OVERFLOW);
    case 8:
      //lt
      return ((tempCC & (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW)) == CC_ALU_NEGATIVE) ||
        ((tempCC & (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW)) == CC_ALU_OVERFLOW);
    case 9:
      //mvc
      return (tempCC & CC_MUL_OVERFLOW) == 0;
    case 10:
      //mvs
      return (tempCC & CC_MUL_OVERFLOW);
    case 11:
      //hi
      return (tempCC & (CC_ALU_CARRY | CC_ALU_ZERO)) == 0;
    case 12:
      //le
      return (tempCC & CC_ALU_ZERO) || ((tempCC & (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW)) == CC_ALU_NEGATIVE) ||
        ((tempCC & (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW)) == CC_ALU_OVERFLOW);
    case 13:
      //ls
      return (tempCC & (CC_ALU_CARRY | CC_ALU_ZERO));
    case 14:
      //pl
      return (tempCC & CC_ALU_NEGATIVE) == 0;
    case 15:
      //mi
      return (tempCC & CC_ALU_NEGATIVE);
    case 16:
      //gt
      return ((tempCC & (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_ZERO)) == (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW)) ||
        ((tempCC & (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_ZERO)) == 0);
    case 17:
      //always
      return true;
    case 18:
      //modmi
      return (tempCC & CC_MODMI);
    case 19:
      //modpl
      return (tempCC & CC_MODMI) == 0;
    case 20:
      //ge
      return ((tempCC & (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW)) == (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW)) ||
        ((tempCC & (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW)) == 0);
    case 21:
      //modge
      return (tempCC & CC_MODGE);
    case 22:
      //modlt
      return (tempCC & CC_MODGE) == 0;
    case 23:
      //never
      return false;
    case 24:
      //c0ne
      return (tempCC & CC_COUNTER0_ZERO) == 0;
    case 25:
      //never
      return false;
    case 26:
      //never
      return false;
    case 27:
      //cf0lo
      return (tempCC & CC_COPROCESSOR0) == 0;
    case 28:
      //c1ne
      return (tempCC & CC_COUNTER1_ZERO) == 0;
    case 29:
      //cf0hi
      return (tempCC & CC_COPROCESSOR0);
    case 30:
      //cf1lo
      return (tempCC & CC_COPROCESSOR1) == 0;
    case 31:
      //cf1hi
      return (tempCC & CC_COPROCESSOR1);
  }

  return false;
}