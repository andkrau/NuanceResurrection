#ifndef BASETYPES_H
#define BASETYPES_H

#include <xmmintrin.h>

#define ENABLE_ASSERTS
#define ENABLE_EMULATION_MESSAGEBOXES // also enables "DumpCompiledBlocks" option

#define STRICT
#define WIN32_LEAN_AND_MEAN

typedef signed __int8 int8;
typedef signed __int16 int16;
typedef signed __int32 int32;
typedef signed __int64 int64;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
typedef __m128 uint128;
typedef unsigned char uchar;
typedef signed char schar;

#if 1 // initialize all nuon memory to a defined value (0xcd, like VS debugger)
#include <memory.h>
inline void init_nuon_mem(uint8* const p, const size_t length)
{
  memset(p,0xcd,length);
}
#else
inline void init_nuon_mem(const uint8* const p, const size_t length) {}
#endif

#ifdef ENABLE_ASSERTS
#include <cassert>
#else
#define assert(expression) ((void)0)
#endif

#endif
