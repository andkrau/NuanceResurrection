#include "basetypes.h"
#include "InstructionCache.h"
#include "mpe.h"
#include <cstdlib>
#include <intrin.h>

void Execute_ABS(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_ZERO | CC_ALU_CARRY);
  const int32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  if(src1 < 0)
  {
    mpe.cc |= CC_ALU_CARRY;
    if(src1 == (int32)0x80000000)
    {
      // "function might be considered to fail" according to spec
      //source was negative, result is negative (non-zero)
      mpe.cc |= (CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
    }
    else
    {
      mpe.regs[nuance.fields[FIELD_ALU_DEST]] = -src1;
    }
  }
  else
  {
    if(!src1)
    {
      //source wasnt negative, result is zero (non-negative)
      mpe.cc |= CC_ALU_ZERO;
    }
  }
}

void Execute_BITSScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE);
  const uint32 result = nuance.fields[FIELD_ALU_SRC1] & 
    (pRegs[nuance.fields[FIELD_ALU_DEST]] >> (pRegs[nuance.fields[FIELD_ALU_SRC2]] & 0x1F));

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    // negative = bit 31 of result
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_BITSImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE);
  const uint32 result = nuance.fields[FIELD_ALU_SRC1] & 
    (pRegs[nuance.fields[FIELD_ALU_DEST]] >> nuance.fields[FIELD_ALU_SRC2]);

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    // negative = bit 31 of result
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_BTST(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  // SPEC MISMATCH - DO NOT "FIX":
  // "MPE Instruction Set Reference" describes BTST:
  //   z : set if the selected bit is zero, cleared otherwise.
  //   n : set if the selected bit was bit 31 and it was not zero.
  //   c : unchanged.
  //   v : cleared.
  //
  // The N-flag clause looks like a sign-of-MSB rule for "treat the source as
  // negative if bit 31 is set". A spec-following implementation would
  // be the commented-out block below.
  //
  // HOWEVER, T3K's own audio engine at 0x80014e3a..0x80014f54 contains what
  // looks like a CPU self-test for BTST: it sets r6 to known values
  // (0x80000000, 0xF00FF00F, 0x7FFFFFFF), forces CC to a known starting
  // value with `st_s #imm,cc`, runs `btst #$1F,r6`, reads CC back, ANDs
  // with 0x1FF, and `cmp`s against an expected post-state. For
  // r6=0x80000000 / initial CC=0x1F7, the expected post-CC is 0x1F2 -
  // i.e., bit 3 (N, value 0x08) is *cleared*, not set, even though bit 31
  // was tested and was non-zero. The other two tests are consistent.
  //
  // Since T3K is silicon-validated, the real hardware does NOT
  // set N for bit-31 BTST despite the spec wording. The original emulator
  // implementation (clear N along with Z/V, only set Z when the bit is 0)
  // matches that observed silicon behaviour and is therefore preserved.
  //
  // This is also undermined by Mike Perry's instructiontest.c prog,
  // which mentions this NUON BTST hardware bug/behavior in there.

  const uint32 mask = (uint32_t)nuance.fields[FIELD_ALU_SRC1]; // (1 << bitN) per DecodeALU
  const uint32 bit = mask & pRegs[nuance.fields[FIELD_ALU_SRC2]];
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
  if(!bit)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
#if 0 // see above
  if((mask == 0x80000000u) && bit)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
#endif
}

void Execute_BUTT(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];
  const uint32 dest = (uint32_t)nuance.fields[FIELD_ALU_DEST];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 sum;
  mpe.cc |= _addcarry_u32(0, src1, src2, &sum) ? CC_ALU_CARRY : 0;
  mpe.regs[dest + 1] = src2 - src1;

  if((~(src1 ^ src2)) & (src1 ^ sum) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(sum & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!sum)
  {
    mpe.cc |= CC_ALU_ZERO;
  }

  mpe.regs[dest] = sum;
}

void Execute_COPY(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src = pRegs[nuance.fields[FIELD_ALU_SRC2]];
  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = src;

  if(!src)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    // negative = bit 31 of result
    mpe.cc |= ((src >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_MSB(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  uint32 sigbits;

  mpe.cc &= ~CC_ALU_ZERO;
  int32 n = pRegs[nuance.fields[FIELD_ALU_SRC1]];

  if ((uint32)n + 1 <= 1)
  {
    sigbits = 0;
  }
  else
  {
    //n = n if positive, n = ~n if negative
    n = (n ^ (n >> 31));

#ifdef _MSC_VER
    unsigned long idx;
    _BitScanReverse(&idx, (unsigned long)n);
    sigbits = (uint32)(idx + 1);
#else
    sigbits = (uint32)(32 - __builtin_clz((unsigned int)n));
#endif
  }


  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = sigbits;

  if(!sigbits)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
}

void Execute_SAT(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  int32 mask = (0x01U << nuance.fields[FIELD_ALU_SRC2]) - 1;
  const int32 n = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 dest = (uint32_t)nuance.fields[FIELD_ALU_DEST];

  //initial mask is largest negative number using 'bits' bits, minus one to get
  //largest positive signed number using 'bits' bits

  //NOTE: the bits parameter will be one less than the number of bits desired
  //so you do not have to subtract 1 from the shift amount to get the correct
  //mask

  if(n > mask)
  {
    mpe.regs[dest] = mask;
  }
  else
  {
    //inverting mask gives smallest negative number possible using 'bits' bits
    mask = ~mask;

    if(n < mask)
    {
      mpe.regs[dest] = mask;
    }
    else
    {
      mpe.regs[dest] = n;
    }
  }

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE);

  if(!mpe.regs[dest])
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    // negative = bit 31 of result
    mpe.cc |= ((mpe.regs[dest] >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_AS(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
        uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]] & 0x3FU;
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_NEGATIVE | CC_ALU_CARRY);

  if(src1 & 0x20)
  {
    //shift left
    src1 = 64U - src1;
    //carry = bit 31 of source
    mpe.cc |= ((src2 >> 30) & CC_ALU_CARRY);
    const uint32 result = src2 << src1;
    //The Nuon allows left shifts by 32, effectively clearing out the
    //source register.  The x86 shift instructions performs shifts with
    //counts adjusted to the range [0,31] so the result of a shift by 32
    //is treated as a shift by zero.  The result of the shift needs to be
    //monitored for this special case, and when it happens, the destination
    //register should be forced to zero.

    if((result == 0) || (src1 == 32))
    {
      mpe.regs[nuance.fields[FIELD_ALU_DEST]] = 0;
      mpe.cc |= CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
      mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
    }
  }
  else
  {
    //shift right
    const uint32 result = ((int32)src2) >> src1;
    mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
    // carry = bit 0 of source
    mpe.cc |= ((src2 << 1) & CC_ALU_CARRY);    

    if(!result)
    {
      mpe.cc |= CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
    }
  }
}

void Execute_ASL(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_NEGATIVE | CC_ALU_CARRY);

  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  // carry = bit 31 of source
  mpe.cc |= ((src2 >> 30) & CC_ALU_CARRY);
  const uint32 result = src2 << nuance.fields[FIELD_ALU_SRC1];

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    // negative = bit 31 of result
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ASR(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_NEGATIVE | CC_ALU_CARRY);
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  const uint32 result = ((int32)src2) >> nuance.fields[FIELD_ALU_SRC1];
  // carry = bit 0 of source
  mpe.cc |= ((src2 << 1) & CC_ALU_CARRY);

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    // negative = bit 31 of result
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_LS(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
        uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]] & 0x3FU;
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_NEGATIVE | CC_ALU_CARRY);

  if(src1 & 0x20)
  {
    //shift left
    src1 = 64U - src1;
    //carry = bit 31 of source
    mpe.cc |= ((src2 >> 30) & CC_ALU_CARRY);
    const uint32 result = src2 << src1;
    //The Nuon allows left shifts by 32, effectively clearing out the
    //source register.  The x86 shift instructions performs shifts with
    //counts adjusted to the range [0,31] so the result of a shift by 32
    //is treated as a shift by zero.  The result of the shift needs to be
    //monitored for this special case, and when it happens, the destination
    //register should be forced to zero.

    if((result == 0) || (src1 == 32))
    {
      mpe.regs[nuance.fields[FIELD_ALU_DEST]] = 0;
      mpe.cc |= CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
      mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
    }
  }
  else
  {
    //shift right
    const uint32 result = src2 >> src1;
    mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
    // carry = bit 0 of source
    mpe.cc |= ((src2 << 1) & CC_ALU_CARRY);    

    if(!result)
    {
      mpe.cc |= CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
    }
  }
}

void Execute_LSR(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_NEGATIVE | CC_ALU_CARRY);
  const uint32 result = src2 >> nuance.fields[FIELD_ALU_SRC1];
  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
  // carry = bit 0 of source
  mpe.cc |= ((src2 << 1) & CC_ALU_CARRY);

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    // negative = bit 31 of result
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ROT(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
        uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]] & 0x3FU;
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_NEGATIVE);

  if(src1 & 0x20)
  {
    //shift left
    src1 = 64U - src1;
    const uint32 result = _lrotl(src2,src1);
    mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

    if(!result)
    {
      mpe.cc |= CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
    }
  }
  else
  {
    const uint32 result = _lrotr(src2,src1);
    mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

    if(!result)
    {
      mpe.cc |= CC_ALU_ZERO;
    }
    else
    {
      // negative = bit 31 of result
      mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
    }
  }
}

void Execute_ROL(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_NEGATIVE);

  const uint32 result = _lrotl(src2, (int)nuance.fields[FIELD_ALU_SRC1]);
  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    // negative = bit 31 of result
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ROR(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_OVERFLOW | CC_ALU_NEGATIVE);

  const uint32 result = _lrotr(src2, (int)nuance.fields[FIELD_ALU_SRC1]);
  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    // negative = bit 31 of result
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ADD_P(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
  const uint32 dest = (uint32_t)nuance.fields[FIELD_ALU_DEST];

  mpe.regs[dest + 0] =
    (pRegs[src2 + 0] & 0xFFFF0000) +
    (pRegs[src1 + 0] & 0xFFFF0000);

  mpe.regs[dest + 1] =
    (pRegs[src2 + 1] & 0xFFFF0000) +
    (pRegs[src1 + 1] & 0xFFFF0000);

  mpe.regs[dest + 2] =
    (pRegs[src2 + 2] & 0xFFFF0000) +
    (pRegs[src1 + 2] & 0xFFFF0000);
}

void Execute_SUB_P(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
  const uint32 dest = (uint32_t)nuance.fields[FIELD_ALU_DEST];

  mpe.regs[dest + 0] =
    (pRegs[src2 + 0] & 0xFFFF0000) -
    (pRegs[src1 + 0] & 0xFFFF0000);

  mpe.regs[dest + 1] =
    (pRegs[src2 + 1] & 0xFFFF0000) -
    (pRegs[src1 + 1] & 0xFFFF0000);

  mpe.regs[dest + 2] =
    (pRegs[src2 + 2] & 0xFFFF0000) -
    (pRegs[src1 + 2] & 0xFFFF0000);
}

void Execute_ADD_SV(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
  const uint32 dest = (uint32_t)nuance.fields[FIELD_ALU_DEST];

  mpe.regs[dest + 0] =
    (pRegs[src2 + 0] & 0xFFFF0000) +
    (pRegs[src1 + 0] & 0xFFFF0000);

  mpe.regs[dest + 1] =
    (pRegs[src2 + 1] & 0xFFFF0000) +
    (pRegs[src1 + 1] & 0xFFFF0000);

  mpe.regs[dest + 2] =
    (pRegs[src2 + 2] & 0xFFFF0000) +
    (pRegs[src1 + 2] & 0xFFFF0000);

  mpe.regs[dest + 3] =
    (pRegs[src2 + 3] & 0xFFFF0000) +
    (pRegs[src1 + 3] & 0xFFFF0000);
}

void Execute_SUB_SV(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
  const uint32 dest = (uint32_t)nuance.fields[FIELD_ALU_DEST];

  mpe.regs[dest + 0] =
    (pRegs[src2 + 0] & 0xFFFF0000) -
    (pRegs[src1 + 0] & 0xFFFF0000);

  mpe.regs[dest + 1] =
    (pRegs[src2 + 1] & 0xFFFF0000) -
    (pRegs[src1 + 1] & 0xFFFF0000);

  mpe.regs[dest + 2] =
    (pRegs[src2 + 2] & 0xFFFF0000) -
    (pRegs[src1 + 2] & 0xFFFF0000);

  mpe.regs[dest + 3] =
    (pRegs[src2 + 3] & 0xFFFF0000) -
    (pRegs[src1 + 3] & 0xFFFF0000);
}

void Execute_ADDImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _addcarry_u32(0, src1, src2, &result) ? CC_ALU_CARRY : 0;

  if((~(src1 ^ src2)) & (src1 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_ADDScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _addcarry_u32(0, src1, src2, &result) ? CC_ALU_CARRY : 0;

  if((~(src1 ^ src2)) & (src1 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_ADDScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = ((int32)pRegs[nuance.fields[FIELD_ALU_SRC1]]) >> nuance.fields[FIELD_ALU_SRC2];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _addcarry_u32(0, src1, src2, &result) ? CC_ALU_CARRY : 0;

  if((~(src1 ^ src2)) & (src1 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_ADDScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]] << nuance.fields[FIELD_ALU_SRC2]; // do not cast to u64 before shift!
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _addcarry_u32(0, src1, src2, &result) ? CC_ALU_CARRY : 0;

  if((~(src1 ^ src2)) & (src1 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_SUBImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _subborrow_u32(0, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_SUBImmediateReverse(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _subborrow_u32(0, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_SUBScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _subborrow_u32(0, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_SUBScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = ((int32)pRegs[nuance.fields[FIELD_ALU_SRC1]]) >> nuance.fields[FIELD_ALU_SRC2];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _subborrow_u32(0, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_SUBScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]] << nuance.fields[FIELD_ALU_SRC2]; // do not cast to u64 before shift!
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _subborrow_u32(0, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_CMPImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _subborrow_u32(0, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
}

void Execute_CMPImmediateReverse(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _subborrow_u32(0, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
}

void Execute_CMPScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _subborrow_u32(0, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
}

void Execute_CMPScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = ((int32)pRegs[nuance.fields[FIELD_ALU_SRC1]]) >> nuance.fields[FIELD_ALU_SRC2];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _subborrow_u32(0, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
}

void Execute_CMPScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]] << nuance.fields[FIELD_ALU_SRC2]; // do not cast to u64 before shift!
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY);

  uint32 result;
  mpe.cc |= _subborrow_u32(0, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
}

void Execute_ANDImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  const uint32 result = src1 & src2;

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ANDScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  const uint32 result = src1 & src2;

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ANDImmediateShiftScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    const uint32 shamt = 64U - src2;
    dest &= (shamt < 32) ? (src1 << shamt) : 0;
  }
  else
  {
    dest &= (src1 >> src2);
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ANDScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  dest &= (src1 >> src2);

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ANDScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  dest &= (src1 << src2);

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ANDScalarShiftScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    const uint32 shamt = 64U - src2;
    dest &= (shamt < 32) ? (src1 << shamt) : 0;
  }
  else
  {
    dest &= (src1 >> src2);
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ANDScalarRotateScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    dest &= _lrotl(src1, (64U - src2) & 31U);
  }
  else
  {
    dest &= _lrotr(src1, src2);
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_FTSTImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  const uint32 result = src1 & src2;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_FTSTScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  const uint32 result = src1 & src2;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_FTSTImmediateShiftScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    const uint32 shamt = 64U - src2;
    dest &= (shamt < 32) ? (src1 << shamt) : 0;
  }
  else
  {
    dest &= (src1 >> src2);
  }

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_FTSTScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  dest &= (src1 >> src2);

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_FTSTScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  dest &= (src1 << src2);

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_FTSTScalarShiftScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    const uint32 shamt = 64U - src2;
    dest &= (shamt < 32) ? (src1 << shamt) : 0;
  }
  else
  {
    dest &= (src1 >> src2);
  }

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_FTSTScalarRotateScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    dest &= _lrotl(src1, (64U - src2) & 31U);
  }
  else
  {
    dest &= _lrotr(src1, src2);
  }

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ORImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  const uint32 result = src1 | src2;

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ORScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  const uint32 result = src1 | src2;

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ORImmediateShiftScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    const uint32 shamt = 64U - src2;
    dest |= (shamt < 32) ? (src1 << shamt) : 0;
  }
  else
  {
    dest |= (src1 >> src2);
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ORScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  dest |= (src1 >> src2);

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ORScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  dest |= (src1 << src2);

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ORScalarShiftScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    const uint32 shamt = 64U - src2;
    dest |= (shamt < 32) ? (src1 << shamt) : 0;
  }
  else
  {
    dest |= (src1 >> src2);
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ORScalarRotateScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    dest |= _lrotl(src1, (64U - src2) & 31U);
  }
  else
  {
    dest |= _lrotr(src1, src2);
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_EORImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  const uint32 result = src1 ^ src2;

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_EORScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  const uint32 result = src1 ^ src2;

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;

  if(!result)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((result >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_EORImmediateShiftScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    const uint32 shamt = 64U - src2;
    dest ^= (shamt < 32) ? (src1 << shamt) : 0;
  }
  else
  {
    dest ^= (src1 >> src2);
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_EORScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  dest ^= (src1 >> src2);

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_EORScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  dest ^= (src1 << src2);

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_EORScalarShiftScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    const uint32 shamt = 64U - src2;
    dest ^= (shamt < 32) ? (src1 << shamt) : 0;
  }
  else
  {
    dest ^= (src1 >> src2);
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_EORScalarRotateScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  mpe.cc &= ~(CC_ALU_ZERO | CC_ALU_NEGATIVE | CC_ALU_OVERFLOW);

  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (((int32)(pRegs[nuance.fields[FIELD_ALU_SRC2]] << 26)) >> 26) & 0x3FU;
        uint32 dest = pRegs[nuance.fields[FIELD_ALU_DEST]];

  if(src2 & 0x20)
  {
    dest ^= _lrotl(src1, (64U - src2) & 31U);
  }
  else
  {
    dest ^= _lrotr(src1, src2);
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = dest;

  if(!dest)
  {
    mpe.cc |= CC_ALU_ZERO;
  }
  else
  {
    mpe.cc |= ((dest >> 28) & CC_ALU_NEGATIVE);
  }
}

void Execute_ADDWCImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _addcarry_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src1, src2, &result) ? CC_ALU_CARRY : 0;

  if((~(src1 ^ src2)) & (src1 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_ADDWCScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _addcarry_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src1, src2, &result) ? CC_ALU_CARRY : 0;

  if((~(src1 ^ src2)) & (src1 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_ADDWCScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = ((int32)pRegs[nuance.fields[FIELD_ALU_SRC1]]) >> nuance.fields[FIELD_ALU_SRC2];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _addcarry_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src1, src2, &result) ? CC_ALU_CARRY : 0;

  if((~(src1 ^ src2)) & (src1 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_ADDWCScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]] << nuance.fields[FIELD_ALU_SRC2]; // do not cast to u64 before shift!
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _addcarry_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src1, src2, &result) ? CC_ALU_CARRY : 0;

  if((~(src1 ^ src2)) & (src1 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_SUBWCImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _subborrow_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_SUBWCImmediateReverse(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _subborrow_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_SUBWCScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _subborrow_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_SUBWCScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = ((int32)pRegs[nuance.fields[FIELD_ALU_SRC1]]) >> nuance.fields[FIELD_ALU_SRC2];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _subborrow_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_SUBWCScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]] << nuance.fields[FIELD_ALU_SRC2]; // do not cast to u64 before shift!
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _subborrow_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }

  mpe.regs[nuance.fields[FIELD_ALU_DEST]] = result;
}

void Execute_CMPWCImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = (uint32_t)nuance.fields[FIELD_ALU_SRC1];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _subborrow_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }
}

void Execute_CMPWCImmediateReverse(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = (uint32_t)nuance.fields[FIELD_ALU_SRC2];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _subborrow_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }
}

void Execute_CMPWCScalar(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_SRC2]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _subborrow_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }
}

void Execute_CMPWCScalarShiftRightImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = ((int32)pRegs[nuance.fields[FIELD_ALU_SRC1]]) >> nuance.fields[FIELD_ALU_SRC2];
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _subborrow_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }
}

void Execute_CMPWCScalarShiftLeftImmediate(MPE &mpe, const uint32 pRegs[48], const Nuance &nuance)
{
  const uint32 src1 = pRegs[nuance.fields[FIELD_ALU_SRC1]] << nuance.fields[FIELD_ALU_SRC2]; // do not cast to u64 before shift!
  const uint32 src2 = pRegs[nuance.fields[FIELD_ALU_DEST]];

  mpe.cc &= ~(CC_ALU_NEGATIVE | CC_ALU_OVERFLOW | CC_ALU_CARRY); // WC: do NOT clear Z here (spec: z unchanged if result==0, cleared otherwise)

  uint32 result;
  mpe.cc |= _subborrow_u32((mpe.tempCC & CC_ALU_CARRY) >> 1, src2, src1, &result) ? CC_ALU_CARRY : 0;

  if((src1 ^ src2) & (src2 ^ result) & 0x80000000u)
  {
    mpe.cc |= CC_ALU_OVERFLOW;
  }
  if(result & 0x80000000u)
  {
    mpe.cc |= CC_ALU_NEGATIVE;
  }
  if(result)
  {
    mpe.cc &= ~CC_ALU_ZERO;
  }
}
