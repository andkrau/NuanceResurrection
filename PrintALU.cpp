#include "basetypes.h"
#include <stdio.h>
#include <string.h>
#include "InstructionCache.h"

uint32 GetBtstSrc1(const uint32 src1)
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

uint32 Print_ABS(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"abs r%lu",nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_BITSScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"bits #%lu, >>r%lu, r%lu",nuance.fields[FIELD_ALU_INFO],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_BITSImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"bits #%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_INFO],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_BTST(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"btst #%lu, r%lu",GetBtstSrc1(nuance.fields[FIELD_ALU_SRC1]),nuance.fields[FIELD_ALU_SRC2]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_BUTT(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"butt r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_COPY(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"copy r%lu, r%lu",nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MSB(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"msb r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SAT(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"sat #%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC2] + 1,nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_AS(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"as >>r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ASL(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"asl #%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ASR(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"asr #%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_LS(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"ls >>r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_LSR(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"lsr #%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ROT(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"rot <>r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ROL(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"rot #-%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ROR(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"rot #%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}
uint32 Print_ADD_P(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"add_p v%lu, v%lu, v%lu",nuance.fields[FIELD_ALU_SRC1] >> 2,nuance.fields[FIELD_ALU_SRC2] >> 2,nuance.fields[FIELD_ALU_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUB_P(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"sub_p v%lu, v%lu, v%lu",nuance.fields[FIELD_ALU_SRC1] >> 2,nuance.fields[FIELD_ALU_SRC2] >> 2,nuance.fields[FIELD_ALU_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ADD_SV(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"add_sv v%lu, v%lu, v%lu",nuance.fields[FIELD_ALU_SRC1] >> 2,nuance.fields[FIELD_ALU_SRC2] >> 2,nuance.fields[FIELD_ALU_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUB_SV(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"sub_sv v%lu, v%lu, v%lu",nuance.fields[FIELD_ALU_SRC1] >> 2,nuance.fields[FIELD_ALU_SRC2] >> 2,nuance.fields[FIELD_ALU_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ADDImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"add #$%8.8lX, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ADDScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"add r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ADDScalarShiftRightImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"add r%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ADDScalarShiftLeftImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"add r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"sub #$%8.8lX, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}
uint32 Print_SUBImmediateReverse(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"sub r%lu, #$%8.8lX, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"sub r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBScalarShiftRightImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"sub r%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBScalarShiftLeftImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"sub r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CMPImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"cmp #$%8.8lX, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CMPImmediateReverse(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"cmp r%lu, #$%8.8lX",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CMPScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"cmp r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CMPScalarShiftRightImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"cmp r%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CMPScalarShiftLeftImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"cmp r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ANDImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"and #$%8.8lX, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ANDScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"and r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ANDImmediateShiftScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"and #$%8.8lX, >>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ANDScalarShiftRightImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"and r%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ANDScalarShiftLeftImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"and r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ANDScalarShiftScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"and r%lu, >>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ANDScalarRotateScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"and r%lu, <>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_FTSTImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"ftst #$%8.8lX, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_FTSTScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"ftst r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_FTSTImmediateShiftScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"ftst #$%8.8lX, >>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_FTSTScalarShiftRightImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"ftst r%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_FTSTScalarShiftLeftImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"ftst r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}
uint32 Print_FTSTScalarShiftScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"ftst r%lu, >>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}
uint32 Print_FTSTScalarRotateScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"ftst r%lu, <>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ORImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"or #$%8.8lX, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ORScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"or r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ORImmediateShiftScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"or #$%8.8lX, >>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ORScalarShiftRightImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"or r%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ORScalarShiftLeftImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"or r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ORScalarShiftScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"or r%lu, >>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ORScalarRotateScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"or r%lu, <>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_EORImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"eor #$%8.8lX, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_EORScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"eor r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_EORImmediateShiftScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"eor #$%8.8lX, >>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_EORScalarShiftRightImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"eor r%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_EORScalarShiftLeftImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"eor r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_EORScalarShiftScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"eor r%lu, >>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_EORScalarRotateScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"eor r%lu, <>r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ADDWCImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"addwc #$%8.8lX, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ADDWCScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"addwc r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ADDWCScalarShiftRightImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"addwc r%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ADDWCScalarShiftLeftImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"addwc r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBWCImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"subwc #$%8.8lX, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBWCImmediateReverse(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"subwc r%lu, #$%8.8lX, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBWCScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"subwc r%lu, r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBWCScalarShiftRightImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"subwc r%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBWCScalarShiftLeftImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"subwc r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CMPWCImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"cmpwc #$%8.8lX, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CMPWCImmediateReverse(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"cmpwc r%lu, #$%8.8lX",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CMPWCScalar(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"cmpwc r%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CMPWCScalarShiftRightImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"cmpwc r%lu, >>#%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_CMPWCScalarShiftLeftImmediate(char *buffer, Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"cmpwc r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_ALU_SRC1],nuance.fields[FIELD_ALU_SRC2],nuance.fields[FIELD_ALU_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}
