#ifndef BYTESWAP_H
#define BYTESWAP_H

#define LITTLE_ENDIAN

#include "basetypes.h"

#ifdef LITTLE_ENDIAN
void __fastcall SwapWordBytes(uint16 *toswap);
void __fastcall SwapScalarBytes(uint32 *toswap);
void __fastcall SwapShortVectorBytes(uint16 *toswap);
void __fastcall SwapVectorBytes(uint32 *toswap);
#else
#define SwapWordBytes(x) 
#define SwapScalarBytes(x) 
#define SwapShortVectorBytes(x) 
#define SwapVectorBytes(x) 
#endif

#endif