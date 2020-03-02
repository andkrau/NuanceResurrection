#include "basetypes.h"
#include <windows.h>
#include "mpe.h"
#include "NativeCodeCache.h"
#include "PageMap.h"

#define x86Emit_MRM(mod,reg,rm) (*pEmitLoc++ = (((mod) << 6) | ((reg) << 3) | (rm)))
#define x86Emit_SIB(base, scale, index) (*pEmitLoc++ = (((scale) << 6) | ((index) << 3) | (base)))

NativeCodeCache::NativeCodeCache(uint32 _numBytes, uint32 _desiredTLBEntries)
    : numBytes(_numBytes == 0 ? DEFAULT_CODE_BUFFER_BYTES : _numBytes)
    , numTLBEntries(_desiredTLBEntries == 0 ? DEFAULT_NUM_TLB_ENTRIES : _desiredTLBEntries)
{
  ptrNativeCodeBuffer = (uint8 *)VirtualAlloc(NULL,numBytes,MEM_RESERVE | MEM_COMMIT,PAGE_EXECUTE_READWRITE/*PAGE_READWRITE*/); //!! pages are f.e. 64k wide, so this does not work
  pEmitLoc = ptrNativeCodeBuffer;

  if(ptrNativeCodeBuffer)
  {
    FlushInstructionCache(GetCurrentProcess(), pEmitLoc, numBytes); //!! should not be necessary, as each code block is flushed independently below in ReleaseBuffer(), but do it in case that ReleaseBuffer() is not the only thing called after emitting code

    warningThreshold = (uint32)(0.97 * numBytes);
  }
  else
  {
    numBytes = 0;
    warningThreshold = 0;
  }
}

NativeCodeCache::~NativeCodeCache()
{
  if(ptrNativeCodeBuffer)
  {
    VirtualFree(ptrNativeCodeBuffer,0,MEM_RELEASE);
  }
}

bool NativeCodeCache::ReleaseBuffer(NativeCodeCacheEntryPoint entryPoint, uint32 virtualAddress, uint32 nextVirtualAddress, uint32 newUsedBytes, uint32 packetCount, uint32 instructionCount, SuperBlockCompileType compileType, uint32 nextDelayCount, uint32 alignment)
{
  NativeCodeCacheEntry newEntry;
  newEntry.entryPoint = entryPoint;
  newEntry.virtualAddress = virtualAddress;
  newEntry.nextVirtualAddress = nextVirtualAddress;
  newEntry.codeSize = newUsedBytes;
  newEntry.numPackets = packetCount;
  newEntry.numInstructions = instructionCount;
  newEntry.compileType = compileType;
  newEntry.nextBranchDelayCount = nextDelayCount;
  newEntry.accessCount = 0;

  pageMap.UpdateEntry(newEntry);
  pEmitLoc = ((uint8 *)entryPoint) + newUsedBytes;
  /*DWORD oldProtect;
  if (!VirtualProtect(entryPoint, newUsedBytes, PAGE_EXECUTE_READ, &oldProtect) || oldProtect != PAGE_READWRITE) //!! pages are f.e. 64k wide, so this does not work
  {
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    MessageBox(NULL, "VirtualProtect failed", "VirtualProtect failed", MB_OK);
#endif
  }*/
  FlushInstructionCache(GetCurrentProcess(), entryPoint, newUsedBytes);

  if(alignment)
  {
    size_t address = (size_t)pEmitLoc;
    address = ((address + ((1 << alignment) - 1)) & ~((1 << alignment) - 1));
    pEmitLoc = (uint8 *)address;
  }

  return IsBeyondThreshold();
}


void NativeCodeCache::Flush()
{
  pageMap.Invalidate();
  pEmitLoc = ptrNativeCodeBuffer;
  /*DWORD oldProtect;
  if (!VirtualProtect(pEmitLoc, numBytes, PAGE_READWRITE, &oldProtect)) //!! pages are f.e. 64k wide, so this does not work
  {
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    MessageBox(NULL, "VirtualProtect failed", "VirtualProtect failed", MB_OK);
#endif
  }*/
  FlushInstructionCache(GetCurrentProcess(), pEmitLoc, numBytes); //!! should not be necessary, as each code block is flushed independently below in ReleaseBuffer(), but do it in case that ReleaseBuffer() is not the only thing called after emitting code
}

void NativeCodeCache::X86Emit_ModRegRM(x86ModType modType, x86ModReg regSpare, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(base > 7)
  {
    //[dword]: mod = 0, reg = regSpare, rm = 5
    x86Emit_MRM(x86ModType_mem,regSpare,x86BaseReg_sdword);
    *((uint32 *)pEmitLoc) = base;
    pEmitLoc += sizeof(uint32);
  }
  else
  {
    if(modType == x86ModType_reg)
    {
      //reg
      x86Emit_MRM(x86ModType_reg, regSpare, base);
    }
    else
    {
      if(index != x86IndexReg_none)
      {
        if(!disp)
        {
          //[sib]
          x86Emit_MRM(x86ModType_mem, regSpare, x86BaseReg_sib);
          x86Emit_SIB(base, scale, index);
        }
        else
        {
          //[sib + disp]
          if((disp < -128) || (disp > 127))
          {
            //[base + scale*index + disp32]
            x86Emit_MRM(x86ModType_mem_disp32, regSpare, x86BaseReg_sib);
            x86Emit_SIB(base, scale, index);
            *((int32 *)pEmitLoc) = disp;
            pEmitLoc += sizeof(int32);
          }
          else
          {
            //[sib + disp8]
            x86Emit_MRM(x86ModType_mem_disp8, regSpare, x86BaseReg_sib);
            x86Emit_SIB(base, scale, index);
            *pEmitLoc++ = (uint8)disp;
          }
        }
      }
      else
      {
        //[base + disp] or [base]
        if(!disp && (base != x86BaseReg_ebp))
        {
          //[base]
          x86Emit_MRM(x86ModType_mem, regSpare, base);
        }
        else
        {
          //[base + disp]
          if((disp < -128) || (disp > 127))
          {
            //[base + disp32]
            x86Emit_MRM(x86ModType_mem_disp32, regSpare, base);
            *((int32 *)pEmitLoc) = disp;
            pEmitLoc += sizeof(int32);
          }
          else
          {
            //[base + disp8]
            x86Emit_MRM(x86ModType_mem_disp8, regSpare, base);
            *pEmitLoc++ = (uint8)disp;
          }
        }
      }
    }
  }
}

void NativeCodeCache::X86Emit_Group1RR(x86Reg regDest, x86Reg regSrc, uint8 groupIndex)
{
  //OP r8, reg8, OP r16, reg16 or OP r32, reg32
  
  if(regSrc < x86Reg_ax)
  {
    //m8, r8
    *pEmitLoc++ = groupIndex << 3;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
  }
  else if(regSrc < x86Reg_eax)
  {
    //m16, r16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = (groupIndex << 3) + 1;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
  }
  else
  {
    //m32, r32
    *pEmitLoc++ = (groupIndex << 3) + 1;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
  }
}

void NativeCodeCache::X86Emit_Group1RM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex)
{
  //OP m8, reg8, OP m16, reg16 or OP m32, reg32

  if(regSrc < x86Reg_ax)
  {
    //m8, r8
    *pEmitLoc++ = (groupIndex << 3);
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
  }
  else if(regSrc < x86Reg_eax)
  {
    //m16, r16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = (groupIndex << 3) + 1;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
  }
  else
  {
    //m32, r32
    *pEmitLoc++ = (groupIndex << 3) + 1;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
  }
}

void NativeCodeCache::X86Emit_Group1MR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex)
{
  //OP reg8, m8, OP reg16, m16 or OP reg32, m32
  if(regDest < x86Reg_ax)
  {
    //r8, m8
    *pEmitLoc++ = (groupIndex << 3) + 0x02;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
  }
  else if(regDest < x86Reg_eax)
  {
    //r16, m16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = (groupIndex << 3) + 0x03;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
  }
  else
  {
    //r32, m32
    *pEmitLoc++ = (groupIndex << 3) + 0x03;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
  }
}

void NativeCodeCache::X86Emit_Group1IR(int32 imm, x86Reg regDest, uint8 groupIndex)
{
  if(regDest < x86Reg_ax)
  {
    //r8,imm8
    if(regDest == x86Reg_al)
    {
      //al, imm8
      *pEmitLoc++ = (groupIndex << 3) + 0x04;
      *pEmitLoc++ = (int8)imm;
    }
    else
    {
      //r8, imm8
      *pEmitLoc++ = 0x80;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
      *pEmitLoc++ = (int8)imm;
    }
  }
  else if(regDest < x86Reg_eax)
  {
    //r16, imm
    if((imm <= 127) && (imm >= -128))
    {
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 0x83;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
      *pEmitLoc++ = (int8)imm;
    }
    else if(regDest == x86Reg_ax)
    {
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = (groupIndex << 3) + 0x05;
      *((int16 *)pEmitLoc) = (int16)imm;
      pEmitLoc += sizeof(int16);
    }
    else
    {
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 0x81;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
      *((int16 *)pEmitLoc) = (int16)imm;
      pEmitLoc += sizeof(int16);
    }
  }
  else
  {
    //r32, imm
    if((imm <= 127) && (imm >= -128))
    {
      *pEmitLoc++ = 0x83;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
      *pEmitLoc++ = (int8)imm;
    }
    else if(regDest == x86Reg_eax)
    {
      *pEmitLoc++ = (groupIndex << 3) + 0x05;
      *((int32 *)pEmitLoc) = (int32)imm;
      pEmitLoc += sizeof(int32);

    }
    else
    {
      *pEmitLoc++ = 0x81;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
      *((int32 *)pEmitLoc) = (int32)imm;
      pEmitLoc += sizeof(int32);
    }
  }
}

void NativeCodeCache::X86Emit_Group1IM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex)
{
  if(ptrType == x86MemPtr_byte)
  {
    //mem8,imm8
    *pEmitLoc++ = 0x80;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
    *pEmitLoc++ = (int8)imm;
  }
  else if(ptrType == x86MemPtr_dword)
  {
    //mem32, imm
    if((imm <= 127) && (imm >= -128))
    {
      //mem32, imm8
      *pEmitLoc++ = 0x83;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
      *pEmitLoc++ = (int8)imm;
    }
    else
    {
      //mem32, imm32
      *pEmitLoc++ = 0x81;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
      *((int32 *)pEmitLoc) = (int32)imm;
      pEmitLoc += sizeof(int32);
    }
  }
  else
  {
    //mem16, imm
    if((imm <= 127) && (imm >= -128))
    {
      //mem16, imm8
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 0x83;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
      *pEmitLoc++ = imm;
    }
    else
    {
      //mem16, imm16
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 0x81;
      //ModRegRM(modtype, regspare, base, index, scale, disp)
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
      *((int16 *)pEmitLoc) = (int16)imm;
      pEmitLoc += sizeof(int16);

    }
  }
}

void NativeCodeCache::X86Emit_Group2IR(x86Reg regDest, uint8 shiftCount, uint8 groupIndex)
{
  //SHIFTOP r8, CL, SHIFTOP r16, CL, or SHIFTOP r32, CL
  
  if(regDest < x86Reg_ax)
  {
    if(shiftCount == 1)
    {
      *pEmitLoc++ = 0xD0;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
    }
    else
    {
      *pEmitLoc++ = 0xC0;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
      *pEmitLoc++ = shiftCount;
    }
  }
  else if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;

    if(shiftCount == 1)
    {
      *pEmitLoc++ = 0xD1;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
    }
    else
    {
      *pEmitLoc++ = 0xC1;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
      *pEmitLoc++ = shiftCount;
    }
  }
  else
  {
    if(shiftCount == 1)
    {
      *pEmitLoc++ = 0xD1;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
    }
    else
    {
      *pEmitLoc++ = 0xC1;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
      *pEmitLoc++ = shiftCount;
    }
  }
}

void NativeCodeCache::X86Emit_Group2IM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex)
{
  if(ptrType == x86MemPtr_byte)
  {
    if(shiftCount == 1)
    {
      *pEmitLoc++ = 0xD0;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
    }
    else
    {
      *pEmitLoc++ = 0xC0;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
      *pEmitLoc++ = shiftCount;
    }
  }
  else if(ptrType == x86MemPtr_word)
  {
    *pEmitLoc++ = 0x66;

    if(shiftCount == 1)
    {
      *pEmitLoc++ = 0xD1;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
    }
    else
    {
      *pEmitLoc++ = 0xC1;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
      *pEmitLoc++ = shiftCount;
    }
  }
  else
  {
    if(shiftCount == 1)
    {
      *pEmitLoc++ = 0xD1;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
    }
    else
    {
      *pEmitLoc++ = 0xC1;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
      *pEmitLoc++ = shiftCount;
    }
  }
}

void NativeCodeCache::X86Emit_Group2RR(x86Reg regDest, uint8 groupIndex)
{
  //SHIFTOP r8, CL, SHIFTOP r16, CL, or SHIFTOP r32, CL
  
  if(regDest < x86Reg_ax)
  {
    //m8, r8
    *pEmitLoc++ = 0xD2;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
  }
  else if(regDest < x86Reg_eax)
  {
    //m16, r16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xD3;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
  }
  else
  {
    //m32, r32
    *pEmitLoc++ = 0xD3;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)groupIndex,(regDest & 0x07));
  }
}

void NativeCodeCache::X86Emit_Group2RM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp, uint8 groupIndex)
{
  //OP m8, reg8, OP m16, reg16 or OP m32, reg32

  if(ptrType == x86MemPtr_byte)
  {
    *pEmitLoc++ = 0xD2;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
  }
  else if(ptrType == x86MemPtr_word)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xD3;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
  }
  else
  {
    *pEmitLoc++ = 0xD3;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)groupIndex,base,index,scale,disp);
  }
}

void NativeCodeCache::X86Emit_ADDRR(x86Reg regDest, x86Reg regSrc)
{
  X86Emit_Group1RR(regDest, regSrc, 0);
}

void NativeCodeCache::X86Emit_ADDRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1RM(regSrc, base, index, scale, disp, 0);
}

void NativeCodeCache::X86Emit_ADDMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1MR(regDest, base, index, scale, disp, 0);
}

void NativeCodeCache::X86Emit_ADDIR(int32 imm, x86Reg regDest)
{
  X86Emit_Group1IR(imm, regDest, 0);
}

void NativeCodeCache::X86Emit_ADDIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1IM(imm, ptrType, base, index, scale, disp, 0);
}

void NativeCodeCache::X86Emit_PUSHES()
{
  *pEmitLoc++ = 0x06;
}

void NativeCodeCache::X86Emit_POPES()
{
  *pEmitLoc++ = 0x07;
}

void NativeCodeCache::X86Emit_ORRR(x86Reg regDest, x86Reg regSrc)
{
  X86Emit_Group1RR(regDest, regSrc, 1);
}

void NativeCodeCache::X86Emit_ORRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1RM(regSrc, base, index, scale, disp, 1);
}

void NativeCodeCache::X86Emit_ORMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1MR(regDest, base, index, scale, disp, 1);
}

void NativeCodeCache::X86Emit_ORIR(int32 imm, x86Reg regDest)
{
  X86Emit_Group1IR(imm, regDest, 1);
}

void NativeCodeCache::X86Emit_ORIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1IM(imm, ptrType, base, index, scale, disp, 1);
}

void NativeCodeCache::X86Emit_PUSHCS()
{
  *pEmitLoc++ = 0x0E;
}

void NativeCodeCache::X86Emit_ADCRR(x86Reg regDest, x86Reg regSrc)
{
  X86Emit_Group1RR(regDest, regSrc, 2);
}

void NativeCodeCache::X86Emit_ADCRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1RM(regSrc, base, index, scale, disp, 2);
}

void NativeCodeCache::X86Emit_ADCMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1MR(regDest, base, index, scale, disp, 2);
}

void NativeCodeCache::X86Emit_ADCIR(int32 imm, x86Reg regDest)
{
  X86Emit_Group1IR(imm, regDest, 2);
}

void NativeCodeCache::X86Emit_ADCIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1IM(imm, ptrType, base, index, scale, disp, 2);
}

void NativeCodeCache::X86Emit_PUSHSS()
{
  *pEmitLoc++ = 0x16;
}

void NativeCodeCache::X86Emit_POPSS()
{
  *pEmitLoc++ = 0x17;
}

void NativeCodeCache::X86Emit_SBBRR(x86Reg regDest, x86Reg regSrc)
{
  X86Emit_Group1RR(regDest, regSrc, 3);
}

void NativeCodeCache::X86Emit_SBBRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1RM(regSrc, base, index, scale, disp, 3);
}

void NativeCodeCache::X86Emit_SBBMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1MR(regDest, base, index, scale, disp, 3);
}

void NativeCodeCache::X86Emit_SBBIR(int32 imm, x86Reg regDest)
{
  X86Emit_Group1IR(imm, regDest, 3);
}

void NativeCodeCache::X86Emit_SBBIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1IM(imm, ptrType, base, index, scale, disp, 3);
}

void NativeCodeCache::X86Emit_PUSHDS()
{
  *pEmitLoc++ = 0x1E;
}

void NativeCodeCache::X86Emit_POPDS()
{
  *pEmitLoc++ = 0x1F;
}

void NativeCodeCache::X86Emit_ANDRR(x86Reg regDest, x86Reg regSrc)
{
  X86Emit_Group1RR(regDest, regSrc, 4);
}

void NativeCodeCache::X86Emit_ANDRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1RM(regSrc, base, index, scale, disp, 4);
}

void NativeCodeCache::X86Emit_ANDMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1MR(regDest, base, index, scale, disp, 4);
}

void NativeCodeCache::X86Emit_ANDIR(int32 imm, x86Reg regDest)
{
  X86Emit_Group1IR(imm, regDest, 4);
}

void NativeCodeCache::X86Emit_ANDIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1IM(imm, ptrType, base, index, scale, disp, 4);
}

void NativeCodeCache::X86Emit_ES()
{
  *pEmitLoc++ = 0x26;
}

void NativeCodeCache::X86Emit_DAA()
{
  *pEmitLoc++ = 0x27;
}

void NativeCodeCache::X86Emit_SUBRR(x86Reg regDest, x86Reg regSrc)
{
  X86Emit_Group1RR(regDest, regSrc, 5);
}

void NativeCodeCache::X86Emit_SUBRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1RM(regSrc, base, index, scale, disp, 5);
}

void NativeCodeCache::X86Emit_SUBMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1MR(regDest, base, index, scale, disp, 5);
}

void NativeCodeCache::X86Emit_SUBIR(int32 imm, x86Reg regDest)
{
  X86Emit_Group1IR(imm, regDest, 5);
}

void NativeCodeCache::X86Emit_SUBIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1IM(imm, ptrType, base, index, scale, disp, 5);
}

void NativeCodeCache::X86Emit_CS()
{
  *pEmitLoc++ = 0x2E;
}

void NativeCodeCache::X86Emit_DAS()
{
  *pEmitLoc++ = 0x2F;
}

void NativeCodeCache::X86Emit_XORRR(x86Reg regDest, x86Reg regSrc)
{
  X86Emit_Group1RR(regDest, regSrc, 6);
}

void NativeCodeCache::X86Emit_XORRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1RM(regSrc, base, index, scale, disp, 6);
}

void NativeCodeCache::X86Emit_XORMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1MR(regDest, base, index, scale, disp, 6);
}

void NativeCodeCache::X86Emit_XORIR(int32 imm, x86Reg regDest)
{
  X86Emit_Group1IR(imm, regDest, 6);
}

void NativeCodeCache::X86Emit_XORIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1IM(imm, ptrType, base, index, scale, disp, 6);
}

void NativeCodeCache::X86Emit_SS()
{
  *pEmitLoc++ = 0x36;
}

void NativeCodeCache::X86Emit_AAA()
{
  *pEmitLoc++ = 0x37;
}

void NativeCodeCache::X86Emit_CMPRR(x86Reg regDest, x86Reg regSrc)
{
  X86Emit_Group1RR(regDest, regSrc, 7);
}

void NativeCodeCache::X86Emit_CMPRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1RM(regSrc, base, index, scale, disp, 7);
}

void NativeCodeCache::X86Emit_CMPMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1MR(regDest, base, index, scale, disp, 7);
}

void NativeCodeCache::X86Emit_CMPIR(int32 imm, x86Reg regDest)
{
  X86Emit_Group1IR(imm, regDest, 7);
}

void NativeCodeCache::X86Emit_CMPIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group1IM(imm, ptrType, base, index, scale, disp, 7);
}

void NativeCodeCache::X86Emit_DS()
{
  *pEmitLoc++ = 0x3E;
}

void NativeCodeCache::X86Emit_AAS()
{
  *pEmitLoc++ = 0x3F;
}

void NativeCodeCache::X86Emit_INCR(x86Reg reg)
{
  if(reg < x86Reg_ax)
  {
    *pEmitLoc++ = 0xFE;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)0,(reg & 0x07));
  }
  else if(reg < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x40 + (reg & 0x07);  
  }
  else
  {
    *pEmitLoc++ = 0x40 + (reg & 0x07);  
  }
}

void NativeCodeCache::X86Emit_DECR(x86Reg reg)
{
  if(reg < x86Reg_ax)
  {
    *pEmitLoc++ = 0xFE;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)1,(reg & 0x07));
  }
  else if(reg < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x48 + (reg & 0x07);  
  }
  else
  {
    *pEmitLoc++ = 0x48 + (reg & 0x07);  
  }
}

void NativeCodeCache::X86Emit_PUSHR(x86Reg reg)
{
  if(reg < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x50 + (reg & 0x07);  
}

void NativeCodeCache::X86Emit_POPR(x86Reg reg)
{
  if(reg < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x58 + (reg & 0x07);  
}

void NativeCodeCache::X86Emit_PUSHAW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0x60;
}

void NativeCodeCache::X86Emit_PUSHAD()
{
  *pEmitLoc++ = 0x60;
}

void NativeCodeCache::X86Emit_POPAW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0x61;
}

void NativeCodeCache::X86Emit_POPAD()
{
  *pEmitLoc++ = 0x61;
}

void NativeCodeCache::X86Emit_FS()
{
  *pEmitLoc++ = 0x64;
}

void NativeCodeCache::X86Emit_GS()
{
  *pEmitLoc++ = 0x65;
}

void NativeCodeCache::X86Emit_OPSIZE()
{
  *pEmitLoc++ = 0x66;
}

void NativeCodeCache::X86Emit_ADSIZE()
{
  *pEmitLoc++ = 0x67;
}

void NativeCodeCache::X86Emit_PUSHID(int32 imm)
{
  *pEmitLoc++ = 0x68;
  *((int32 *)pEmitLoc++) = imm;
}

void NativeCodeCache::X86Emit_PUSHIW(int16 imm)
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0x68;
  *((int16 *)pEmitLoc++) = imm;
}

void NativeCodeCache::X86Emit_IMULMRR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //imul reg16, r16
    *pEmitLoc++ = 0x66;
  }
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xAF;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
}

void NativeCodeCache::X86Emit_IMULRRR(x86Reg regDest, x86Reg regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //imul reg16, r16
    *pEmitLoc++ = 0x66;
  }
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xAF;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_IMULIRR(x86Reg regDest, int32 imm, x86Reg regSrc)
{
  if(regSrc < x86Reg_eax)
  {
    //reg16, r8, imm8 or reg16, r16, imm16
    if((imm >= -128) && (imm <= 127))
    {
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 0x6B;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
      *pEmitLoc++ = (int8)imm;
    }
    else
    {
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 0x69;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
      *((int16 *)pEmitLoc) = (int16)imm;
      pEmitLoc += sizeof(int16);
    }
  }
  else
  {
    //reg32, r32, imm8 or reg32, r32, imm32
    if((imm >= -128) && (imm <= 127))
    {
      *pEmitLoc++ = 0x6B;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
      *pEmitLoc++ = (int8)imm;
    }
    else
    {
      *pEmitLoc++ = 0x69;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
      *((int32 *)pEmitLoc) = (int32)imm;
      pEmitLoc += sizeof(int32);
    }
  }
}

void NativeCodeCache::X86Emit_IMULIMR(x86Reg regDest, int32 imm, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //reg16, m8, imm8 or reg16, m16, imm16
    if((imm >= -128) && (imm <= 127))
    {
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 0x6B;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
      *pEmitLoc++ = (int8)imm;
    }
    else
    {
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 0x69;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
      *((int16 *)pEmitLoc) = (int16)imm;
      pEmitLoc += sizeof(int16);
    }
  }
  else
  {
    //reg32, m32, imm8 or reg32, m32, imm32
    if((imm >= -128) && (imm <= 127))
    {
      *pEmitLoc++ = 0x6B;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
      *pEmitLoc++ = (int8)imm;
    }
    else
    {
      *pEmitLoc++ = 0x69;
      X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
      *((int32 *)pEmitLoc) = (int32)imm;
      pEmitLoc += sizeof(int32);
    }
  }
}

void NativeCodeCache::X86Emit_PUSHIB(int8 imm)
{
  *pEmitLoc++ = 0x6A;
  *pEmitLoc++ = imm;
}

void NativeCodeCache::X86Emit_INSB()
{
  *pEmitLoc++ = 0x6C;
}

void NativeCodeCache::X86Emit_INSW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0x6C;
}

void NativeCodeCache::X86Emit_INSD()
{
  *pEmitLoc++ = 0x6C;
}

void NativeCodeCache::X86Emit_OUTSB()
{
  *pEmitLoc++ = 0x6E;
}

void NativeCodeCache::X86Emit_OUTSW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0x6E;
}

void NativeCodeCache::X86Emit_OUTSD()
{
  *pEmitLoc++ = 0x6E;
}

void NativeCodeCache::X86Emit_FEMMS()
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x0E;
}

void NativeCodeCache::X86Emit_EMMS()
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x77;
}

void NativeCodeCache::X86Emit_JCC(uint8 *pTarget, int8 conditionCode)
{
  int32 pOffset = (int32)(pTarget - (pEmitLoc + 2));
  int32 pOffsetNear = (int32)(pTarget - (pEmitLoc + 6));
  
  if((pOffset >= -128) && (pOffset <= 127))
  {
    *pEmitLoc++ = 0x70 + conditionCode;
    *pEmitLoc++ = (int8)pOffset;
  }
  else
  {
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0x80 + conditionCode;
    *((int32 *)pEmitLoc) = pOffsetNear;
    pEmitLoc += sizeof(int32);
  }
}

void NativeCodeCache::X86Emit_JCC_Label(PatchManager &patchMgr, int8 conditionCode, uint32 labelIndex)
{
  if(labelIndex >= patchMgr.numLabels)
  {
    patchMgr.AddPatch(pEmitLoc + 2, PatchType_Rel32, pEmitLoc + 6, labelIndex);
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0x80 + conditionCode;
    *((int32 *)pEmitLoc) = 0;
    pEmitLoc += sizeof(int32);
  }
  else
  {
    X86Emit_JCC(patchMgr.GetLabelPointer(labelIndex),conditionCode);
  }
}

void NativeCodeCache::X86Emit_JO(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,0);
}

void NativeCodeCache::X86Emit_JNO(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,1);
}

void NativeCodeCache::X86Emit_JB(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,2);
}

void NativeCodeCache::X86Emit_JNB(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,3);
}

void NativeCodeCache::X86Emit_JZ(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,4);
}

void NativeCodeCache::X86Emit_JNZ(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,5);
}

void NativeCodeCache::X86Emit_JBE(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,6);
}

void NativeCodeCache::X86Emit_JNBE(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,7);
}

void NativeCodeCache::X86Emit_JS(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,8);
}

void NativeCodeCache::X86Emit_JNS(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,9);
}

void NativeCodeCache::X86Emit_JP(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,10);
}

void NativeCodeCache::X86Emit_JNP(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,11);
}

void NativeCodeCache::X86Emit_JL(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,12);
}

void NativeCodeCache::X86Emit_JNL(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,13);
}

void NativeCodeCache::X86Emit_JLE(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,14);
}

void NativeCodeCache::X86Emit_JNLE(uint8 *pTarget)
{
  X86Emit_JCC(pTarget,15);
}

void NativeCodeCache::X86Emit_TESTRR(x86Reg regDest, x86Reg regSrc)
{
  //OP r8, reg8, OP r16, reg16 or OP r32, reg32
  if(regSrc < x86Reg_ax)
  {
    //r8, r8
    *pEmitLoc++ = 0x84;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
  }
  else if(regSrc < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x85;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
  }
  else
  {
    //r32, r32
    *pEmitLoc++ = 0x85;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
  }
}

void NativeCodeCache::X86Emit_TESTRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  //OP m8, reg8, OP m16, reg16 or OP m32, reg32

  if(regSrc < x86Reg_ax)
  {
    //m8, r8
    *pEmitLoc++ = 0x84;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
  }
  else if(regSrc < x86Reg_eax)
  {
    //m16, r16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x85;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
  }
  else
  {
    //m32, r32
    *pEmitLoc++ = 0x85;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
  }
}

void NativeCodeCache::X86Emit_XCHGRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  //OP m8, reg8, OP m16, reg16 or OP m32, reg32

  if(regSrc < x86Reg_ax)
  {
    //m8, r8
    *pEmitLoc++ = 0x86;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
  }
  else if(regSrc < x86Reg_eax)
  {
    //m16, r16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x87;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
  }
  else
  {
    //m32, r32
    *pEmitLoc++ = 0x87;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index,scale,disp);
  }
}

void NativeCodeCache::X86Emit_MOVRR(x86Reg regDest, x86Reg regSrc)
{
  X86Emit_Group1RR(regDest,regSrc,17);
}

void NativeCodeCache::X86Emit_LEA(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x8D;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index,scale,disp);
}

void NativeCodeCache::X86Emit_POPM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(ptrType == x86MemPtr_word)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x8F;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)0,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_NOP()
{
  //Alias for XCHG rAX, rAX
  *pEmitLoc++ = 0x90;
}

void NativeCodeCache::X86Emit_XCHGRR(x86Reg reg1, x86Reg reg2)
{
  if((reg2 == x86Reg_eax) || (reg2 == x86Reg_ax))
  {
    //r32, eax or r16, ax
    if(reg2 == x86Reg_ax)
    {
      *pEmitLoc++ = 0x66;
    }
    *pEmitLoc++ = 0x90 + (reg1 & 0x07);
  }
  else if((reg1 == x86Reg_eax) || (reg1 == x86Reg_ax))
  {
    //eax, r32 or ax, r16
    if(reg1 == x86Reg_ax)
    {
      *pEmitLoc++ = 0x66;
    }
    *pEmitLoc++ = 0x90 + (reg2 & 0x07);
  }
  else
  {
    if(reg1 < x86Reg_ax)
    {
      //r8, r8
      *pEmitLoc++ = 86;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(reg2 & 0x07),(reg1 & 0x07));
    }
    else if(reg1 < x86Reg_eax)
    {
      //r16, r16
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 87;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(reg2 & 0x07),(reg1 & 0x07));
    }
    else
    {
      //r32, r32
      *pEmitLoc++ = 87;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(reg2 & 0x07),(reg1 & 0x07));
    }
  }
}

void NativeCodeCache::X86Emit_CBW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0x98;
}

void NativeCodeCache::X86Emit_CWDE()
{
  *pEmitLoc++ = 0x98;
}

void NativeCodeCache::X86Emit_CWD()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0x99;
}

void NativeCodeCache::X86Emit_CDQ()
{
  *pEmitLoc++ = 0x99;
}

void NativeCodeCache::X86Emit_CALLI(uint32 offset, uint16 seg)
{
  offset -= (uint32)(pEmitLoc + 5);

  if(seg == 0)
  {
    //CALL imm32
    *pEmitLoc++ = 0xE8;
    *((uint32 *)pEmitLoc) = offset;
    pEmitLoc += sizeof(int32);

  }
  else
  {
    //CALL imm:imm32
    *pEmitLoc++ = 0x9A;
    *((uint32 *)pEmitLoc) = offset;
    pEmitLoc += sizeof(int32);
    *((uint16 *)pEmitLoc) = seg;
    pEmitLoc += sizeof(int16);
  }
}

void NativeCodeCache::X86Emit_JMPI(uint8 *target, uint16 seg)
{
  int32 offset = (int32)(target - (pEmitLoc + 2));
  int32 offsetNear = (int32)(target - (pEmitLoc + 5));

  if(seg == 0)
  {
    if((offset >= -128) && (offset < 128))
    {
      *pEmitLoc++ = 0xEB;
      *pEmitLoc++ = (int8)offset;
    }
    else
    {
      *pEmitLoc++ = 0xE9;
      *((int32 *)pEmitLoc) = offsetNear;
      pEmitLoc += sizeof(int32);
    }
  }
  else
  {
    //JMP imm:imm32
    *pEmitLoc++ = 0xEA;
    *((uint32 *)pEmitLoc) = offset;
    pEmitLoc += sizeof(uint32);
    *((uint16 *)pEmitLoc) = seg;
    pEmitLoc += sizeof(uint16);
  }
}

void NativeCodeCache::X86Emit_JMPI_Label(PatchManager &patchMgr, uint32 labelIndex)
{
  if(labelIndex >= patchMgr.numPatches)
  {
    patchMgr.AddPatch(pEmitLoc + 1, PatchType_Rel32, pEmitLoc + 5, labelIndex);
    *pEmitLoc++ = 0xE9;
    *((int32 *)pEmitLoc) = 0;
    pEmitLoc += sizeof(int32);
  }
  else
  {
    X86Emit_JMPI(patchMgr.GetLabelPointer(labelIndex),0);
  }
}

void NativeCodeCache::X86Emit_WAIT()
{
  *pEmitLoc++ = 0x9B;
}

void NativeCodeCache::X86Emit_PUSHFW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0x9C;
}

void NativeCodeCache::X86Emit_PUSHFD()
{
  *pEmitLoc++ = 0x9C;
}

void NativeCodeCache::X86Emit_POPFW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0x9D;
}

void NativeCodeCache::X86Emit_POPFD()
{
  *pEmitLoc++ = 0x9D;
}

void NativeCodeCache::X86Emit_SAHF()
{
  *pEmitLoc++ = 0x9E;
}

void NativeCodeCache::X86Emit_LAHF()
{
  *pEmitLoc++ = 0x9F;
}

void NativeCodeCache::X86Emit_MOVMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if((base > 7) && ((regDest == x86Reg_eax) || (regDest == x86Reg_ax) || (regDest == x86Reg_al)))
  {
    if(regDest == x86Reg_al)
    {
      *pEmitLoc++ = 0xA0;
    }
    else
    {
      if(regDest == x86Reg_ax)
      {
        *pEmitLoc++ = 0x66;
      }
      *pEmitLoc++ = 0xA1;
    }
    *((uint32 *)pEmitLoc) = base;
    pEmitLoc += sizeof(int32);

  }
  else
  {
    X86Emit_Group1MR(regDest,base,index,scale,disp,17);
  }
}

void NativeCodeCache::X86Emit_MOVRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if((base > 7) && ((regSrc == x86Reg_eax) || (regSrc == x86Reg_ax) || (regSrc == x86Reg_al)))
  {
    if(regSrc == x86Reg_al)
    {
      *pEmitLoc++ = 0xA2;
    }
    else
    {
      if(regSrc == x86Reg_ax)
      {
        *pEmitLoc++ = 0x66;
      }
      *pEmitLoc++ = 0xA3;
    }
    *((uint32 *)pEmitLoc) = base;
    pEmitLoc += sizeof(int32);
  }
  else
  {
    X86Emit_Group1RM(regSrc,base,index,scale,disp,17);
  }
}

void NativeCodeCache::X86Emit_MOVIR(int32 imm, x86Reg regDest)
{
  if(regDest < x86Reg_ax)
  {
    //r8,imm8
    *pEmitLoc++ = 0xB0 + (regDest & 0x07);
    *pEmitLoc++ = (int8)imm;
  }
  else if(regDest < x86Reg_eax)
  {
    //r16, imm16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xB8 + (regDest & 0x07);
    *((int16 *)pEmitLoc) = (int16)imm;
    pEmitLoc += sizeof(int16);

  }
  else
  {
    //r32, imm32
    *pEmitLoc++ = 0xB8 + (regDest & 0x07);
    *((int32 *)pEmitLoc) = imm;
    pEmitLoc += sizeof(int32);
  }
}

void NativeCodeCache::X86Emit_MOVSB()
{
  *pEmitLoc++ = 0xA4;
}

void NativeCodeCache::X86Emit_MOVSW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0xA5;
}

void NativeCodeCache::X86Emit_MOVSD()
{
  *pEmitLoc++ = 0xA5;
}

void NativeCodeCache::X86Emit_CMPSB()
{
  *pEmitLoc++ = 0xA6;
}

void NativeCodeCache::X86Emit_CMPSW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0xA7;
}

void NativeCodeCache::X86Emit_CMPSD()
{
  *pEmitLoc++ = 0xA7;
}

void NativeCodeCache::X86Emit_STOSB()
{
  *pEmitLoc++ = 0xAA;
}

void NativeCodeCache::X86Emit_STOSW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0xAB;
}

void NativeCodeCache::X86Emit_STOSD()
{
  *pEmitLoc++ = 0xAB;
}

void NativeCodeCache::X86Emit_LODSB()
{
  *pEmitLoc++ = 0xAC;
}

void NativeCodeCache::X86Emit_LODSW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0xAD;
}

void NativeCodeCache::X86Emit_LODSD()
{
  *pEmitLoc++ = 0xAD;
}

void NativeCodeCache::X86Emit_SCASB()
{
  *pEmitLoc++ = 0xAD;
}

void NativeCodeCache::X86Emit_SCASW()
{
  *pEmitLoc++ = 0x66;
  *pEmitLoc++ = 0xAF;
}

void NativeCodeCache::X86Emit_SCASD()
{
  *pEmitLoc++ = 0xAF;
}

void NativeCodeCache::X86Emit_ROLIR(x86Reg regDest, uint8 shiftCount)
{
  X86Emit_Group2IR(regDest, shiftCount, 0);
}

void NativeCodeCache::X86Emit_RORIR(x86Reg regDest, uint8 shiftCount)
{
  X86Emit_Group2IR(regDest, shiftCount, 1);
}

void NativeCodeCache::X86Emit_RCLIR(x86Reg regDest, uint8 shiftCount)
{
  X86Emit_Group2IR(regDest, shiftCount, 2);
}

void NativeCodeCache::X86Emit_RCRIR(x86Reg regDest, uint8 shiftCount)
{
  X86Emit_Group2IR(regDest, shiftCount, 3);
}

void NativeCodeCache::X86Emit_SHLIR(x86Reg regDest, uint8 shiftCount)
{
  X86Emit_Group2IR(regDest, shiftCount, 4);
}

void NativeCodeCache::X86Emit_SHRIR(x86Reg regDest, uint8 shiftCount)
{
  X86Emit_Group2IR(regDest, shiftCount, 5);
}

void NativeCodeCache::X86Emit_SALIR(x86Reg regDest, uint8 shiftCount)
{
  X86Emit_Group2IR(regDest, shiftCount, 6);
}

void NativeCodeCache::X86Emit_SARIR(x86Reg regDest, uint8 shiftCount)
{
  X86Emit_Group2IR(regDest, shiftCount, 7);
}

void NativeCodeCache::X86Emit_ROLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2IM(ptrType, shiftCount, base, index, scale, disp, 0);
}

void NativeCodeCache::X86Emit_RORIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2IM(ptrType, shiftCount, base, index, scale, disp, 1);
}

void NativeCodeCache::X86Emit_RCLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2IM(ptrType, shiftCount, base, index, scale, disp, 2);
}

void NativeCodeCache::X86Emit_RCRIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2IM(ptrType, shiftCount, base, index, scale, disp, 3);
}

void NativeCodeCache::X86Emit_SHLIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2IM(ptrType, shiftCount, base, index, scale, disp, 4);
}

void NativeCodeCache::X86Emit_SHRIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2IM(ptrType, shiftCount, base, index, scale, disp, 5);
}

void NativeCodeCache::X86Emit_SALIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2IM(ptrType, shiftCount, base, index, scale, disp, 6);
}

void NativeCodeCache::X86Emit_SARIM(x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2IM(ptrType, shiftCount, base, index, scale, disp, 7);
}

void NativeCodeCache::X86Emit_ROLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2RM(ptrType, base, index, scale, disp, 0);
}

void NativeCodeCache::X86Emit_RORRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2RM(ptrType, base, index, scale, disp, 1);
}

void NativeCodeCache::X86Emit_RCLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2RM(ptrType, base, index, scale, disp, 2);
}

void NativeCodeCache::X86Emit_RCRRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2RM(ptrType, base, index, scale, disp, 3);
}

void NativeCodeCache::X86Emit_SHLRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2RM(ptrType, base, index, scale, disp, 4);
}

void NativeCodeCache::X86Emit_SHRRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2RM(ptrType, base, index, scale, disp, 5);
}

void NativeCodeCache::X86Emit_SALRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2RM(ptrType, base, index, scale, disp, 6);
}

void NativeCodeCache::X86Emit_SARRM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  X86Emit_Group2RM(ptrType, base, index, scale, disp, 7);
}

void NativeCodeCache::X86Emit_RETN(uint16 iw)
{
  if(iw == 0)
  {
    *pEmitLoc++ = 0xC3;
  }
  else
  {
    *pEmitLoc++ = 0xC2;
    *((uint16 *)pEmitLoc) = iw;
    pEmitLoc += sizeof(uint16);
  }
}

void NativeCodeCache::X86Emit_MOVIM(int32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(ptrType == x86MemPtr_byte)
  {
    //m8,imm8
    *pEmitLoc++ = 0xC6;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)0,base,index,scale,disp);
    *pEmitLoc++ = (int8)imm;
  }
  else if(ptrType == x86MemPtr_word)
  {
    //m16, imm16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xC7;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)0,base,index,scale,disp);
    *((int16 *)pEmitLoc) = (int16)imm;
    pEmitLoc += sizeof(int16);

  }
  else
  {
    //m32, imm32
    *pEmitLoc++ = 0xC7;
    X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)0,base,index,scale,disp);
    *((int32 *)pEmitLoc) = imm;
    pEmitLoc += sizeof(int32);
  }
}

void NativeCodeCache::X86Emit_LEAVE()
{
  *pEmitLoc++ = 0xC9;
}

void NativeCodeCache::X86Emit_RETF(uint16 iw)
{
  if(iw == 0)
  {
    *pEmitLoc++ = 0xCB;
  }
  else
  {
    *pEmitLoc++ = 0xCA;
    *((uint16 *)pEmitLoc) = iw;
    pEmitLoc += sizeof(uint16);
  }
}
void NativeCodeCache::X86Emit_INT3()
{
  *pEmitLoc++ = 0xCC;
}

void NativeCodeCache::X86Emit_INT(int8 vector)
{
  *pEmitLoc++ = 0xCD;
  *pEmitLoc++ = vector;
}

void NativeCodeCache::X86Emit_INTO()
{
  *pEmitLoc++ = 0xCE;
}

void NativeCodeCache::X86Emit_IRET()
{
  *pEmitLoc++ = 0xCF;
}

void NativeCodeCache::X86Emit_ROLRR(x86Reg regDest)
{
  X86Emit_Group2RR(regDest, 0);
}

void NativeCodeCache::X86Emit_RORRR(x86Reg regDest)
{
  X86Emit_Group2RR(regDest, 1);
}

void NativeCodeCache::X86Emit_RCLRR(x86Reg regDest)
{
  X86Emit_Group2RR(regDest, 2);
}

void NativeCodeCache::X86Emit_RCRRR(x86Reg regDest)
{
  X86Emit_Group2RR(regDest, 3);
}

void NativeCodeCache::X86Emit_SHLRR(x86Reg regDest)
{
  X86Emit_Group2RR(regDest, 4);
}

void NativeCodeCache::X86Emit_SHRRR(x86Reg regDest)
{
  X86Emit_Group2RR(regDest, 5);
}

void NativeCodeCache::X86Emit_SALRR(x86Reg regDest)
{
  X86Emit_Group2RR(regDest, 6);
}

void NativeCodeCache::X86Emit_SARRR(x86Reg regDest)
{
  X86Emit_Group2RR(regDest, 7);
}

void NativeCodeCache::X86Emit_AAM(uint8 divisor)
{
  *pEmitLoc++ = 0xD4;
  if(divisor != 10)
  {
    *pEmitLoc++ = divisor;
  }
}

void NativeCodeCache::X86Emit_AAD(uint8 divisor)
{
  *pEmitLoc++ = 0xD5;
  if(divisor != 10)
  {
    *pEmitLoc++ = divisor;
  }
}

void NativeCodeCache::X86Emit_XLAT()
{
  *pEmitLoc++ = 0xD7;
}

void NativeCodeCache::X86Emit_ESC0()
{
  *pEmitLoc++ = 0xD8;
}

void NativeCodeCache::X86Emit_ESC1()
{
  *pEmitLoc++ = 0xD9;
}

void NativeCodeCache::X86Emit_ESC2()
{
  *pEmitLoc++ = 0xDA;
}

void NativeCodeCache::X86Emit_ESC3()
{
  *pEmitLoc++ = 0xDB;
}

void NativeCodeCache::X86Emit_ESC4()
{
  *pEmitLoc++ = 0xDC;
}

void NativeCodeCache::X86Emit_ESC5()
{
  *pEmitLoc++ = 0xDD;
}

void NativeCodeCache::X86Emit_ESC6()
{
  *pEmitLoc++ = 0xDE;
}

void NativeCodeCache::X86Emit_ESC7()
{
  *pEmitLoc++ = 0xDF;
}

void NativeCodeCache::X86Emit_LOOPNE(uint8 *pTarget)
{
  int32 pOffset = (int32)(pTarget - (pEmitLoc + 2));
  
  *pEmitLoc++ = 0xE0;
  *pEmitLoc++ = (int8)pOffset;
}

void NativeCodeCache::X86Emit_LOOPNE_Label(PatchManager &patchMgr, uint32 labelIndex)
{
  if(labelIndex >= patchMgr.numLabels)
  {
    patchMgr.AddPatch(pEmitLoc + 1, PatchType_Rel8, pEmitLoc + 2, labelIndex);
    *pEmitLoc++ = 0xE2;
    *pEmitLoc++ = 0;
  }
  else
  {
    X86Emit_LOOP(patchMgr.GetLabelPointer(labelIndex));
  }
}

void NativeCodeCache::X86Emit_LOOPE(uint8 *pTarget)
{
  int32 pOffset = (int32)(pTarget - (pEmitLoc + 2));
  
  *pEmitLoc++ = 0xE0;
  *pEmitLoc++ = (int8)pOffset;
}

void NativeCodeCache::X86Emit_LOOPE_Label(PatchManager &patchMgr, uint32 labelIndex)
{
  if(labelIndex >= patchMgr.numLabels)
  {
    patchMgr.AddPatch(pEmitLoc + 1, PatchType_Rel8, pEmitLoc + 2, labelIndex);
    *pEmitLoc++ = 0xE0;
    *pEmitLoc++ = 0;
  }
  else
  {
    X86Emit_LOOPE(patchMgr.GetLabelPointer(labelIndex));
  }
}

void NativeCodeCache::X86Emit_LOOP(uint8 *pTarget)
{
  int32 pOffset = (int32)(pTarget - (pEmitLoc + 2));
  
  *pEmitLoc++ = 0xE2;
  *pEmitLoc++ = (int8)pOffset;
}

void NativeCodeCache::X86Emit_LOOP_Label(PatchManager &patchMgr, uint32 labelIndex)
{
  if(labelIndex >= patchMgr.numLabels)
  {
    patchMgr.AddPatch(pEmitLoc + 1, PatchType_Rel8, pEmitLoc + 2, labelIndex);
    *pEmitLoc++ = 0xE2;
    *pEmitLoc++ = 0;
  }
  else
  {
    X86Emit_LOOP(patchMgr.GetLabelPointer(labelIndex));
  }
}

void NativeCodeCache::X86Emit_JCXZ(uint8 *pTarget)
{
  int32 pOffset = (int32)(pTarget - (pEmitLoc + 3));
  
  //ADDRESS Prefix
  *pEmitLoc++ = 0x67;
  *pEmitLoc++ = 0xE3;
  *pEmitLoc++ = (int8)pOffset;
}

void NativeCodeCache::X86Emit_JCXZ_Label(PatchManager &patchMgr, uint32 labelIndex)
{
  if(labelIndex >= patchMgr.numLabels)
  {
    patchMgr.AddPatch(pEmitLoc + 2, PatchType_Rel8, pEmitLoc + 3, labelIndex);
    *pEmitLoc++ = 0x67;
    *pEmitLoc++ = 0xE3;
    *pEmitLoc++ = 0;
  }
  else
  {
    X86Emit_LOOP(patchMgr.GetLabelPointer(labelIndex));
  }
}

void NativeCodeCache::X86Emit_JECXZ(uint8 *pTarget)
{
  int32 pOffset = (int32)(pTarget - (pEmitLoc + 2));
  
  *pEmitLoc++ = 0xE3;
  *pEmitLoc++ = (int8)pOffset;
}

void NativeCodeCache::X86Emit_JECXZ_Label(PatchManager &patchMgr, uint32 labelIndex)
{
  if(labelIndex >= patchMgr.numLabels)
  {
    patchMgr.AddPatch(pEmitLoc + 1, PatchType_Rel8, pEmitLoc + 2, labelIndex);
    *pEmitLoc++ = 0xE3;
    *pEmitLoc++ = 0;
  }
  else
  {
    X86Emit_JECXZ(patchMgr.GetLabelPointer(labelIndex));
  }
}

void NativeCodeCache::X86Emit_INI(x86Reg regDest, uint8 port)
{
  if(regDest < x86Reg_ax)
  {
    *pEmitLoc++ = 0xE4;
    *pEmitLoc++ = port;
  }
  else if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xE5;
    *pEmitLoc++ = port;
  }
  else
  {
    *pEmitLoc++ = 0xE5;
    *pEmitLoc++ = port;
  }
}

void NativeCodeCache::X86Emit_OUTI(x86Reg regDest, uint8 data)
{
  if(regDest < x86Reg_ax)
  {
    *pEmitLoc++ = 0xE6;
    *pEmitLoc++ = data;
  }
  else if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xE7;
    *pEmitLoc++ = data;
  }
  else
  {
    *pEmitLoc++ = 0xE7;
    *pEmitLoc++ = data;
  }
}

void NativeCodeCache::X86Emit_INR(x86Reg regDest)
{
  if(regDest < x86Reg_ax)
  {
    *pEmitLoc++ = 0xEC;
  }
  else if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xED;
  }
  else
  {
    *pEmitLoc++ = 0xED;
  }
}

void NativeCodeCache::X86Emit_OUTR(x86Reg regDest)
{
  if(regDest < x86Reg_ax)
  {
    *pEmitLoc++ = 0xEE;
  }
  else if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xEF;
  }
  else
  {
    *pEmitLoc++ = 0xEF;
  }
}

void NativeCodeCache::X86Emit_LOCK()
{
  *pEmitLoc++ = 0xF0;
}

void NativeCodeCache::X86Emit_INT1()
{
  *pEmitLoc++ = 0xF1;
}

void NativeCodeCache::X86Emit_REPNE()
{
  *pEmitLoc++ = 0xF2;
}

void NativeCodeCache::X86Emit_REPE()
{
  *pEmitLoc++ = 0xF3;
}

void NativeCodeCache::X86Emit_HLT()
{
  *pEmitLoc++ = 0xF4;
}

void NativeCodeCache::X86Emit_CMC()
{
  *pEmitLoc++ = 0xF5;
}

void NativeCodeCache::X86Emit_NOTR(x86Reg regDest)
{
  if(regDest < x86Reg_ax)
  {
    *pEmitLoc++ = 0xF6;
  }
  else if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xF7;
  }
  else
  {
    *pEmitLoc++ = 0xF7;
  }
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,regDest & 0x07);
}

void NativeCodeCache::X86Emit_NOTM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(ptrType == x86MemPtr_byte)
  {
    *pEmitLoc++ = 0xF6;
  }
  else if(ptrType == x86MemPtr_word)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xF7;
  }
  else
  {
    *pEmitLoc++ = 0xF7;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_IMULRR(x86Reg regSrc)
{
  if(regSrc < x86Reg_ax)
  {
    *pEmitLoc++ = 0xF6;
  }
  else
  {
    if(regSrc < x86Reg_eax)
    {
      *pEmitLoc++ = 0x66;
    }
    *pEmitLoc++ = 0xF7;
  }
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)5,regSrc & 0x07);
}

void NativeCodeCache::X86Emit_IMULMR(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(ptrType == x86MemPtr_byte)
  {
    *pEmitLoc++ = 0xF6;
  }
  else
  {
    if(ptrType == x86MemPtr_word)
    {
      *pEmitLoc++ = 0x66;
    }
    *pEmitLoc++ = 0xF7;
  }
  
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)5,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_NEGR(x86Reg regDest)
{
  if(regDest < x86Reg_ax)
  {
    *pEmitLoc++ = 0xF6;
  }
  else if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xF7;
  }
  else
  {
    *pEmitLoc++ = 0xF7;
  }
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)3,regDest & 0x07);
}

void NativeCodeCache::X86Emit_TESTIR(uint32 imm, x86Reg regSrc)
{
  if(regSrc < x86Reg_ax)
  {
    if(regSrc == x86Reg_al)
    {
      *pEmitLoc++ = 0xA8;
      *pEmitLoc++ = (uint8)imm;
    }
    else
    {
      *pEmitLoc++ = 0xF6;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)0,regSrc & 0x07);
      *pEmitLoc++ = (uint8)imm;
    }
  }
  else if(regSrc < x86Reg_eax)
  {
    if(regSrc == x86Reg_ax)
    {
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 0xA9;
      *((uint16 *)pEmitLoc) = (uint16)imm;
      pEmitLoc += sizeof(uint16);
    }
    else
    {
      *pEmitLoc++ = 0x66;
      *pEmitLoc++ = 0xF7;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)0,regSrc & 0x07);
      *((uint16 *)pEmitLoc) = (uint16)imm;
      pEmitLoc += sizeof(uint16);
    }
  }
  else
  {
    if(regSrc == x86Reg_eax)
    {
      *pEmitLoc++ = 0xA9;
      *((uint32 *)pEmitLoc) = (uint32)imm;
      pEmitLoc += sizeof(uint32);
    }
    else
    {
      *pEmitLoc++ = 0xF7;
      X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)0,regSrc & 0x07);
      *((uint32 *)pEmitLoc) = imm;
      pEmitLoc += sizeof(uint32);
    }
  }
}

void NativeCodeCache::X86Emit_TESTIM(uint32 imm, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(ptrType == x86MemPtr_byte)
  {
    *pEmitLoc++ = 0xF6;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)0,base,index,scale,disp);
    *pEmitLoc++ = (uint8)imm;
  }
  else if(ptrType == x86MemPtr_word)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xF7;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)0,base,index,scale,disp);
    *((uint16 *)pEmitLoc) = (uint16)imm;
    pEmitLoc += sizeof(uint16);
  }
  else
  {
    *pEmitLoc++ = 0xF7;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)0,base,index,scale,disp);
    *((uint32 *)pEmitLoc) = imm;
    pEmitLoc += sizeof(uint32);
  }

}


void NativeCodeCache::X86Emit_NEGM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(ptrType == x86MemPtr_byte)
  {
    *pEmitLoc++ = 0xF6;
  }
  else if(ptrType == x86MemPtr_word)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xF7;
  }
  else
  {
    *pEmitLoc++ = 0xF7;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)3,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_CLC()
{
  *pEmitLoc++ = 0xF8;
}

void NativeCodeCache::X86Emit_STC()
{
  *pEmitLoc++ = 0xF9;
}

void NativeCodeCache::X86Emit_CLI()
{
  *pEmitLoc++ = 0xFA;
}

void NativeCodeCache::X86Emit_STI()
{
  *pEmitLoc++ = 0xFB;
}

void NativeCodeCache::X86Emit_CLD()
{
  *pEmitLoc++ = 0xFC;
}

void NativeCodeCache::X86Emit_STD()
{
  *pEmitLoc++ = 0xFD;
}

void NativeCodeCache::X86Emit_INCM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(ptrType == x86MemPtr_byte)
  {
    *pEmitLoc++ = 0xFE;
  }
  else if(ptrType == x86MemPtr_word)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xFF;
  }
  else
  {
    *pEmitLoc++ = 0xFF;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)0,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_DECM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(ptrType == x86MemPtr_byte)
  {
    *pEmitLoc++ = 0xFE;
  }
  else if(ptrType == x86MemPtr_word)
  {
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xFF;
  }
  else
  {
    *pEmitLoc++ = 0xFF;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)1,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_CALLNM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0xFF;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_CALLR(x86Reg regSrc)
{
  if(regSrc < x86Reg_eax)
  {
    //CALL r16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xFF;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(regSrc & 0x07));
  }
  else
  {
    //CALL r32
    *pEmitLoc++ = 0xFF;
  }
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CALLFM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0xFF;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)3,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_JMPR(x86Reg regSrc)
{
  if(regSrc < x86Reg_eax)
  {
    //JMP r16
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0xFF;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)4,(regSrc & 0x07));
  }
  else
  {
    //JMP r32
    *pEmitLoc++ = 0xFF;
    X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)4,(regSrc & 0x07));
  }
}

void NativeCodeCache::X86Emit_JMPNM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0xFF;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)4,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_JMPFM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0xFF;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)5,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_PUSHM(x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(ptrType == x86MemPtr_word)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0xFF;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)6,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_INVD(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x08;
}

void NativeCodeCache::X86Emit_WBINVD(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x09;
}

void NativeCodeCache::X86Emit_UD1(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x0B;
}

void NativeCodeCache::X86Emit_BSWAP(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xC8 + (reg & 0x07);
}

void NativeCodeCache::X86Emit_SETOR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x90;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETOM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x90;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETNOR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x91;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETNOM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x91;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETBR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x92;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETBM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x92;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETNBR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x93;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETNBM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x93;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETZR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x94;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETZM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x94;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETNZR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x95;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETNZM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x95;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETBER(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x96;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETBEM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x96;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETNBER(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x97;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETNBEM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x97;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETSR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x98;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETSM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x98;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETNSR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x99;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETNSM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x99;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETPR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9A;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETPM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9A;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETNPR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9B;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETNPM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9B;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETLR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9C;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETLM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9C;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETNLR(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9D;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETNLM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9D;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETLER(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9E;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETLEM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9E;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_SETNLER(x86Reg reg)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9F;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)2,(reg & 0x07));
}

void NativeCodeCache::X86Emit_SETNLEM(uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x9F;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)2,base,index,scale,disp);
}

void NativeCodeCache::X86Emit_CMOVORR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x40;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVOMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x40;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVNORR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x41;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVNOMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x41;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVBRR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x42;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVBMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x42;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVNBRR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x43;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVNBMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x43;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVZRR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x44;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVZMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x44;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVNZRR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x45;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVNZMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x45;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVBERR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x46;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVBEMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x46;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVNBERR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x47;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVNBEMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x47;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

//

void NativeCodeCache::X86Emit_CMOVSRR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x48;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVSMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x48;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVNSRR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x49;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVNSMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x49;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVPRR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4A;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVPMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4A;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVNPRR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4B;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVNPMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4B;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVLRR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4C;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVLMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4C;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVNLRR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4D;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVNLMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4D;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVLERR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4E;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVLEMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4E;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_CMOVNLERR(x86Reg regDest, uint32 regSrc)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4F;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_CMOVNLEMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    //r16, r16
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0x4F;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_MOVQRR(x86Reg regDest, x86Reg regSrc)
{
  if(regDest < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0x6F;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0xF3;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0x7E;
  }
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_MOVQMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0x6F;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0xF3;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0x7E;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index, scale, disp);
}

void NativeCodeCache::X86Emit_MOVQRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regSrc < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0x7F;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xD6;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index, scale, disp);
}

void NativeCodeCache::X86Emit_SHLDIRR(x86Reg regDest, x86Reg regSrc, uint8 shiftCount)
{
  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xA4;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
  *pEmitLoc++ = shiftCount;

}

void NativeCodeCache::X86Emit_SHLDRRR(x86Reg regDest, x86Reg regSrc)
{
  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xA5;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
}

void NativeCodeCache::X86Emit_SHLDIMR(x86Reg regDest, x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xA4;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),(base & 0x07),index, scale, disp);
  *pEmitLoc++ = shiftCount;

}

void NativeCodeCache::X86Emit_SHLDRMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xA5;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),(base & 0x07),index, scale, disp);
}

void NativeCodeCache::X86Emit_SHRDIRR(x86Reg regDest, x86Reg regSrc, uint8 shiftCount)
{
  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xAC;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
  *pEmitLoc++ = shiftCount;

}

void NativeCodeCache::X86Emit_SHRDRRR(x86Reg regDest, x86Reg regSrc)
{
  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xAD;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regSrc & 0x07),(regDest & 0x07));
}

void NativeCodeCache::X86Emit_SHRDIMR(x86Reg regDest, x86MemPtr ptrType, uint8 shiftCount, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xAC;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),(base & 0x07),index, scale, disp);
  *pEmitLoc++ = shiftCount;

}

void NativeCodeCache::X86Emit_SHRDRMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }

  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = 0xAD;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),(base & 0x07),index, scale, disp);
}

void NativeCodeCache::X86Emit_MOVZXRR(x86Reg regDest, uint32 regSrc)
{
  uint8 opcode = 0xB6;

  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }
  else if(regSrc >= x86Reg_ax)
  {
    opcode = 0xB7;
  }
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = opcode;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_MOVZXMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  uint8 opcode = 0xB6;

  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }
  else if(ptrType == x86MemPtr_word)
  {
    opcode = 0xB7;
  }
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = opcode;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base, index, scale, disp);
}

void NativeCodeCache::X86Emit_MOVSXRR(x86Reg regDest, uint32 regSrc)
{
  uint8 opcode = 0xBE;

  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }
  else if(regSrc >= x86Reg_ax)
  {
    opcode = 0xBF;
  }
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = opcode;
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_MOVSXMR(x86Reg regDest, x86MemPtr ptrType, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  uint8 opcode = 0xBE;

  if(regDest < x86Reg_eax)
  {
    *pEmitLoc++ = 0x66;
  }
  else if(ptrType == x86MemPtr_dword)
  {
    opcode = 0xBF;
  }
  *pEmitLoc++ = 0x0F;
  *pEmitLoc++ = opcode;
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base ,index, scale, disp);
}

void NativeCodeCache::X86Emit_PANDRR(x86Reg regDest, x86Reg regSrc)
{
  if(regDest < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDB;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDB;
  }
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_PANDMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDB;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDB;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index, scale, disp);
}

void NativeCodeCache::X86Emit_PANDRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regSrc < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDB;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDB;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index, scale, disp);
}

void NativeCodeCache::X86Emit_PANDBRR(x86Reg regDest, x86Reg regSrc)
{
  if(regDest < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDF;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDF;
  }
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_PANDNMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDF;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDF;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07),base,index, scale, disp);
}

void NativeCodeCache::X86Emit_PANDNRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regSrc < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDF;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xDF;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07),base,index, scale, disp);
}

void NativeCodeCache::X86Emit_PSUBDRR(x86Reg regDest, x86Reg regSrc)
{
  if(regDest < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFA;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFA;
  }
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_PSUBDMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFA;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFA;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_PSUBDRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regSrc < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFA;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFA;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_PADDRR(x86Reg regDest, x86Reg regSrc)
{
  if(regDest < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFE;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFE;
  }
  X86Emit_ModRegRM(x86ModType_reg,(x86ModReg)(regDest & 0x07),(regSrc & 0x07));
}

void NativeCodeCache::X86Emit_PADDMR(x86Reg regDest, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regDest < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFE;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFE;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regDest & 0x07), base, index, scale, disp);
}

void NativeCodeCache::X86Emit_PADDRM(x86Reg regSrc, uint32 base, x86IndexReg index, x86ScaleVal scale, int32 disp)
{
  if(regSrc < x86Reg_xmm0)
  {
    //MMX
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFE;
  }
  else
  {
    //SSE2
    *pEmitLoc++ = 0x66;
    *pEmitLoc++ = 0x0F;
    *pEmitLoc++ = 0xFE;
  }
  X86Emit_ModRegRM(x86ModType_mem,(x86ModReg)(regSrc & 0x07), base, index, scale, disp);
}
