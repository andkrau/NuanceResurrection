#include "basetypes.h"
#include "byteswap.h"
#include "NuonEnvironment.h"
#include "video.h"

extern NuonEnvironment nuonEnv;

#ifdef NUANCE_BYTESWAP_USE_PSHUFB
// SSSE3 kernels for the Type 4 (32-bit Y:Cr:Cb:ctrl) <-> Type 2 (16-bit Y6:Cr5:Cb5) pixel conversions
// used below, 8 pixels per call. Reproduces the scalar math exactly (and that one is according to the spec):
// Truncate low bits going 32->16, zero-pad low bits going 16->32, control byte = 0; and the same big-endian load/store byteswap

// 8x 32-bit pixel (big-endian in memory) -> 16-bit pixel (big-endian in memory)
static __forceinline void BDMA_Type8_Convert32to16_x8(const uint32* const pSrc32, uint16* const pDest16)
{
  const __m128i bswap32 = _mm_set_epi8(12,13,14,15, 8,9,10,11, 4,5,6,7, 0,1,2,3);
  // Gather the low 16 bits of each 32-bit lane into the low 8 bytes while swapping each pair of bytes (the SwapBytes on the 16-bit result); upper 8 bytes are zeroed.
  const __m128i packswap = _mm_set_epi8(-1,-1,-1,-1,-1,-1,-1,-1, 12,13, 8,9, 4,5, 0,1);
  const __m128i mY  = _mm_set1_epi32(0xFC00);
  const __m128i mCr = _mm_set1_epi32(0x03E0);
  const __m128i mCb = _mm_set1_epi32(0x001F);

  __m128i a = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)(pSrc32 + 0)), bswap32);
  __m128i b = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)(pSrc32 + 4)), bswap32);

  a = _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_srli_epi32(a, 16), mY),
                                _mm_and_si128(_mm_srli_epi32(a, 14), mCr)),
                                _mm_and_si128(_mm_srli_epi32(a, 11), mCb));
  b = _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_srli_epi32(b, 16), mY),
                                _mm_and_si128(_mm_srli_epi32(b, 14), mCr)),
                                _mm_and_si128(_mm_srli_epi32(b, 11), mCb));

  a = _mm_shuffle_epi8(a, packswap);
  b = _mm_shuffle_epi8(b, packswap);
  _mm_storeu_si128((__m128i*)pDest16, _mm_unpacklo_epi64(a, b));
}

// 8x 16-bit pixel (big-endian in memory) -> 32-bit pixel (big-endian in memory)
static __forceinline void BDMA_Type8_Convert16to32_x8(const uint16* const pSrc16, uint32* const pDest32)
{
  const __m128i bswap16 = _mm_set_epi8(14,15, 12,13, 10,11, 8,9, 6,7, 4,5, 2,3, 0,1);
  const __m128i bswap32 = _mm_set_epi8(12,13,14,15, 8,9,10,11, 4,5,6,7, 0,1,2,3);
  const __m128i zero = _mm_setzero_si128();
  const __m128i mY  = _mm_set1_epi32(0xFC00);
  const __m128i mCr = _mm_set1_epi32(0x03E0);
  const __m128i mCb = _mm_set1_epi32(0x001F);

  const __m128i v = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)pSrc16), bswap16);
  __m128i lo = _mm_unpacklo_epi16(v, zero); // pixels 0-3, zero-extended to 32-bit lanes
  __m128i hi = _mm_unpackhi_epi16(v, zero); // pixels 4-7

  lo = _mm_or_si128(_mm_or_si128(_mm_slli_epi32(_mm_and_si128(lo, mY), 16),
                                 _mm_slli_epi32(_mm_and_si128(lo, mCr), 14)),
                                 _mm_slli_epi32(_mm_and_si128(lo, mCb), 11));
  hi = _mm_or_si128(_mm_or_si128(_mm_slli_epi32(_mm_and_si128(hi, mY), 16),
                                 _mm_slli_epi32(_mm_and_si128(hi, mCr), 14)),
                                 _mm_slli_epi32(_mm_and_si128(hi, mCb), 11));

  _mm_storeu_si128((__m128i*)(pDest32 + 0), _mm_shuffle_epi8(lo, bswap32));
  _mm_storeu_si128((__m128i*)(pDest32 + 4), _mm_shuffle_epi8(hi, bswap32));
}
#endif

// 32bit -> 16bit RGB conversion
void BDMA_Type8_Write_0(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  const bool bRemote = flags & (1U << 28);
  const bool bDirect = flags & (1U << 27);
  const bool bDup = flags & (3U << 26); //bDup = dup | direct
  //const bool bTrigger = flags & (1U << 25);
  //const bool bRead = flags & (1U << 13);
  const int32 xsize = (flags >> 13) & 0x7F8U;
  //const uint32 type = (flags >> 14) & 0x03U;
  //const uint32 mode = flags & 0xFFFU;
  //const uint32 zcompare = (flags >> 1) & 0x07U;
  //const uint32 pixtype = (flags >> 4) & 0x0FU;
  //const uint32 bva = ((flags >> 7) & 0x06U) | (flags & 0x01U);
  const uint32 sdramBase = baseaddr & 0x7FFFFFFEU;
  const uint32 mpeBase = intaddr & 0x7FFFFFFCU;
  const uint32 xlen = (xinfo >> 16) & 0x3FFU;
  const uint32 xpos = xinfo & 0x7FFU;
        uint32 ylen = (yinfo >> 16) & 0x3FFU;
  const uint32 ypos = yinfo & 0x7FFU;

  //internal address is: bRemote ? system address (but still in MPE memory) : local to MPE
  void* const intMemory = nuonEnv.GetPointerToMemory(bRemote ? (mpeBase >> 23) & 0x1Fu : mpe.mpeIndex, mpeBase & 0x207FFFFF, false);

  //base address is always a system address (absolute)
  void* const baseMemory = nuonEnv.GetPointerToMemory((sdramBase >> 23) & 0x1Fu, sdramBase, false);

  const void* pSrc = intMemory;

  uint32 directValue;
  int32 srcAStep, srcBStep;
  if(bDup)
  {
    if(bDirect)
    {
      //Direct and Dup: intaddr is data.
      directValue = SwapBytes(intaddr);
    }
    else
    {
      //Dup but not Direct: read scalar from memory, no need to swap
      directValue = *(uint32 *)intMemory;
    }

    pSrc = (void *)&directValue;
    srcAStep = 0;
    srcBStep = 0;
  }
  else
  {
    srcAStep = 1;
    srcBStep = xlen;
  }

  constexpr uint32 srcOffset = 0;
  const uint32 destOffset = ypos * (uint32)xsize + xpos;

  /*if((GetPixBaseAddr(sdramBase,destOffset,2) >= nuonEnv.mainChannelLowerLimit) && (GetPixBaseAddr(sdramBase,destOffset,2) <= nuonEnv.mainChannelUpperLimit) ||
      (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) >= nuonEnv.mainChannelLowerLimit) && (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) <= nuonEnv.mainChannelUpperLimit))
  {
    nuonEnv.bMainBufferModified = true;
  }
  else if((GetPixBaseAddr(sdramBase,destOffset,2) >= nuonEnv.overlayChannelLowerLimit) && (GetPixBaseAddr(sdramBase,destOffset,2) <= nuonEnv.overlayChannelUpperLimit) ||
      (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) >= nuonEnv.overlayChannelLowerLimit) && (GetPixBaseAddr(sdramBase,(destOffset+((xsize - 1)*ylen)+xlen),2) <= nuonEnv.overlayChannelUpperLimit))
  {
    nuonEnv.bOverlayBufferModified = true;
  }*/

  //BVA = 000 (horizontal DMA, x increment, y increment)
  constexpr int32 destAStep = 1;
  const int32 destBStep = xsize;

  const uint32* pSrc32 = ((uint32 *)pSrc) + srcOffset;
  uint16* pDest16 = ((uint16 *)baseMemory) + destOffset;

  while(ylen--)
  {
    uint32 destA = 0;

#ifdef NUANCE_BYTESWAP_USE_PSHUFB
    // Fast path: contiguous source (srcAStep == 1, i.e. not Dup), 8 pixels at a time
    if(SSSE3_supported && srcAStep == 1)
    {
      for(; destA + 8 <= xlen; destA += 8)
        BDMA_Type8_Convert32to16_x8(pSrc32 + destA, pDest16 + destA);
    }
#endif

    // Scalar remainder, and the general (Dup / srcAStep == 0) path
    uint32 srcA = destA * (uint32)srcAStep;
    for(; destA < xlen; ++destA) // as destAStep==1
    {
      const uint32 pix32 = SwapBytes(pSrc32[srcA]);
      pDest16[destA] = SwapBytes((uint16)(((pix32 >> 16) & 0xFC00U) | ((pix32 >> 14) & 0x03E0U) | ((pix32 >> 11) & 0x001FU)));

      srcA += srcAStep;
    }

    pSrc32 += srcBStep;
    pDest16 += destBStep;
  }
}

void BDMA_Type8_Write_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_1", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_2", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_3", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_4", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_5", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_6", "Error", MB_OK);
#endif
}

void BDMA_Type8_Write_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Write_7", "Error", MB_OK);
#endif
}

// 16bit -> 32bit RGB conversion
void BDMA_Type8_Read_0(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
  //SI: Rem 0 Dir 0 Dup 0 Read 1 xs 256 zc 7 pix 8 bva 0

  const bool bRemote = flags & (1U << 28);
  //const bool bDirect = flags & (1U << 27);
  //const bool bDup = flags & (3U << 26); //bDup = dup | direct
  //const bool bTrigger = flags & (1U << 25);
  //const bool bRead = flags & (1U << 13);
  const int32 xsize = (flags >> 13) & 0x7F8U;
  //const uint32 type = (flags >> 14) & 0x03U;
  //const uint32 mode = flags & 0xFFFU;
  //const uint32 zcompare = (flags >> 1) & 0x07U;
  //const uint32 pixtype = (flags >> 4) & 0x0FU;
  //const uint32 bva = ((flags >> 7) & 0x06U) | (flags & 0x01U);
  const uint32 sdramBase = baseaddr & 0x7FFFFFFEU;
  const uint32 mpeBase = intaddr & 0x7FFFFFFCU;
  const uint32 xlen = (xinfo >> 16) & 0x3FFU;
  const uint32 xpos = xinfo & 0x7FFU;
        uint32 ylen = (yinfo >> 16) & 0x3FFU;
  const uint32 ypos = yinfo & 0x7FFU;

  //internal address is: bRemote ? system address (but still in MPE memory) : local to MPE
  void* const intMemory = nuonEnv.GetPointerToMemory(bRemote ? (mpeBase >> 23) & 0x1FU : mpe.mpeIndex, mpeBase & 0x207FFFFF, false);

  //base address is always a system address (absolute)
  void* const baseMemory = nuonEnv.GetPointerToMemory((sdramBase >> 23) & 0x1FU, sdramBase, false);

  constexpr int32 srcAStep = 1;
  const int32 srcBStep = xsize;

  const uint32 srcOffset = ypos * (uint32)xsize + xpos;
  constexpr uint32 destOffset = 0;

  //BVA = 000 (horizontal DMA, x increment, y increment)
  constexpr int32 destAStep = 1;
  const int32 destBStep = xlen;

  const uint16* pSrc16 = ((uint16 *)baseMemory) + srcOffset;
  uint32* pDest32 = ((uint32 *)intMemory) + destOffset;

  while(ylen--)
  {
    uint32 A = 0;

#ifdef NUANCE_BYTESWAP_USE_PSHUFB
    if(SSSE3_supported) // srcAStep and destAStep == 1, so always contiguous
    {
      for(; A + 8 <= xlen; A += 8)
        BDMA_Type8_Convert16to32_x8(pSrc16 + A, pDest32 + A);
    }
#endif

    for(; A < xlen; ++A) // as srcAStep and destAStep==1
    {
      const uint32 pix16 = SwapBytes(pSrc16[A]);
      pDest32[A] = SwapBytes(((pix16 & 0xFC00U) << 16) | ((pix16 & 0x03E0U) << 14) | ((pix16 & 0x001FU) << 11));
    }

    pSrc16 += srcBStep;
    pDest32 += destBStep;
  }
}

void BDMA_Type8_Read_1(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_1", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_2(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_2", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_3(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_3", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_4(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_4", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_5(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_5", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_6(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_6", "Error", MB_OK);
#endif
}

void BDMA_Type8_Read_7(MPE& mpe, const uint32 flags, const uint32 baseaddr, const uint32 xinfo, const uint32 yinfo, const uint32 intaddr)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL, "BDMA_Type8_Read_7", "Error", MB_OK);
#endif
}
