#ifndef UTILITY_H
#define UTILITY_H

#include "basetypes.h"

// CPU SIMD feature flags
//
// If the compiler is already targeting a given ISA level via command line param, the corresponding flag is a
// compile-time constexpr 'true'. This lets the compiler optimize the "if(Xxx_supported)"
// feature branches and drop the scalar fallbacks entirely. Otherwise the flag is a runtime bool,
// detected once via CPUID in init_supported_CPU_extensions() (see Utility.cpp)
//
// Detecting "is this ISA level enabled at compile time?" differs by compiler, should work for MSVC, clang and gcc.
// NUANCE_ISA_* helper macros below capture this

#if defined(__SSE__)   || defined(__AVX__)
  #define NUANCE_ISA_SSE   1
#endif
#if defined(__SSE2__)  || defined(__AVX__)
  #define NUANCE_ISA_SSE2  1
#endif
#if defined(__SSE3__)  || defined(__AVX__)
  #define NUANCE_ISA_SSE3  1
#endif
#if defined(__SSSE3__) || defined(__AVX__)
  #define NUANCE_ISA_SSSE3 1
#endif
#if defined(__SSE4_1__) || defined(__AVX__)
  #define NUANCE_ISA_SSE41 1
#endif
#if defined(__SSE4_2__) || defined(__AVX__)
  #define NUANCE_ISA_SSE42 1
#endif
#ifdef __SSE4A__ // AMD-only extension, not implied by AVX
  #define NUANCE_ISA_SSE4A 1
#endif
#ifdef __AVX__
  #define NUANCE_ISA_AVX   1
#endif

#ifdef NUANCE_ISA_SSE
constexpr bool SSE_supported = true;
#else
extern bool SSE_supported;
#endif

#ifdef NUANCE_ISA_SSE2
constexpr bool SSE2_supported = true;
#else
extern bool SSE2_supported;
#endif

#ifdef NUANCE_ISA_SSE3
constexpr bool SSE3_supported = true;
#else
extern bool SSE3_supported;
#endif

#ifdef NUANCE_ISA_SSSE3
constexpr bool SSSE3_supported = true;
#else
extern bool SSSE3_supported;
#endif

#ifdef NUANCE_ISA_SSE41
constexpr bool SSE41_supported = true;
#else
extern bool SSE41_supported;
#endif

#ifdef NUANCE_ISA_SSE42
constexpr bool SSE42_supported = true;
#else
extern bool SSE42_supported;
#endif

#ifdef NUANCE_ISA_SSE4A
constexpr bool SSE4a_supported = true;
#else
extern bool SSE4a_supported;
#endif

// SSE5 was never/not yet released, and has no predefined compiler macro, so it is always runtime-detected
extern bool SSE5_supported;

#ifdef NUANCE_ISA_AVX
constexpr bool AVX_supported = true;
#else
extern bool AVX_supported;
#endif

void init_supported_CPU_extensions();

__forceinline uint32 OnesCount(const uint32 x)
{
#ifdef _MSC_VER
    return __popcnt(x);
#else
    return __builtin_popcount(x);
#endif
    /*x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);
    return(x & 0x0000003f);*/
}

#endif
