#include "basetypes.h"
#include <cstdio>
#include <cstring>
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

uint32 Print_ECU_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ecu_nop%s", bNewline ? "\n" : "");
  return length;
}

uint32 Print_Halt(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "halt%s", bNewline ? "\n" : "");
  return length;
}

uint32 Print_BRAAlways(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"bra $%8.8lX%s",nuance.fields[FIELD_ECU_ADDRESS], bNewline ? "\n" : "");
  return length;
}

uint32 Print_BRAAlways_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"bra $%8.8lX, nop%s",nuance.fields[FIELD_ECU_ADDRESS], bNewline ? "\n" : "");
  return length;
}

uint32 Print_BRAConditional(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"bra %s, $%8.8lX%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS], bNewline ? "\n" : "");
  return length;
}

uint32 Print_BRAConditional_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"bra %s, $%8.8lX, nop%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JMPAlwaysIndirect(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jmp (r%lu)%s",nuance.fields[FIELD_ECU_ADDRESS], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JMPAlwaysIndirect_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jmp (r%lu), nop%s",nuance.fields[FIELD_ECU_ADDRESS], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JMPConditionalIndirect(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jmp %s, (r%lu)%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JMPConditionalIndirect_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jmp %s, (r%lu), nop%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JSRAlways(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jsr $%8.8lX [$%8.8lX,$%8.8lX]%s",nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JSRAlways_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jsr $%8.8lX, nop [$%8.8lX,$%8.8lX]%s",nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JSRConditional(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jsr %s, $%8.8lX [$%8.8lX,$%8.8lX]%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JSRConditional_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jsr %s, $%8.8lX, nop [$%8.8lX,$%8.8lX]%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JSRAlwaysIndirect(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jsr (r%lu) [$%8.8lX,$%8.8lX]%s",nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JSRAlwaysIndirect_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jsr (r%lu), nop [$%8.8lX,$%8.8lX]%s",nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JSRConditionalIndirect(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jsr %s, (r%lu) [$%8.8lX,$%8.8lX]%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT], bNewline ? "\n" : "");
  return length;
}

uint32 Print_JSRConditionalIndirect_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"jsr %s, (r%lu), nop [$%8.8lX,$%8.8lX]%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]),nuance.fields[FIELD_ECU_ADDRESS],nuance.fields[FIELD_ECU_PCROUTE],nuance.fields[FIELD_ECU_PCFETCHNEXT], bNewline ? "\n" : "");
  return length;
}

uint32 Print_RTSAlways(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "rts%s", bNewline ? "\n" : "");
  return length;
}

uint32 Print_RTSAlways_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer,512, "rts, nop%s", bNewline ? "\n" : "");
  return length;
}

uint32 Print_RTSConditional(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"rts %s%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]), bNewline ? "\n" : "");
  return length;
}

uint32 Print_RTSConditional_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"rts %s, nop%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]), bNewline ? "\n" : "");
  return length;
}

uint32 Print_RTI1Conditional(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"rti1 %s%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]), bNewline ? "\n" : "");
  return length;
}

uint32 Print_RTI1Conditional_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"rti1 %s, nop%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]), bNewline ? "\n" : "");
  return length;
}

uint32 Print_RTI2Conditional(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"rti2 %s%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]), bNewline ? "\n" : "");
  return length;
}

uint32 Print_RTI2Conditional_NOP(char *buffer, size_t bufSize,  const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize,"rti2 %s, nop%s",GetConditionCode(nuance.fields[FIELD_ECU_CONDITION]), bNewline ? "\n" : "");
  return length;
}
