#include "basetypes.h"
#include "byteswap.h"

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif

#ifdef LITTLE_ENDIAN
void __fastcall SwapWordBytes(uint16 *toswap)
{
  __asm
  {
    mov ax, [ecx]
    xchg ah, al
    mov [ecx], ax
  }
}

void __fastcall SwapScalarBytes(uint32 *toswap)
{
  __asm
  {
    mov eax, [ecx]
    bswap eax
    mov [ecx], eax
  }
}

void __fastcall SwapShortVectorBytes(uint16 *toswap)
{
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
}

void __fastcall SwapVectorBytes(uint32 *toswap)
{
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
}
#endif
