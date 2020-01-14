#include "basetypes.h"
#include "InstructionCache.h"
#include <stdio.h>
#include <string.h>

uint32 Print_PacketStart(char *buffer, Nuance &nuance, bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"PacketStart");
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_PacketEnd(char *buffer, Nuance &nuance, bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"PacketEnd");
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CheckECUSkipCounter(char *buffer, Nuance &nuance, bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"CheckECUSkipCounter");
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SaveFlags(char *buffer, Nuance &nuance, bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"SaveFlags");
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SaveRegs(char *buffer, Nuance &nuance, bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"SaveRegs");
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}
