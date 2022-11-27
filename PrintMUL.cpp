#include "basetypes.h"
#include <cstdio>
#include <cstring>
#include "InstructionCache.h"

static const char *GetImmediateShift(const uint32 which)
{
  switch(which)
  {
    case 0:
      return "16";
    case 1:
      return "24";
    case 2:
      return "32";
    case 3:
      return "30";
    default:
      return "???";
  }
}

uint32 Print_ADDMImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "addm #$%lX, r%lu, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_ADDM(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "addm r%lu, r%lu, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_SUBMImmediateReverse(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "subm r%lu, #$%lX, r%lu%s",nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_SUBM(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "subm r%lu, r%lu, r%lu%s",nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MULScalarShiftAcshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul r%lu, r%lu, >>acshift, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MULScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul r%lu, r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MULScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul r%lu, r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MULImmediateShiftAcshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul #%lu, r%lu, >>acshift, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MULScalarShiftScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul r%lu, r%lu, >>r%lu, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MULImmediateShiftScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul #%lu, r%lu, >>r%lu, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MULImmediateShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul #%lu, r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MULImmediateShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul #%lu, r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_SVImmediateShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_sv #$%lX, v%lu, >>#%s, v%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_SVScalarShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_sv r%lu, v%lu, >>#%s, v%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_SVScalarShiftSvshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_sv r%lu, v%lu, >>svshift, v%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_SVRuShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_sv ru, v%lu, >>#%s, v%lu%s",nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_SVRuShiftSvshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_sv ru, v%lu, >>svshift, v%lu%s",nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_SVRvShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_sv rv, v%lu, >>#%s, v%lu%s",nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_SVRvShiftSvshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_sv rv, v%lu, >>svshift, v%lu%s",nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_SVVectorShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_sv v%lu, v%lu, >>#%s, v%lu%s",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_SVVectorShiftSvshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_sv v%lu, v%lu, >>svshift, v%lu%s",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_PImmediateShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_p #$%lX, v%lu, >>#%s, v%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_PScalarShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_p r%lu, v%lu, >>#%s, v%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_PScalarShiftSvshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_p r%lu, v%lu, >>svshift, v%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_PRuShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_p ru, v%lu, >>#%s, v%lu%s",nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_PRuShiftSvshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_p ru, v%lu, >>svshift, v%lu%s",nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_PRvShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_p rv, v%lu, >>#%s, v%lu%s",nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_PRvShiftSvshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_p rv, v%lu, >>svshift, v%lu%s",nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_PVectorShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_p v%lu, v%lu, >>#%s, v%lu%s",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_MUL_PVectorShiftSvshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "mul_p v%lu, v%lu, >>svshift, v%lu%s",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2, bNewline ? "\n" : "" );

  return length;
}

uint32 Print_DOTPScalarShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "dotp r%lu, v%lu, >>#%s, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_DOTPScalarShiftSvshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "dotp r%lu, v%lu, >>svshift, r%lu%s",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_DOTPVectorShiftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "dotp v%lu, v%lu, >>#%s, r%lu%s",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}

uint32 Print_DOTPVectorShiftSvshift(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length
  = sprintf_s(buffer, bufSize, "dotp v%lu, v%lu, >>svshift, r%lu%s",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST], bNewline ? "\n" : "" );

  return length;
}
