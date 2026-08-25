// intrin.h stub for Linux
// MSVC intrinsics - GCC/Clang equivalents
#ifndef _COMPAT_INTRIN_H
#define _COMPAT_INTRIN_H

// x86intrin.h only exists on x86 toolchains. On ARM (and other non-x86 hosts)
// none of the intrinsics it provides are actually used here: the _lrotr/_lrotl
// macros below are pure C, and the only x86-specific helper (__cpuid) is itself
// guarded out. SSE-style _mm_* intrinsics, where referenced, come from sse2neon
// (pulled in via basetypes.h), not from this header.
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
#include <x86intrin.h>
#define _COMPAT_INTRIN_X86 1
#endif

// gcc's <ia32intrin.h> defines _lrotr/_lrotl as MACROS that map to
// __rorq/__rolq on LP64 (64-bit Linux) - i.e. 64-bit rotate. NUON code
// expects 32-bit rotate semantics, so a uint32 value zero-extended to
// 64-bit gets rotated wrongly. Force-redefine to 32-bit rotates.
#undef _lrotr
#undef _lrotl
#define _lrotr(val, shift) ((unsigned int)(((unsigned int)(val) >> ((shift) & 31)) | ((unsigned int)(val) << ((32 - ((shift) & 31)) & 31))))
#define _lrotl(val, shift) ((unsigned int)(((unsigned int)(val) << ((shift) & 31)) | ((unsigned int)(val) >> ((32 - ((shift) & 31)) & 31))))

#if defined(_COMPAT_INTRIN_X86) && !defined(__cpuid)
static inline void __cpuid(int cpuinfo[4], int info) {
    __asm__ __volatile__(
        "xchg %%ebx, %%edi;"
        "cpuid;"
        "xchg %%ebx, %%edi;"
        :"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
        :"0" (info)
    );
}
#endif

#endif
