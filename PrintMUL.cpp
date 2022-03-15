#include "basetypes.h"
#include <cstdio>
#include <cstring>
#include "InstructionCache.h"

const char *GetImmediateShift(uint32 which)
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

uint32 Print_ADDMImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"addm #$%lX, r%lu, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_ADDM(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"addm r%lu, r%lu, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBMImmediateReverse(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"subm r%lu, #$%lX, r%lu",nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_SUBM(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"subm r%lu, r%lu, r%lu",nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MULScalarShiftAcshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul r%lu, r%lu, >>acshift, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MULScalarShiftRightImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul r%lu, r%lu, >>#%lu, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MULScalarShiftLeftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul r%lu, r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MULImmediateShiftAcshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul #%lu, r%lu, >>acshift, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MULScalarShiftScalar(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul r%lu, r%lu, >>r%lu, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MULImmediateShiftScalar(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul #%lu, r%lu, >>r%lu, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MULImmediateShiftRightImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul #%lu, r%lu, >>#%lu, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MULImmediateShiftLeftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul #%lu, r%lu, >>#-%lu, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2],nuance.fields[FIELD_MUL_INFO],nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_SVImmediateShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_sv #$%lX, v%lu, >>#%s, v%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_SVScalarShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_sv r%lu, v%lu, >>#%s, v%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}
uint32 Print_MUL_SVScalarShiftSvshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_sv r%lu, v%lu, >>svshift, v%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}
uint32 Print_MUL_SVRuShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_sv ru, v%lu, >>#%s, v%lu",nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_SVRuShiftSvshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_sv ru, v%lu, >>svshift, v%lu",nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_SVRvShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_sv rv, v%lu, >>#%s, v%lu",nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_SVRvShiftSvshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_sv rv, v%lu, >>svshift, v%lu",nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_SVVectorShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_sv v%lu, v%lu, >>#%s, v%lu",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_SVVectorShiftSvshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_sv v%lu, v%lu, >>svshift, v%lu",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_PImmediateShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_p #$%lX, v%lu, >>#%s, v%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_PScalarShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_p r%lu, v%lu, >>#%s, v%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_PScalarShiftSvshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_p r%lu, v%lu, >>svshift, v%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_PRuShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_p ru, v%lu, >>#%s, v%lu",nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_PRuShiftSvshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_p ru, v%lu, >>svshift, v%lu",nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_PRvShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_p rv, v%lu, >>#%s, v%lu",nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_PRvShiftSvshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_p rv, v%lu, >>svshift, v%lu",nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_PVectorShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_p v%lu, v%lu, >>#%s, v%lu",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_MUL_PVectorShiftSvshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"mul_p v%lu, v%lu, >>svshift, v%lu",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST] >> 2);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_DOTPScalarShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"dotp r%lu, v%lu, >>#%s, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_DOTPScalarShiftSvshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"dotp r%lu, v%lu, >>svshift, r%lu",nuance.fields[FIELD_MUL_SRC1],nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_DOTPVectorShiftImmediate(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"dotp v%lu, v%lu, >>#%s, r%lu",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,GetImmediateShift(nuance.fields[FIELD_MUL_INFO]),nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}

uint32 Print_DOTPVectorShiftSvshift(char *buffer, const Nuance &nuance, const bool bNewline)
{
  char tempStr[512];
  sprintf(tempStr,"dotp v%lu, v%lu, >>svshift, r%lu",nuance.fields[FIELD_MUL_SRC1] >> 2,nuance.fields[FIELD_MUL_SRC2] >> 2,nuance.fields[FIELD_MUL_DEST]);
  sprintf(buffer,"%s%s",tempStr, bNewline ? "\n" : "");
  return strlen(tempStr) + (bNewline ? 1 : 0);
}