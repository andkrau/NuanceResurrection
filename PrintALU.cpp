#include "basetypes.h"
#include <cstdio>
#include <cstring>
#include "InstructionCache.h"

static uint32 GetBtstSrc1(const uint32 src1)
{
  for(uint32 i = 0; i < 32; i++)
  {
    if(src1 == (1UL << i))
    {
      return i;
    }
  }

  return src1;
}

uint32 Print_ABS(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "abs r%lu%s",nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_BITSScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "bits #%lu, >>r%lu, r%lu%s",nuance.fields[FIELD_ALU_INFO],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_BITSImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "bits #%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_INFO],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_BTST(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "btst #%lu, r%lu%s",GetBtstSrc1(nuance.fields[FIELD_ALU_SRC1]),nuance.fields[FIELD_ALU_SRC2], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_BUTT(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "butt r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_COPY(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "copy r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_MSB(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "msb r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SAT(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "sat #%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC2] + 1,nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_AS(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "as >>r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ASL(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "asl #%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ASR(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "asr #%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LS(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ls >>r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_LSR(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "lsr #%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ROT(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "rot <>r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ROL(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "rot #-%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ROR(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "rot #%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}
uint32 Print_ADD_P(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "add_p v%lu, v%lu, v%lu%s",nuance.fields[FIELD_ALU_SRC1] >> 2,nuance.fields[FIELD_ALU_SRC2] >> 2,nuance.fields[FIELD_ALU_DEST] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUB_P(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "sub_p v%lu, v%lu, v%lu%s",nuance.fields[FIELD_ALU_SRC1] >> 2,nuance.fields[FIELD_ALU_SRC2] >> 2,nuance.fields[FIELD_ALU_DEST] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ADD_SV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "add_sv v%lu, v%lu, v%lu%s",nuance.fields[FIELD_ALU_SRC1] >> 2,nuance.fields[FIELD_ALU_SRC2] >> 2,nuance.fields[FIELD_ALU_DEST] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUB_SV(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "sub_sv v%lu, v%lu, v%lu%s",nuance.fields[FIELD_ALU_SRC1] >> 2,nuance.fields[FIELD_ALU_SRC2] >> 2,nuance.fields[FIELD_ALU_DEST] >> 2, bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ADDImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "add #$%8.8lX, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ADDScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "add r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ADDScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "add r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ADDScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "add r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUBImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "sub #$%8.8lX, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}
uint32 Print_SUBImmediateReverse(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "sub r%lu, #$%8.8lX, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUBScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "sub r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUBScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "sub r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUBScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "sub r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_CMPImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "cmp #$%8.8lX, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_CMPImmediateReverse(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "cmp r%lu, #$%8.8lX%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_CMPScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "cmp r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_CMPScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "cmp r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_CMPScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "cmp r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ANDImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "and #$%8.8lX, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ANDScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "and r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ANDImmediateShiftScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "and #$%8.8lX, >>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ANDScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "and r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ANDScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "and r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ANDScalarShiftScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "and r%lu, >>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ANDScalarRotateScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "and r%lu, <>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_FTSTImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ftst #$%8.8lX, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_FTSTScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ftst r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_FTSTImmediateShiftScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ftst #$%8.8lX, >>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_FTSTScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ftst r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_FTSTScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ftst r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}
uint32 Print_FTSTScalarShiftScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ftst r%lu, >>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}
uint32 Print_FTSTScalarRotateScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "ftst r%lu, <>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ORImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "or #$%8.8lX, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ORScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "or r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ORImmediateShiftScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "or #$%8.8lX, >>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ORScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "or r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ORScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "or r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ORScalarShiftScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "or r%lu, >>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ORScalarRotateScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "or r%lu, <>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_EORImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "eor #$%8.8lX, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_EORScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "eor r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_EORImmediateShiftScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "eor #$%8.8lX, >>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_EORScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "eor r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_EORScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "eor r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_EORScalarShiftScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "eor r%lu, >>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_EORScalarRotateScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "eor r%lu, <>r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ADDWCImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "addwc #$%8.8lX, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ADDWCScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "addwc r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ADDWCScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "addwc r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_ADDWCScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "addwc r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUBWCImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "subwc #$%8.8lX, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUBWCImmediateReverse(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "subwc r%lu, #$%8.8lX, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUBWCScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "subwc r%lu, r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUBWCScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "subwc r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_SUBWCScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "subwc r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_CMPWCImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "cmpwc #$%8.8lX, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_CMPWCImmediateReverse(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "cmpwc r%lu, #$%8.8lX%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_CMPWCScalar(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "cmpwc r%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_CMPWCScalarShiftRightImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "cmpwc r%lu, >>#%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}

uint32 Print_CMPWCScalarShiftLeftImmediate(char *buffer, size_t bufSize, const Nuance &nuance, const bool bNewline)
{
  const uint32 length = sprintf_s(buffer, bufSize, "cmpwc r%lu, >>#-%lu, r%lu%s",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST], bNewline ? "\n" : "" );
  return length;
}
