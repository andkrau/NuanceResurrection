#include "basetypes.h"
#include <stdio.h>
#include <string.h>
#include "InstructionCache.h"

const char *GetIndexRegister(uint32 which)
{
  switch(which & 0x3)
  {
    default: //supress warning
    case 0:
      return "rx";
    case 1:
      return "ry";
    case 2:
      return "ru";
    case 3:
      return "rv";
  }
}

uint32 Print_DEC(char *buffer, const Nuance &nuance, const bool bNewline)
{
  uint32 bufinc = 0;

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC0)
  {
    if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
    {
      sprintf(buffer,"dec rc0\ndec rc1%s",bNewline ? "\n" : "");
      constexpr char decstr1[] = "dec rc0\ndec rc1";
      bufinc += (strlen(decstr1) + (bNewline ? 1 : 0));
    }
    else
    {
      sprintf(buffer,"dec rc0%s",bNewline ? "\n" : "");
      constexpr char decstr0[] = "dec rc0\n";
      bufinc += (strlen(decstr0) + (bNewline ? 1 : 0));
    }
  }
  else
  {
    if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
    {
      sprintf(buffer,"dec rc1%s",bNewline ? "\n" : "");
      constexpr char decstr1[] = "dec rc1\n";
      bufinc += (strlen(decstr1) + (bNewline ? 1 : 0));
    }
    else
    {
      if(bNewline)
      {
        sprintf(buffer,"%s",bNewline ? "\n" : "");
        bufinc += (bNewline ? 1 : 0);
      }
    }
  }

  return bufinc;
}

uint32 Print_ADDRImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  uint32 bufinc;

  sprintf(tempStr,"addr #$%8.8lX, %s\n",nuance.fields[FIELD_RCU_SRC],GetIndexRegister(nuance.fields[FIELD_RCU_DEST]));
  sprintf(buffer,"%s",tempStr);
  bufinc = strlen(tempStr);
  buffer += bufinc;
  return bufinc + Print_DEC(buffer,nuance,bNewline);
}

uint32 Print_ADDRScalar(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  uint32 bufinc;

  sprintf(tempStr,"addr r%lu, %s\n",nuance.fields[FIELD_RCU_SRC],GetIndexRegister(nuance.fields[FIELD_RCU_DEST]));
  sprintf(buffer,"%s",tempStr);
  bufinc = strlen(tempStr);
  buffer += bufinc;
  return bufinc + Print_DEC(buffer,nuance,bNewline);
}

uint32 Print_MVRImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  uint32 bufinc;

  sprintf(tempStr,"mvr #$%8.8lX, %s\n",nuance.fields[FIELD_RCU_SRC],GetIndexRegister(nuance.fields[FIELD_RCU_DEST]));
  sprintf(buffer,"%s",tempStr);
  bufinc = strlen(tempStr);
  buffer += bufinc;
  return bufinc + Print_DEC(buffer,nuance,bNewline);
}

uint32 Print_MVRScalar(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  uint32 bufinc;

  sprintf(tempStr,"mvr r%lu, %s\n",nuance.fields[FIELD_RCU_SRC],GetIndexRegister(nuance.fields[FIELD_RCU_DEST]));
  sprintf(buffer,"%s",tempStr);
  bufinc = strlen(tempStr);
  buffer += bufinc;
  return bufinc + Print_DEC(buffer,nuance,bNewline);
}

uint32 Print_Range(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  uint32 bufinc;

  sprintf(tempStr,"range %s\n",GetIndexRegister(nuance.fields[FIELD_RCU_SRC]));
  sprintf(buffer,"%s",tempStr);
  bufinc = strlen(tempStr);
  buffer += bufinc;
  return bufinc + Print_DEC(buffer,nuance,bNewline);
}

uint32 Print_Modulo(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  uint32 bufinc;

  sprintf(tempStr,"modulo %s\n",GetIndexRegister(nuance.fields[FIELD_RCU_SRC]));
  sprintf(buffer,"%s",tempStr);
  bufinc = strlen(tempStr);
  buffer += bufinc;
  return bufinc + Print_DEC(buffer,nuance,bNewline);
}
