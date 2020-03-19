#include "basetypes.h"
#include <stdio.h>
#include <string.h>
#include "InstructionCache.h"

const char *GetConditionCode(uint32 which)
{
  switch(which)
  {
    case 0:
      return "ne";
    case 1:
      return "c0eq";
    case 2:
      return "c1eq";
    case 3:
      return "cc";
    case 4:
      return "eq ";
    case 5:
      return "cs";
    case 6:
      return "vc";
    case 7:
      return "vs";
    case 8:
      return "lt";
    case 9:
      return "mvc";
    case 10:
      return "mvs";
    case 11:
      return "hi";
    case 12:
      return "le";
    case 13:
      return "ls";
    case 14:
      return "pl";
    case 15:
      return "mi";
    case 16:
      return "gt";
    case 17:
      return "t";
    case 18:
      return "modmi";
    case 19:
      return "modpl";
    case 20:
      return "ge";
    case 21:
      return "modge";
    case 22:
      return "modlt";
    case 23:
      return "???";
    case 24:
      return "c0ne";
    case 25:
      return "???";
    case 26:
      return "???";
    case 27:
      return "cf0lo";
    case 28:
      return "c1ne";
    case 29:
      return "cf0hi";
    case 30:
      return "cf1lo";
    case 31:
      return "cf1hi";
    default:
      return "???";
  }
}

uint32 Print_ECU_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[] = "ecu_nop";
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_Halt(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[] = "halt";
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_BRAAlways(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"bra $%8.8lX",nuance.fields[FIELD_ECU_ADDRESS]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_BRAAlways_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"bra $%8.8lX, nop",nuance.fields[FIELD_ECU_ADDRESS]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_BRAConditional(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"bra %s, $%8.8lX",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_BRAConditional_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"bra %s, $%8.8lX, nop",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JMPAlwaysIndirect(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jmp (r%lu)",nuance.fields[FIELD_ECU_ADDRESS]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JMPAlwaysIndirect_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jmp (r%lu), nop",nuance.fields[FIELD_ECU_ADDRESS]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JMPConditionalIndirect(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jmp %s, (r%lu)",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JMPConditionalIndirect_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jmp %s, (r%lu), nop",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JSRAlways(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jsr $%8.8lX [$%8.8lX,$%8.8lX]",nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JSRAlways_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jsr $%8.8lX, nop [$%8.8lX,$%8.8lX]",nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JSRConditional(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jsr %s, $%8.8lX [$%8.8lX,$%8.8lX]",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JSRConditional_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jsr %s, $%8.8lX, nop [$%8.8lX,$%8.8lX]",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JSRAlwaysIndirect(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jsr (r%lu) [$%8.8lX,$%8.8lX]",nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JSRAlwaysIndirect_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jsr (r%lu), nop [$%8.8lX,$%8.8lX]",nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JSRConditionalIndirect(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jsr %s, (r%lu) [$%8.8lX,$%8.8lX]",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_JSRConditionalIndirect_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"jsr %s, (r%lu), nop [$%8.8lX,$%8.8lX]",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_RTSAlways(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[] = "rts";
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_RTSAlways_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[] = "rts, nop";
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_RTSConditional(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"rts %s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]));
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_RTSConditional_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"rts %s, nop",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]));
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_RTI1Conditional(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"rti1 %s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]));
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_RTI1Conditional_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"rti1 %s, nop",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]));
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_RTI2Conditional(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"rti2 %s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]));
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_RTI2Conditional_NOP(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"rti2 %s, nop",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]));
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}
