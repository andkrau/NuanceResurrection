// intrin.h stub for Linux
// MSVC intrinsics - GCC/Clang equivalents
#ifndef _COMPAT_INTRIN_H
#define _COMPAT_INTRIN_H

#include <x86intrin.h>

#ifndef __cpuid
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
