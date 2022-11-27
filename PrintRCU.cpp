#include "basetypes.h"
#include <cstdio>
#include <cstring>
#include "InstructionCache.h"

static const char *GetIndexRegister(const uint32 which)
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

uint32 Print_DEC(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  uint32 bufinc = 0;

  if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC0)
  {
    if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
    {
      bufinc = sprintf_s(buffer, bufSize, "dec rc0\ndec rc1%s",bNewline ? "\n" : "");
    }
    else
    {
      bufinc = sprintf_s(buffer, bufSize, "dec rc0%s",bNewline ? "\n" : "");
    }
  }
  else
  {
    if(nuance.fields[FIELD_RCU_INFO] & RCU_DEC_RC1)
    {
      bufinc = sprintf_s(buffer, bufSize, "dec rc1%s",bNewline ? "\n" : "");
    }
    else
    {
      bufinc = sprintf_s(buffer, bufSize, "%s",bNewline ? "\n" : "");
    }
  }

  return bufinc;
}

uint32 Print_ADDRImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 bufinc = sprintf_s(buffer, bufSize,"addr #$%8.8lX, %s\n",nuance.fields[FIELD_RCU_SRC],GetIndexRegister(nuance.fields[FIELD_RCU_DEST]));

  buffer += bufinc;
  bufSize -= bufinc;
  return bufinc + Print_DEC(buffer, bufSize, nuance,bNewline);
}

uint32 Print_ADDRScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 bufinc = sprintf_s(buffer, bufSize, "addr r%lu, %s\n",nuance.fields[FIELD_RCU_SRC],GetIndexRegister(nuance.fields[FIELD_RCU_DEST]));

  buffer += bufinc;
  bufSize -= bufinc;
  return bufinc + Print_DEC(buffer, bufSize, nuance,bNewline);
}

uint32 Print_MVRImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 bufinc = sprintf_s(buffer, bufSize, "mvr #$%8.8lX, %s\n",nuance.fields[FIELD_RCU_SRC],GetIndexRegister(nuance.fields[FIELD_RCU_DEST]));

  buffer += bufinc;
  bufSize -= bufinc;
  return bufinc + Print_DEC(buffer, bufSize, nuance,bNewline);
}

uint32 Print_MVRScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 bufinc = sprintf_s(buffer, bufSize, "mvr r%lu, %s\n",nuance.fields[FIELD_RCU_SRC],GetIndexRegister(nuance.fields[FIELD_RCU_DEST]));

  buffer += bufinc;
  bufSize -= bufinc;
  return bufinc + Print_DEC(buffer, bufSize, nuance,bNewline);
}

uint32 Print_Range(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 bufinc = sprintf_s(buffer, bufSize, "range %s\n",GetIndexRegister(nuance.fields[FIELD_RCU_SRC]));

  buffer += bufinc;
  bufSize -= bufinc;
  return bufinc + Print_DEC(buffer, bufSize, nuance,bNewline);
}

uint32 Print_Modulo(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 bufinc = sprintf_s(buffer, bufSize, "modulo %s\n",GetIndexRegister(nuance.fields[FIELD_RCU_SRC]));

  buffer += bufinc;
  bufSize -= bufinc;
  return bufinc + Print_DEC(buffer, bufSize, nuance,bNewline);
}
