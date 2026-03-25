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
#if 0 // old __fastcall dependent code
  __asm
  {
    mov dx, [ecx]
    mov ax, [ecx + 2]
    xchg dl, dh
    xchg al, ah
    mov [ecx], dx
    mov [ecx + 2], ax
    mov dx, [ecx + 4]
    mov ax, [ecx + 6]
    xchg dl, dh
    xchg al, ah
    mov [ecx + 4], dx
    mov [ecx + 6], ax
  }
#else
  toswap[0] = _byteswap_ushort(toswap[0]);
  toswap[1] = _byteswap_ushort(toswap[1]);
  toswap[2] = _byteswap_ushort(toswap[2]);
  toswap[3] = _byteswap_ushort(toswap[3]);
#endif
}

__forceinline void SwapVectorBytes(uint32 toswap[4])
{
#if 0 // old __fastcall dependent code
  __asm
  {
    mov edx, [ecx]
    mov eax, [ecx + 4]
    bswap edx
    bswap eax
    mov [ecx],edx
    mov [ecx + 4],eax
    mov edx, [ecx + 8]
    mov eax, [ecx + 12]
    bswap edx
    bswap eax
    mov [ecx + 8],edx
    mov [ecx + 12],eax
  }
#else
  toswap[0] = _byteswap_ulong(toswap[0]);
  toswap[1] = _byteswap_ulong(toswap[1]);
  toswap[2] = _byteswap_ulong(toswap[2]);
  toswap[3] = _byteswap_ulong(toswap[3]);
#endif
}

#else // !NUANCE_LITTLE_ENDIAN

#define SwapWordBytes(x)
#define SwapScalarBytes(x)
#define SwapShortVectorBytes(x)
#define SwapVectorBytes(x)

#define SwapBytes(x) (x)

#endif // NUANCE_LITTLE_ENDIAN

#endif
