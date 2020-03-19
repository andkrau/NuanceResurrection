#ifndef EXECUTE_MEM_H
#define EXECUTE_MEM_H

#include "mpe.h"

struct structBilinearAddressInfo
{
  void *pPixelData;
  uint32* pRegs;
  uint32 clutBase;
  uint32 base;
  uint32 control;
  uint32 x;
  uint32 y;
  uint32 mipped_xoffset;
  uint32 offset_address;
}; 

void GenerateMirrorLookupTable();
void GenerateSaturateColorTables();

void LoadPixelAbsolute(void);
void LoadPixelZAbsolute(void);
void StorePixelAbsolute(void);
void StorePixelZAbsolute(void);
void GetBilinearAddress(void);

NuanceHandlerProto Execute_Mirror;
NuanceHandlerProto Execute_MV_SImmediate;
NuanceHandlerProto Execute_MV_SScalar;
NuanceHandlerProto Execute_MV_V;
NuanceHandlerProto Execute_PopVector;
NuanceHandlerProto Execute_PopVectorRz;
NuanceHandlerProto Execute_PopScalarRzi1;
NuanceHandlerProto Execute_PopScalarRzi2;
NuanceHandlerProto Execute_PushVector;
NuanceHandlerProto Execute_PushVectorRz;
NuanceHandlerProto Execute_PushScalarRzi1;
NuanceHandlerProto Execute_PushScalarRzi2;
NuanceHandlerProto Execute_LoadScalarLinear;
NuanceHandlerProto Execute_LoadScalarControlRegisterAbsolute;
NuanceHandlerProto Execute_LoadByteAbsolute;
NuanceHandlerProto Execute_LoadWordAbsolute;
NuanceHandlerProto Execute_LoadScalarAbsolute;
NuanceHandlerProto Execute_LoadScalarRegisterAbsolute;
NuanceHandlerProto Execute_LoadShortVectorAbsolute;
NuanceHandlerProto Execute_LoadVectorAbsolute;
NuanceHandlerProto Execute_LoadVectorControlRegisterAbsolute;
NuanceHandlerProto Execute_LoadPixelAbsolute;
NuanceHandlerProto Execute_LoadPixelZAbsolute;
NuanceHandlerProto Execute_LoadByteLinear;
NuanceHandlerProto Execute_LoadByteBilinearUV;
NuanceHandlerProto Execute_LoadByteBilinearXY;
NuanceHandlerProto Execute_LoadWordLinear;
NuanceHandlerProto Execute_LoadWordBilinearUV;
NuanceHandlerProto Execute_LoadWordBilinearXY;
NuanceHandlerProto Execute_LoadScalarBilinearUV;
NuanceHandlerProto Execute_LoadScalarBilinearXY;
NuanceHandlerProto Execute_LoadShortVectorLinear;
NuanceHandlerProto Execute_LoadShortVectorBilinearUV;
NuanceHandlerProto Execute_LoadShortVectorBilinearXY;
NuanceHandlerProto Execute_LoadVectorLinear;
NuanceHandlerProto Execute_LoadVectorBilinearUV;
NuanceHandlerProto Execute_LoadVectorBilinearXY;
NuanceHandlerProto Execute_LoadPixelLinear;
NuanceHandlerProto Execute_LoadPixelBilinearUV;
NuanceHandlerProto Execute_LoadPixelBilinearXY;
NuanceHandlerProto Execute_LoadPixelZLinear;
NuanceHandlerProto Execute_LoadPixelZBilinearUV;
NuanceHandlerProto Execute_LoadPixelZBilinearXY;
NuanceHandlerProto Execute_StoreScalarAbsolute;
NuanceHandlerProto Execute_StoreScalarControlRegisterAbsolute;
NuanceHandlerProto Execute_StoreShortVectorAbsolute;
NuanceHandlerProto Execute_StoreVectorAbsolute;
NuanceHandlerProto Execute_StoreVectorControlRegisterAbsolute;
NuanceHandlerProto Execute_StorePixelAbsolute;
NuanceHandlerProto Execute_StorePixelZAbsolute;
NuanceHandlerProto Execute_StoreScalarLinear;
NuanceHandlerProto Execute_StoreScalarBilinearUV;
NuanceHandlerProto Execute_StoreScalarBilinearXY;
NuanceHandlerProto Execute_StoreShortVectorLinear;
NuanceHandlerProto Execute_StoreShortVectorBilinearUV;
NuanceHandlerProto Execute_StoreShortVectorBilinearXY;
NuanceHandlerProto Execute_StoreVectorLinear;
NuanceHandlerProto Execute_StoreVectorBilinearUV;
NuanceHandlerProto Execute_StoreVectorBilinearXY;
NuanceHandlerProto Execute_StorePixelLinear;
NuanceHandlerProto Execute_StorePixelBilinearUV;
NuanceHandlerProto Execute_StorePixelBilinearXY;
NuanceHandlerProto Execute_StorePixelZLinear;
NuanceHandlerProto Execute_StorePixelZBilinearUV;
NuanceHandlerProto Execute_StorePixelZBilinearXY;
NuanceHandlerProto Execute_StoreScalarControlRegisterImmediate;
NuanceHandlerProto Execute_StoreScalarImmediate;

#endif
