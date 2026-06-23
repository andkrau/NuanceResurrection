#include "Utility.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

// Storage is only needed for flags that are runtime-detected; the ones fixed at compile time are
// constexpr in Utility.h (and must not be redefined or assigned here). The NUANCE_ISA_* macros are
// defined by Utility.h above and capture the per-compiler "ISA enabled at compile time?" logic.
#ifndef NUANCE_ISA_SSE
bool SSE_supported   = false;
#endif
#ifndef NUANCE_ISA_SSE2
bool SSE2_supported  = false;
#endif
#ifndef NUANCE_ISA_SSE3
bool SSE3_supported  = false;
#endif
#ifndef NUANCE_ISA_SSSE3
bool SSSE3_supported = false;
#endif
#ifndef NUANCE_ISA_SSE41
bool SSE41_supported = false;
#endif
#ifndef NUANCE_ISA_SSE42
bool SSE42_supported = false;
#endif
#ifndef NUANCE_ISA_SSE4A
bool SSE4a_supported = false;
#endif
bool SSE5_supported  = false;
#ifndef NUANCE_ISA_AVX
bool AVX_supported   = false;
#endif

// x86 CPU-feature probing. None of this exists on non-x86 hosts (e.g. ARM),
// where init_supported_CPU_extensions() below leaves the runtime flags false.
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
#define NUANCE_ARCH_X86 1
#endif

#if defined(NUANCE_ARCH_X86) && defined(__GNUC__)
static void __cpuid(int* cpuinfo, const int info)
{
#if defined(__x86_64__)
	__asm__ __volatile__(
		"xchgq %%rbx, %%rdi;"
		"cpuid;"
		"xchgq %%rbx, %%rdi;"
		:"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
		:"0" (info)
	);
#else
	__asm__ __volatile__(
		"xchgl %%ebx, %%edi;"
		"cpuid;"
		"xchgl %%ebx, %%edi;"
		:"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
		:"0" (info)
	);
#endif
}
#ifndef NUANCE_ISA_AVX
// GCC 11+ declares a builtin _xgetbv() in <x86gprintrin.h> (pulled in via the
// intrinsics headers), so a function named _xgetbv here is an ambiguating
// redeclaration. Use our own name to avoid the clash on modern toolchains.
static unsigned long long nuance_xgetbv(const unsigned int index)
{
	unsigned int eax, edx;
	__asm__ __volatile__(
		"xgetbv;"
		: "=a" (eax), "=d"(edx)
		: "c" (index)
	);
	return ((unsigned long long)edx << 32) | eax;
}
#endif
#endif

#if defined(NUANCE_ARCH_X86) && defined(_MSC_VER) && !defined(NUANCE_ISA_AVX)
// MSVC provides the _xgetbv intrinsic via <intrin.h>; wrap it under our name so
// the call site below is toolchain-agnostic.
static inline unsigned long long nuance_xgetbv(const unsigned int index) { return _xgetbv(index); }
#endif

void init_supported_CPU_extensions()
{
#ifdef NUANCE_ARCH_X86
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);

	// Check SSE, SSE2, SSE3, SSSE3, SSE4.1, and SSE4.2 support
	// (only for flags not already constexpr'ed at compile time; see NUANCE_ISA_* in Utility.h)
#ifndef NUANCE_ISA_SSE
	SSE_supported   = !!(cpuinfo[3] & (1 << 25));
#endif
#ifndef NUANCE_ISA_SSE2
	SSE2_supported  = !!(cpuinfo[3] & (1 << 26));
#endif
#ifndef NUANCE_ISA_SSE3
	SSE3_supported  = !!(cpuinfo[2] & (1 << 0));
#endif
#ifndef NUANCE_ISA_SSSE3
	SSSE3_supported = !!(cpuinfo[2] & (1 << 9));
#endif
#ifndef NUANCE_ISA_SSE41
	SSE41_supported = !!(cpuinfo[2] & (1 << 19));
#endif
#ifndef NUANCE_ISA_SSE42
	SSE42_supported = !!(cpuinfo[2] & (1 << 20));
#endif

	// ----------------------------------------------------------------------

	// Check AVX support
	// http://software.intel.com/en-us/blogs/2011/04/14/is-avx-enabled/
	// http://insufficientlycomplicated.wordpress.com/2011/11/07/detecting-intel-advanced-vector-extensions-avx-in-visual-studio/

#ifndef NUANCE_ISA_AVX
	AVX_supported = !!(cpuinfo[2] & (1 << 28));
	const bool osxsave_supported = !!(cpuinfo[2] & (1 << 27));
	if (osxsave_supported && AVX_supported)
	{
		// _XCR_XFEATURE_ENABLED_MASK = 0
		const unsigned long long xcrFeatureMask = nuance_xgetbv(0);
		AVX_supported = ((xcrFeatureMask & 0x6) == 0x6);
	}
#endif

	// ----------------------------------------------------------------------

	// Check SSE4a and SSE5 support

	__cpuid(cpuinfo, 0x80000000);
	const int numExtendedIds = cpuinfo[0];
	if (numExtendedIds >= 0x80000001)
	{
		__cpuid(cpuinfo, 0x80000001);
#ifndef NUANCE_ISA_SSE4A
		SSE4a_supported = !!(cpuinfo[2] & (1 << 6));
#endif
		SSE5_supported  = !!(cpuinfo[2] & (1 << 11));
	}
#else
	// Non-x86 (e.g. ARM): no x86 SSE feature bits to probe. The runtime
	// *_supported flags keep their false defaults, so every SIMD-gated path
	// takes its scalar fallback. sse2neon still supplies the _mm_* API for the
	// translation units that reference it unconditionally.
#endif // NUANCE_ARCH_X86
}
