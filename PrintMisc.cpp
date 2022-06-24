#include "basetypes.h"
#include <cstdio>
#include <cstring>
#include "InstructionCache.h"

uint32 Print_PacketStart(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "PacketStarts%s", bNewline ? "\n" : "");
  return length;
}

uint32 Print_PacketEnd(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "PacketEnd%s", bNewline ? "\n" : "");
  return length;
}

uint32 Print_CheckECUSkipCounter(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "CheckECUSkipCounter%s", bNewline ? "\n" : "");
  return length;
}

uint32 Print_SaveFlags(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "SaveFlags%s", bNewline ? "\n" : "");
  return length;
}

uint32 Print_SaveRegs(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "SaveRegs%s", bNewline ? "\n" : "");
  return length;
}
