#ifndef NATIVE_CODE_CACHE_H
#define NATIVE_CODE_CACHE_H

#include "basetypes.h"
#include "EmitMisc.h"
#include "PageMap.h"
#include "PatchManager.h"
#include "X86EmitTypes.h"

#define DEFAULT_CODE_BUFFER_BYTES (8UL*1024UL*1024UL)
//#define DEFAULT_NUM_TLB_ENTRIES (16384UL)

class NativeCodeCache
{
public:
  NativeCodeCache();
  NativeCodeCache(uint32 _numBytes/*, uint32 _desiredTLBEntries*/);
  void Init();
  ~NativeCodeCache();

  void Flush();
  void FlushRegion(const uint32 start, const uint32 end)
  {
    pageMap.InvalidateRegion(start, end);
  }
  bool ReleaseBuffer(NativeCodeCacheEntryPoint entryPoint, uint32 virtualAddress, uint32 nextVirtualAddress, uint32 newUsedBytes, uint32 packetCount, uint32 instructionCount, SuperBlockCompileType compileType, uint32 nextDelayCount, uint32 alignment);

  uint8 *GetBuffer() const
  {
    return ptrNativeCodeBuffer;
  }

  uint8 *GetEmitPointer() const
  {
    return pEmitLoc;
  }

  void AlignEmitPointer(const uint8 boundary)
  {
    if(boundary)
    {
      const size_t poweroftwominusone = (1UL << boundary) - 1UL;
      pEmitLoc = (uint8 *)(((size_t)(pEmitLoc + poweroftwominusone)) & ~(poweroftwominusone));
    }
  }

  uint32 GetAvailableCodeBufferSize() const
  {
    return numBytes - (uint32)(pEmitLoc - ptrNativeCodeBuffer);
  }  

  uint32 GetUsedCodeBufferSize() const
  {
    return (uint32)(pEmitLoc - ptrNativeCodeBuffer);
  }

#if 0
  uint8 *LockBuffer(uint32 * const pByteCount, const uint32 alignment)
  {
    if(pByteCount)
      *pByteCount = GetAvailableCodeBufferSize();

    return pEmitLoc;
  }
#endif

  uint32 GetTotalCodeBufferSize() const
  {
    return numBytes;
  }

  uint32 GetWarningThreshold() const
  {
    return warningThreshold;
  }

  bool IsBeyondThreshold() const
  {
    return GetUsedCodeBufferSize() > warningThreshold;
  }

  void SetEmitVars(MPE* const mpe)
  {
    emitVars.mpe = mpe;
  }

  void SetLabelPointer(const uint32 labelIndex)
  {
    patchMgr.SetLabelPointer(labelIndex,GetEmitPointer());
  }

  void X86Emit_ModRegRM(const x86ModType, const x86ModReg modReg, const uint32 baseReg, const x86IndexReg = x86IndexReg::x86IndexReg_none, const x86ScaleVal = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_Group1RR(const x86Reg regDest, const x86Reg regSrc, const uint8 groupIndex);
  void X86Emit_Group1RM(const x86Reg regSrc, const uint32 base, const x86IndexReg index, const x86ScaleVal scale, const int32 disp, const uint8 groupIndex);
  void X86Emit_Group1MR(const x86Reg regDest, const uint32 base, const x86IndexReg index, const x86ScaleVal scale, const int32 disp, const uint8 groupIndex);
  void X86Emit_Group1IR(const int32 imm, const x86Reg regDest, const uint8 groupIndex);
  void X86Emit_Group1IM(const int32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index, const x86ScaleVal scale, const int32 disp, const uint8 groupIndex);
  void X86Emit_Group2IR(const x86Reg regDest, const uint8 shiftCount, const uint8 groupIndex);
  void X86Emit_Group2IM(const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index, const x86ScaleVal scale, const int32 disp, const uint8 groupIndex);
  void X86Emit_Group2RR(const x86Reg regDest, const uint8 groupIndex);
  void X86Emit_Group2RM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index, const x86ScaleVal scale, const int32 disp, const uint8 groupIndex);
  
  void X86Emit_ADDRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_ADDRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_ADDMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_ADDMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_ADDMR(regDest, (uint32)base, index, scale, disp);
  }
  void X86Emit_ADDIR(const int32 imm, const x86Reg regDest);
  void X86Emit_ADDIM(const int32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PUSHES();
  void X86Emit_POPES();
  void X86Emit_ORRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_ORRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_ORRM(const x86Reg regSrc, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_ORRM(regSrc, (uint32)base, index, scale, disp);
  }
  void X86Emit_ORMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_ORMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_ORMR(regDest, (uint32)base, index, scale, disp);
  }
  void X86Emit_ORIR(const int32 imm, const x86Reg regDest);
  void X86Emit_ORIM(const int32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_ORIM(const int32 imm, const x86MemPtr ptrType, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_ORIM(imm, ptrType, (uint32)base, index, scale, disp);
  }
  void X86Emit_PUSHCS();
  void X86Emit_ADCRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_ADCRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_ADCMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_ADCMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_ADCMR(regDest, (uint32)base, index, scale, disp);
  }
  void X86Emit_ADCIR(const int32 imm, const x86Reg regDest);
  void X86Emit_ADCIM(const int32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PUSHSS();
  void X86Emit_POPSS();
  void X86Emit_SBBRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_SBBRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SBBMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SBBMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_SBBMR(regDest, (uint32)base, index, scale, disp);
  }
  void X86Emit_SBBIR(const int32 imm, const x86Reg regDest);
  void X86Emit_SBBIM(const int32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PUSHDS();
  void X86Emit_POPDS();
  void X86Emit_ANDRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_ANDRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_ANDMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_ANDMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_ANDMR(regDest, (uint32)base, index, scale, disp);
  }
  void X86Emit_ANDIR(const int32 imm, const x86Reg regDest);
  void X86Emit_ANDIM(const int32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_ANDIM(const int32 imm, const x86MemPtr ptrType, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_ANDIM(imm, ptrType, (uint32)base, index, scale, disp);
  }
  void X86Emit_ES();
  void X86Emit_DAA();
  void X86Emit_SUBRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_SUBRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SUBMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SUBMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_SUBMR(regDest, (uint32)base, index, scale, disp);
  }
  void X86Emit_SUBIR(const int32 imm, const x86Reg regDest);
  void X86Emit_SUBIM(const int32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CS();
  void X86Emit_DAS();
  void X86Emit_XORRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_XORRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_XORMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_XORMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_XORMR(regDest, (uint32)base, index, scale, disp);
  }
  void X86Emit_XORIR(const int32 imm, const x86Reg regDest);
  void X86Emit_XORIM(const int32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SS();
  void X86Emit_AAA();
  void X86Emit_CMPRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMPRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMPMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMPIR(const int32 imm, const x86Reg regDest);
  void X86Emit_CMPIM(const int32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_DS();
  void X86Emit_AAS();
  void X86Emit_INCR(const x86Reg reg);
  void X86Emit_DECR(const x86Reg reg);
  void X86Emit_PUSHR(const x86Reg reg);
  void X86Emit_POPR(const x86Reg reg);
  void X86Emit_PUSHAW();
  void X86Emit_PUSHAD();
  #define X86Emit_PUSHA X86Emit_PUSHAD
  void X86Emit_POPAW();
  void X86Emit_POPAD();
  #define X86Emit_POPA X86Emit_POPAD
  void X86Emit_FS();
  void X86Emit_GS();
  void X86Emit_OPSIZE();
  void X86Emit_ADSIZE();
  void X86Emit_PUSHID(const int32 imm);
  void X86Emit_PUSHIW(const int16 imm);
  void X86Emit_IMULIRR(const x86Reg regDest, const int32 imm, const x86Reg regSrc);
  void X86Emit_IMULIMR(const x86Reg regDest, const int32 imm, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_IMULRR(const x86Reg regSrc);
  void X86Emit_IMULMR(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_IMULMR(const x86MemPtr ptrType, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_IMULMR(ptrType, (uint32)base, index, scale, disp);
  }
  void X86Emit_IMULRRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_IMULMRR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PUSHIB(const int8 imm);
  void X86Emit_INSB();
  void X86Emit_INSW();
  void X86Emit_INSD();
  void X86Emit_OUTSB();
  void X86Emit_OUTSW();
  void X86Emit_OUTSD();
  void X86Emit_JCC(uint8 *pTarget, const int8 conditionCode);
  void X86Emit_JCC_Label(const int8 conditionCode, const uint32 labelIndex);
  void X86Emit_JO(uint8 *pTarget);
  void X86Emit_JNO(uint8 *pTarget);
  void X86Emit_JB(uint8 *pTarget);
  void X86Emit_JNB(uint8 *pTarget);
  void X86Emit_JZ(uint8 *pTarget);
  void X86Emit_JNZ(uint8 *pTarget);
  void X86Emit_JBE(uint8 *pTarget);
  void X86Emit_JNBE(uint8 *pTarget);
  void X86Emit_JS(uint8 *pTarget);
  void X86Emit_JNS(uint8 *pTarget);
  void X86Emit_JP(uint8 *pTarget);
  void X86Emit_JNP(uint8 *pTarget);
  void X86Emit_JL(uint8 *pTarget);
  void X86Emit_JNL(uint8 *pTarget);
  void X86Emit_JLE(uint8 *pTarget);
  void X86Emit_JNLE(uint8 *pTarget);
  void X86Emit_TESTRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_TESTRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_TESTIR(const uint32 imm, const x86Reg regSrc);
  void X86Emit_TESTIM(const uint32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_XCHGRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_LEA(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_POPM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_NOP();
  void X86Emit_XCHGRR(const x86Reg reg1, const x86Reg reg2);
  void X86Emit_CBW();
  void X86Emit_CWDE();
  void X86Emit_CWD();
  void X86Emit_CDQ();
  void X86Emit_CALLI(uint32 offset, uint16 seg);
  void X86Emit_JMPI(uint8 *target, uint16 seg);
  void X86Emit_JMPI_Label(const uint32 labelIndex);
  void X86Emit_WAIT();
  void X86Emit_PUSHFW();
  void X86Emit_PUSHFD();
  #define X86Emit_PUSHF X86Emit_PUSHFD
  void X86Emit_POPFW();
  void X86Emit_POPFD();
  #define X86Emit_POPF X86Emit_POPFD
  void X86Emit_SAHF();
  void X86Emit_LAHF();
  void X86Emit_MOVMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_MOVMR(regDest, (uint32)base, index, scale, disp);
  }
  void X86Emit_MOVRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVRM(const x86Reg regSrc, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_MOVRM(regSrc, (uint32)base, index, scale, disp);
  }
  void X86Emit_MOVIR(const int32 imm, const x86Reg regDest);
  void X86Emit_MOVSB();
  void X86Emit_MOVSW();
  void X86Emit_MOVSD();
  void X86Emit_CMPSB();
  void X86Emit_CMPSW();
  void X86Emit_CMPSD();
  void X86Emit_STOSB();
  void X86Emit_STOSW();
  void X86Emit_STOSD();
  void X86Emit_LODSB();
  void X86Emit_LODSW();
  void X86Emit_LODSD();
  void X86Emit_SCASB();
  void X86Emit_SCASW();
  void X86Emit_SCASD();
  void X86Emit_ROLIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_RORIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_RCLIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_RCRIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_SHLIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_SHLDIRR(const x86Reg regDest, const x86Reg regSrc, const uint8 shiftCount);
  void X86Emit_SHLDIMR(const x86Reg regDest, const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SHLDRRR(const x86Reg regDest, const x86Reg regSrc);
  //void X86Emit_SHLDIRM(const x86Reg regSrc, const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  //void X86Emit_SHLDRRM(const x86Reg regSrc, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SHRDIRR(const x86Reg regDest, const x86Reg regSrc, const uint8 shiftCount);
  void X86Emit_SHRDIMR(const x86Reg regDest, const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index, const x86ScaleVal scale, const int32 disp = 0);
  void X86Emit_SHRDRRR(const x86Reg regDest, const x86Reg regSrc);
  //void X86Emit_SHRDIRM(const x86Reg regSrc, const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  //void X86Emit_SHRDRRM(const x86Reg regSrc, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SHLDRMR(const x86Reg regDest, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SHRDRMR(const x86Reg regDest, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index, const x86ScaleVal scale, const int32 disp);
  void X86Emit_SHRIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_SALIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_SARIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_ROLIM(const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_RORIM(const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_RCLIM(const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_RCRIM(const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SHLIM(const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SHRIM(const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SALIM(const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SARIM(const x86MemPtr ptrType, const uint8 shiftCount, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);

  void X86Emit_ROLRM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_RORRM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_RCLRM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_RCRRM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SHLRM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SHLRM(const x86MemPtr ptrType, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_SHLRM(ptrType, (uint32)base, index, scale, disp);
  }
  void X86Emit_SHRRM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SALRM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SARRM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);

  void X86Emit_RETN(uint16 iw = 0);
  #define X86Emit_RET X86Emit_RETN
  void X86Emit_MOVIM(const int32 imm, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVIM(const int32 imm, const x86MemPtr ptrType, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_MOVIM(imm, ptrType, (uint32)base, index, scale, disp);
  }
  void X86Emit_LEAVE();
  void X86Emit_RETF(uint16 iw = 0);
  void X86Emit_INT3();
  void X86Emit_INT(int8 vector);
  void X86Emit_INTO();
  void X86Emit_IRET();
  void X86Emit_ROLRR(const x86Reg regDest);
  void X86Emit_RORRR(const x86Reg regDest);
  void X86Emit_RCLRR(const x86Reg regDest);
  void X86Emit_RCRRR(const x86Reg regDest);
  void X86Emit_SHLRR(const x86Reg regDest);
  void X86Emit_SHRRR(const x86Reg regDest);
  void X86Emit_SALRR(const x86Reg regDest);
  void X86Emit_SARRR(const x86Reg regDest);
  void X86Emit_AAM(uint8 divisor);
  void X86Emit_AAD(uint8 divisor);
  void X86Emit_XLAT();
  void X86Emit_ESC0();
  void X86Emit_ESC1();
  void X86Emit_ESC2();
  void X86Emit_ESC3();
  void X86Emit_ESC4();
  void X86Emit_ESC5();
  void X86Emit_ESC6();
  void X86Emit_ESC7();
  void X86Emit_LOOPNE(uint8 *pTarget);
  void X86Emit_LOOPNE_Label(const uint32 labelIndex);
  #define X86Emit_LOOPNZ X86Emit_LOOPNE
  #define X86Emit_LOOPNZ_Label X86Emit_LOOPNE_Label
  void X86Emit_LOOPE(uint8 *pTarget);
  void X86Emit_LOOPE_Label(const uint32 labelIndex);
  #define X86Emit_LOOPZ X86Emit_LOOPE
  #define X86Emit_LOOPZ_Label X86Emit_LOOPE_Label
  void X86Emit_LOOP(uint8 *pTarget);
  void X86Emit_LOOP_Label(const uint32 labelIndex);
  void X86Emit_JCXZ(uint8 *pTarget);
  void X86Emit_JCXZ_Label(const uint32 labelIndex);
  void X86Emit_JECXZ(uint8 *pTarget);
  void X86Emit_JECXZ_Label(const uint32 labelIndex);
  void X86Emit_INI(const x86Reg regDest, uint8 port);
  void X86Emit_OUTI(const x86Reg regDest, uint8 data);
  void X86Emit_INR(const x86Reg regDest);
  void X86Emit_OUTR(const x86Reg regDest);
  void X86Emit_LOCK();
  void X86Emit_INT1();
  void X86Emit_REPNE();
  void X86Emit_REPE();
  #define X86Emit_REP X86Emit_REPE
  void X86Emit_HLT();
  void X86Emit_CMC();
  void X86Emit_NOTR(const x86Reg regDest);
  void X86Emit_NOTM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_NEGR(const x86Reg regDest);
  void X86Emit_NEGM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CLC();
  void X86Emit_STC();
  void X86Emit_CLI();
  void X86Emit_STI();
  void X86Emit_CLD();
  void X86Emit_STD();
  void X86Emit_INCM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_DECM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CALLNM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  #define X86Emit_CALLM X86Emit_CALLNM
  void X86Emit_CALLR(const x86Reg regSrc);
  void X86Emit_CALLFM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_JMPR(const x86Reg regSrc);
  void X86Emit_JMPNM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  #define X86Emit_JMPM X86Emit_JMPNM
  void X86Emit_JMPFM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PUSHM(const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_INVD(const x86Reg reg);
  void X86Emit_WBINVD(const x86Reg reg);
  void X86Emit_UD1(const x86Reg reg);
  void X86Emit_BSWAP(const x86Reg reg);
  void X86Emit_SETOR(const x86Reg reg);
  void X86Emit_SETOM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETNOR(const x86Reg reg);
  void X86Emit_SETNOM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETBR(const x86Reg reg);
  void X86Emit_SETBM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETNBR(const x86Reg reg);
  void X86Emit_SETNBM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETZR(const x86Reg reg);
  void X86Emit_SETZM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETNZR(const x86Reg reg);
  void X86Emit_SETNZM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETBER(const x86Reg reg);
  void X86Emit_SETBEM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETNBER(const x86Reg reg);
  void X86Emit_SETNBEM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETSR(const x86Reg reg);
  void X86Emit_SETSM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETNSR(const x86Reg reg);
  void X86Emit_SETNSM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETPR(const x86Reg reg);
  void X86Emit_SETPM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETNPR(const x86Reg reg);
  void X86Emit_SETNPM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETLR(const x86Reg reg);
  void X86Emit_SETLM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETNLR(const x86Reg reg);
  void X86Emit_SETNLM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETLER(const x86Reg reg);
  void X86Emit_SETLEM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_SETNLER(const x86Reg reg);
  void X86Emit_SETNLEM(const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVORR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVOMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVNORR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVNOMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVBRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVBMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVNBRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVNBMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVZRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVZMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVNZRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVNZMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVBERR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVBEMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVNBERR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVNBEMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVSRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVSMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVNSRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVNSMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVPRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVPMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVNPRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVNPMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVLRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVLMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVNLRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVNLMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVLERR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVLEMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_CMOVNLERR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_CMOVNLEMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);

  void X86Emit_MOVSXRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_MOVZXRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_MOVSXMR(const x86Reg regDest, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVZXMR(const x86Reg regDest, const x86MemPtr ptrType, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);

  void X86Emit_PSRADRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_PSRADIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_PSRLDRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_PSRLDIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_PSLDRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_PSLDIR(const x86Reg regDest, const uint8 shiftCount);
  void X86Emit_PMULLDRR(const x86Reg regDest, const x86Reg regSrc);

  void X86Emit_PHADDDRR(const x86Reg regDest, const x86Reg regSrc);

  void X86Emit_PSHUFBRR(const x86Reg regDest, const x86Reg regSrc);

  void X86Emit_DPPSRR(const x86Reg regDest, const x86Reg regSrc, const uint8 mask);

  void X86Emit_MOVDRR(const x86Reg regDest, const x86Reg regSrc); // Move doubleword from r/m32 to xmm
  void X86Emit_MOVDRR2(const x86Reg regDest, const x86Reg regSrc); // Move doubleword from xmm to r/m32
  void X86Emit_MOVSSMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVSSMR(const x86Reg regSrc, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_MOVSSMR(regSrc, (uint32)base, index, scale, disp);
  }

  void X86Emit_UNPCKLRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_MOVLHRR(const x86Reg regDest, const x86Reg regSrc);

  void X86Emit_SHUFIR(const x86Reg regDest, const x86Reg regSrc, const uint8 shiftCount);

  void X86Emit_MOVQRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_MOVQRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVQRM(const x86Reg regSrc, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_MOVQRM(regSrc, (uint32)base, index, scale, disp);
  }
  void X86Emit_MOVDQURM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVDQURM(const x86Reg regSrc, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_MOVDQURM(regSrc, (uint32)base, index, scale, disp);
  }
  void X86Emit_MOVDQARM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVDQARM(const x86Reg regSrc, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_MOVDQARM(regSrc, (uint32)base, index, scale, disp);
  }
  void X86Emit_MOVQMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVQMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_MOVQMR(regDest, (uint32)base, index, scale, disp);
  }
  void X86Emit_MOVDQUMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVDQUMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_MOVDQUMR(regDest, (uint32)base, index, scale, disp);
  }
  void X86Emit_MOVDQAMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_MOVDQAMR(const x86Reg regDest, const x86BaseReg base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0)
  {
    X86Emit_MOVDQAMR(regDest, (uint32)base, index, scale, disp);
  }

  void X86Emit_PANDRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_PANDMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PANDRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PANDBRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_PANDNMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PANDNRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PSUBDRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_PSUBDMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PSUBDRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PADDRR(const x86Reg regDest, const x86Reg regSrc);
  void X86Emit_PADDMR(const x86Reg regDest, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);
  void X86Emit_PADDRM(const x86Reg regSrc, const uint32 base, const x86IndexReg index = x86IndexReg::x86IndexReg_none, const x86ScaleVal scale = x86ScaleVal::x86Scale_1, const int32 disp = 0);

  void X86Emit_EMMS();
  void X86Emit_FEMMS();

  PatchManager patchMgr;
  PageMap pageMap;
  EmitterVariables emitVars;

private:
  uint8 *pEmitLoc;
  uint8 *ptrNativeCodeBuffer;
  uint32 warningThreshold;
  uint32 numBytes;
  //const uint32 numTLBEntries;
};

#endif
