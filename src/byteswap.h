#ifndef BYTESWAP_H
#define BYTESWAP_H

#define NUANCE_LITTLE_ENDIAN

#include "basetypes.h"
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <byteswap.h>
#define _byteswap_ushort(x) __builtin_bswap16(x)
#define _byteswap_ulong(x) __builtin_bswap32(x)
#endif

#ifdef NUANCE_LITTLE_ENDIAN

#include "Utility.h"

#if (defined(_MSC_VER) && !defined(__clang__)) || defined(NUANCE_ISA_SSSE3)
 #define NUANCE_BYTESWAP_USE_PSHUFB 1
#endif

__forceinline void SwapWordBytes(uint16 * const toswap)
{
#if 0 // old __fastcall dependent code
  __asm
  {
    mov ax, [ecx]
    xchg ah, al
    mov [ecx], ax
  }
#else
  *toswap = _byteswap_ushort(*toswap);
#endif
}

__forceinline uint16 SwapBytes(const uint16 toswap)
{
  return _byteswap_ushort(toswap);
}

__forceinline void SwapScalarBytes(uint32 * const toswap)
{
#if 0 // old __fastcall dependent code
  __asm
  {
    mov eax, [ecx]
    bswap eax
    mov [ecx], eax
  }
#else
  *toswap = _byteswap_ulong(*toswap);
#endif
}

__forceinline uint32 SwapBytes(const uint32 toswap)
{
  return _byteswap_ulong(toswap);
}

__forceinline void SwapShortVectorBytes(uint16 toswap[4])
{
/*#ifdef NUANCE_BYTESWAP_USE_PSHUFB
  if(SSSE3_supported)
  {
    // Swap the byte order of four 16-bit words (8 bytes) in a single SSSE3 shuffle
    const __m128i shuf = _mm_set_epi8(-1,-1,-1,-1,-1,-1,-1,-1, 6,7, 4,5, 2,3, 0,1);
    const __m128i v = _mm_loadl_epi64((const __m128i *)toswap);
    _mm_storel_epi64((__m128i *)toswap, _mm_shuffle_epi8(v, shuf));
    return;
  }
#endif*/
  toswap[0] = _byteswap_ushort(toswap[0]);
  toswap[1] = _byteswap_ushort(toswap[1]);
  toswap[2] = _byteswap_ushort(toswap[2]);
  toswap[3] = _byteswap_ushort(toswap[3]);
}

__forceinline void SwapVectorBytes(uint32 toswap[4])
{
/*#ifdef NUANCE_BYTESWAP_USE_PSHUFB
  if(SSSE3_supported)
  {
    // Swap the byte order of four 32-bit scalars (16 bytes) in a single SSSE3 shuffle
    const __m128i shuf = _mm_set_epi8(12,13,14,15, 8,9,10,11, 4,5,6,7, 0,1,2,3);
    const __m128i v = _mm_loadu_si128((const __m128i *)toswap);
    _mm_storeu_si128((__m128i *)toswap, _mm_shuffle_epi8(v, shuf));
    return;
  }
#endif*/
  toswap[0] = _byteswap_ulong(toswap[0]);
  toswap[1] = _byteswap_ulong(toswap[1]);
  toswap[2] = _byteswap_ulong(toswap[2]);
  toswap[3] = _byteswap_ulong(toswap[3]);
}

#else // !NUANCE_LITTLE_ENDIAN

#define SwapWordBytes(x)
#define SwapScalarBytes(x)
#define SwapShortVectorBytes(x)
#define SwapVectorBytes(x)

#define SwapBytes(x) (x)

#endif // NUANCE_LITTLE_ENDIAN

#endif
