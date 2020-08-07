#ifdef _MSC_VER
#include <intrin.h>
#endif

bool SSE_supported   = false;
bool SSE2_supported  = false;
bool SSE3_supported  = false;
bool SSSE3_supported = false;
bool SSE41_supported = false;
bool SSE42_supported = false;
bool SSE4a_supported = false;
bool SSE5_supported  = false;
bool AVX_supported   = false;

#ifdef __GNUC__
static void __cpuid(int* cpuinfo, const int info)
{
	__asm__ __volatile__(
		"xchg %%ebx, %%edi;"
		"cpuid;"
		"xchg %%ebx, %%edi;"
		:"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
		:"0" (info)
	);
}
static unsigned long long _xgetbv(const unsigned int index)
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

void init_supported_CPU_extensions()
{
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);

	// Check SSE, SSE2, SSE3, SSSE3, SSE4.1, and SSE4.2 support
	SSE_supported   = !!(cpuinfo[3] & (1 << 25));
	SSE2_supported  = !!(cpuinfo[3] & (1 << 26));
	SSE3_supported  = !!(cpuinfo[2] & (1 << 0));
	SSSE3_supported = !!(cpuinfo[2] & (1 << 9));
	SSE41_supported = !!(cpuinfo[2] & (1 << 19));
	SSE42_supported = !!(cpuinfo[2] & (1 << 20));

	// ----------------------------------------------------------------------

	// Check AVX support
	// http://software.intel.com/en-us/blogs/2011/04/14/is-avx-enabled/
	// http://insufficientlycomplicated.wordpress.com/2011/11/07/detecting-intel-advanced-vector-extensions-avx-in-visual-studio/

	AVX_supported = !!(cpuinfo[2] & (1 << 28));
	const bool osxsave_supported = !!(cpuinfo[2] & (1 << 27));
	if (osxsave_supported && AVX_supported)
	{
		// _XCR_XFEATURE_ENABLED_MASK = 0
		const unsigned long long xcrFeatureMask = _xgetbv(0);
		AVX_supported = ((xcrFeatureMask & 0x6) == 0x6);
	}

	// ----------------------------------------------------------------------

	// Check SSE4a and SSE5 support

	__cpuid(cpuinfo, 0x80000000);
	const int numExtendedIds = cpuinfo[0];
	if (numExtendedIds >= 0x80000001)
	{
		__cpuid(cpuinfo, 0x80000001);
		SSE4a_supported = !!(cpuinfo[2] & (1 << 6));
		SSE5_supported  = !!(cpuinfo[2] & (1 << 11));
	}
}
