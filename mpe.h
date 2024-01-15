#ifndef MPE_H
#define MPE_H

#define COMPILE_THRESHOLD 50UL

//#define COMPILE_TYPE SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_IL_BLOCK
#define COMPILE_TYPE SuperBlockCompileType::SUPERBLOCKCOMPILETYPE_NATIVE_CODE_BLOCK
#define COMPILE_SINGLE_PACKET false

#define ALLOW_NATIVE_CODE_EMIT true

//#define LOG_COMM
//#define LOG_PROGRAM_FLOW
//#define LOG_ADDRESS_ONLY
//#define LOG_BIOS_CALLS

#include "basetypes.h"
#include "InstructionCache.h"
#include "nativecodecache.h"
#include "NuonMemoryMap.h"
#include "OverlayManager.h"
#include "Syscall.h"
#include "SuperBlock.h"

struct EmitterVariables;

#define INDEX_REG 35

#define REG_X (0)
#define REG_Y (1)
#define REG_U (2)
#define REG_V (3)

#define COUNTER_REG 33
#define RZ_REG 39
#define ACS_REG 46
#define SVS_REG 47
#define XYC_REG 42
#define UVC_REG 43
#define XYR_REG 44
#define UVR_REG 45

#define DECOMPRESS_OPTIONS_INHIBIT_ECU (0x00000004UL)
#define DECOMPRESS_OPTIONS_SCHEDULE_MEM_FIRST (0x00000002UL)
#define DECOMPRESS_OPTIONS_SCHEDULE_ECU_LAST (0x00000001UL)
#define MPECTRL_MPEGOCLR (1UL << 0)
#define MPECTRL_MPEGO (1UL << 1)
#define MPECTRL_MPESINGLESTEPCLR (1UL << 2)
#define MPECTRL_MPESINGLESTEP (1UL << 3)
#define MPECTRL_MPEDARDBRKENCLR (1UL << 4)
#define MPECTRL_MPEDARDBRKEN (1UL << 5)
#define MPECTRL_MPEDAWRBRKENCLR (1UL << 6)
#define MPECTRL_MPEDAWRBRKEN (1UL << 7)
#define MPECTRL_MPEIS2XCLR (1UL << 8)
#define MPECTRL_MPEDIS2X (1UL << 9)
#define MPECTRL_MPEINTTOHOSTCLR (1UL << 10)
#define MPECTRL_MPEINTTOHOST (1UL << 11)
#define MPECTRL_MPERESET (1UL << 13)
#define MPECTRL_MPEWASRESETCLR (1UL << 14)
#define MPECTRL_MPEWASRESET (1UL << 15)
#define MPECTRL_MPECYCLETYPEWREN (1UL << 23)
#define MPECTRL_MPECYCLETYPE (0xFUL << 24)

#define INT_EXCEPTION (1UL)
#define INT_SOFTWARE (1UL << 1)
#define INT_COMMRECV (1UL << 4)
#define INT_COMMXMIT (1UL << 5)
#define INT_MDMADONE (1UL << 6)
#define INT_MDMAREADY (1UL << 7)
#define INT_ODMADONE (1UL << 8)
#define INT_ODMAREADY (1UL << 9)
#define INT_VDMADONE (1UL << 12)
#define INT_VDMAREADY (1UL << 13)
#define INT_SYSTIMER2 (1UL << 16)
#define INT_MDMAOTF (1UL << 17)
#define INT_MDMADUMP (1UL << 18)
#define INT_MDMAFINISH (1UL << 19)
#define INT_IICPERIPH (1UL << 20)
#define INT_BDUERROR (1UL << 21)
#define INT_BDUMBDONE (1UL << 22)
#define INT_MCUDCTDONE (1UL << 23)
#define INT_MCUMBDONE (1UL << 24)
#define INT_DEBUG (1UL << 25)
#define INT_HOST (1UL << 26)
#define INT_AUDIO (1UL << 27)
#define INT_GPIO (1UL << 28)
#define INT_SYSTIMER0 (1UL << 29)
#define INT_SYSTIMER1 (1UL << 30)
#define INT_VIDTIMER (1UL << 31)

#define CC_NUM_FLAGS (11)

#define SLOT_COPROCESSOR1 (10)
#define SLOT_COPROCESSOR0 (9)
#define SLOT_MODMI (8)
#define SLOT_MODGE (7)
#define SLOT_COUNTER1_ZERO (6)
#define SLOT_COUNTER0_ZERO (5)
#define SLOT_MUL_OVERFLOW (4)
#define SLOT_ALU_NEGATIVE (3)
#define SLOT_ALU_OVERFLOW (2)
#define SLOT_ALU_CARRY (1)
#define SLOT_ALU_ZERO (0)

#define CC_COPROCESSOR1 (0x400UL)
#define CC_COPROCESSOR0 (0x200UL)
#define CC_MODMI (0x100UL)
#define CC_MODGE (0x80UL)
#define CC_COUNTER1_ZERO (0x40UL)
#define CC_COUNTER0_ZERO (0x20UL)
#define CC_MUL_OVERFLOW (0x10UL)
#define CC_ALU_NEGATIVE (0x08UL)
#define CC_ALU_OVERFLOW (0x04UL)
#define CC_ALU_CARRY (0x02UL)
#define CC_ALU_ZERO (0x01UL)
#define CC_ALLFLAGS (0x7FFUL)

/*
The following sequence describes the condition variable sequence
utilized by 32/64 bit ECU instructions and TestConditionCode.  The
decode handler for 16/48 bit ECU instructions will convert to
this sequence
*/

#define ECU_CONDITION_NE (0x00UL)
#define ECU_CONDITION_C0EQ (0x01UL)
#define ECU_CONDITION_C1EQ (0x02UL)
#define ECU_CONDITION_CC (0x03UL)
#define ECU_CONDITION_EQ (0x04UL)
#define ECU_CONDITION_CS (0x05UL)
#define ECU_CONDITION_VC (0x06UL)
#define ECU_CONDITION_VS (0x07UL)
#define ECU_CONDITION_LT (0x08UL)
#define ECU_CONDITION_MVC (0x09UL)
#define ECU_CONDITION_MVS (0x0AUL)
#define ECU_CONDITION_HI (0x0BUL)
#define ECU_CONDITION_LE (0x0CUL)
#define ECU_CONDITION_LS (0x0DUL)
#define ECU_CONDITION_PL (0x0EUL)
#define ECU_CONDITION_MI (0x0FUL)
#define ECU_CONDITION_GT (0x10UL)
#define ECU_CONDITION_T (0x11UL)
#define ECU_CONDITION_MODMI (0x12UL)
#define ECU_CONDITION_MODPL (0x13UL)
#define ECU_CONDITION_GE (0x14UL)
#define ECU_CONDITION_MODGE (0x15UL)
#define ECU_CONDITION_MODLT (0x16UL)
#define ECU_CONDITION_C0NE (0x18UL)
#define ECU_CONDITION_CF0LO (0x1BUL)
#define ECU_CONDITION_C1NE (0x1CUL)
#define ECU_CONDITION_CF0HI (0x1DUL)
#define ECU_CONDITION_CF1LO (0x1EUL)
#define ECU_CONDITION_CF1HI (0x1FUL)

/*

The following sequence is for conditions encoded
in the 16/48 bit ECU instructions.  The decode handler
will convert these values to the sequence used by 32/64
bit ECU instructions and TestConditionCode.

#define ECU_CONDITION_NE (0UL)
#define ECU_CONDITION_EQ (1UL)
#define ECU_CONDITION_LT (2UL)
#define ECU_CONDITION_LE (3UL)
#define ECU_CONDITION_GT (4UL)
#define ECU_CONDITION_GE (5UL)
#define ECU_CONDITION_C0NE (6UL)
#define ECU_CONDITION_C1NE (7UL)
#define ECU_CONDITION_C0EQ (8UL)
#define ECU_CONDITION_CS (9UL)
#define ECU_CONDITION_MVC (10UL)
#define ECU_CONDITION_LS (11UL)
#define ECU_CONDITION_T (12UL)
#define ECU_CONDITION_MODGE (13UL)
#define ECU_CONDITION_CF0HI (15UL)
#define ECU_CONDITION_C1EQ (16UL)
#define ECU_CONDITION_VC (17UL)
#define ECU_CONDITION_MVS (18UL)
#define ECU_CONDITION_PL (19UL)
#define ECU_CONDITION_MODMI (20UL)
#define ECU_CONDITION_MODLT (21UL)
#define ECU_CONDITION_CF1LO (23UL)
#define ECU_CONDITION_CC (24UL)
#define ECU_CONDITION_VS (25UL)
#define ECU_CONDITION_HI (26UL)
#define ECU_CONDITION_MI (27UL)
#define ECU_CONDITION_MODPL (28UL)
#define ECU_CONDITION_CF0LO (30UL)
#define ECU_CONDITION_CF1HI (31UL)
*/

#define PACKETSTRUCT_ECU (0)
#define PACKETSTRUCT_RCU (1)
#define PACKETSTRUCT_ALU (2)
#define PACKETSTRUCT_MUL (3)
#define PACKETSTRUCT_MEM (4)

#define BilinearInfo_XRev(data) ((data) & (1UL << 30))
#define BilinearInfo_YRev(data) ((data) & (1UL << 29))
#define BilinearInfo_XYChnorm(data) ((data) & (1UL << 28))
#define BilinearInfo_XYMipmap(data) (((data) >> 24) & 0x07UL)
#define BilinearInfo_XYType(data) (((data) >> 20) & 0x0FUL)
#define BilinearInfo_XTile(data) (((data) >> 16) & 0x0FUL)
#define BilinearInfo_YTile(data) (((data) >> 12) & 0x0FUL)
#define BilinearInfo_XYWidth(data) ((data) & 0x7FFUL)
#define BilinearInfo_PixelWidth(table,data) ((table)[((data) >> 20) & 0x0FUL])

#define REGINDEX_CC (0UL)
#define REGINDEX_RC0 (1UL)
#define REGINDEX_RC1 (2UL)
#define REGINDEX_RX (3UL)
#define REGINDEX_RY (4UL)
#define REGINDEX_RU (5UL)
#define REGINDEX_RV (6UL)
#define REGINDEX_RZ (7UL)
#define REGINDEX_RZI1 (8UL)
#define REGINDEX_RZI2 (9UL)
#define REGINDEX_XYCTL (10UL)
#define REGINDEX_UVCTL (11UL)
#define REGINDEX_XYRANGE (12UL)
#define REGINDEX_UVRANGE (13UL)
#define REGINDEX_ACSHIFT (14UL)
#define REGINDEX_SVSHIFT (15UL)

typedef void (* NuanceHandler)(MPE &, const uint32 pRegs[48], const Nuance &);
typedef void NuanceHandlerProto(MPE &, const uint32 pRegs[48], const Nuance &);
typedef uint32 (* NuancePrintHandler)(char *, size_t, const Nuance &, bool);
typedef uint32 NuancePrintHandlerProto(char *, size_t, const Nuance &, bool);
typedef void (* NativeEmitHandler)(EmitterVariables * const, const Nuance &);
typedef void NativeEmitHandlerProto(EmitterVariables * const, const Nuance &);
typedef void (* NativeCodeBlockFunction)();

extern const NuanceHandler nuanceHandlers[];

// This assumes emulation of a Aries 2 MPE (for now), e.g. due to how the overlay code caching works (see OverlayManager)
__declspec(align(16)) class MPE
{
public:
  union {
      struct {
          //don't change the order of these registers!
          uint32 regs[32];

          uint32 cc;
          uint32 rc0;
          uint32 rc1;

          uint32 rx;
          uint32 ry;
          uint32 ru;
          uint32 rv;
          uint32 rz;
          uint32 rzi1;
          uint32 rzi2;
          uint32 xyctl;
          uint32 uvctl;
          uint32 xyrange;
          uint32 uvrange;
          uint32 acshift;
          uint32 svshift;
      };
      struct {
          uint32 reg_union[32+3+13];
      };
  };

  union {
      struct {
          //don't change the order of the temp registers!
          uint32 tempScalarRegs[32];
        
          uint32 tempCC;
          uint32 tempRc0;
          uint32 tempRc1;
        
          uint32 tempRx;
          uint32 tempRy;
          uint32 tempRu;
          uint32 tempRv;
          uint32 tempRz;
          uint32 tempRzi1;
          uint32 tempRzi2;
          uint32 tempXyctl;
          uint32 tempUvctl;
          uint32 tempXyrange;
          uint32 tempUvrange;
          uint32 tempAcshift;
          uint32 tempSvshift;
      };
      struct {
          uint32 tempreg_union[32+3+13];
      };
  };

  uint32 mpectl;
  uint32 excepsrc;
  uint32 excepclr;
  uint32 excephalten;
  uint32 pcfetch;
  uint32 pcroute;
  uint32 pcexec;
  uint32 intvec1;
  uint32 intvec2;
  uint32 intsrc;
  uint32 intclr; // unused, always reads as zero
  uint32 intctl;
  uint32 inten1;
  uint32 inten1set;
  uint32 inten1clr;
  uint32 inten2sel;
  uint32 xybase;
  uint32 uvbase;
  uint32 linpixctl;
  uint32 clutbase;
  uint32 sp;
  uint32 dabreak;
  uint32 odmactl;
  uint32 odmacptr;
  uint32 mdmactl;
  uint32 mdmacptr;
  uint32 comminfo;
  uint32 commctl;
  uint32 commxmit[4];
  uint32 commrecv[4];
  uint32 configa; // constant
  uint32 configb; // constant
  uint32 dcachectl;
  uint32 icachectl;
  uint32 vdmactla;
  uint32 vdmactlb;
  uint32 vdmaptra;
  uint32 vdmaptrb;

  //emulator variables

  uint8  dtrom[MPE_LOCAL_MEMORY_SIZE];
  uint32 pcfetchnext;
  //uint32 prevPcexec;
  uint32 breakpointAddress;
  uint32 ecuSkipCounter;
  uint64 cycleCounter;

  //uint32 overlayIndex;
  uint32 overlayMask;
  uint32 interpretNextPacket;
  uint32 invalidateRegionStart;
  uint32 invalidateRegionEnd;
  uint32 interpreterInvalidateRegionStart;
  uint32 interpreterInvalidateRegionEnd;

  bool bInvalidateInterpreterCache;
  bool bInvalidateInstructionCaches;
  bool bInterpretedBranchTaken;
  bool nuances_use_tempreg_union; // steers which pointer to 48 MPE Regs is used, either reg_union or tempreg_union, which is then passed to the Nuances

  uint32 numInterpreterCacheFlushes;
  uint32 numNativeCodeCacheFlushes;
  uint32 numNonCompilablePackets;
  //uint32 mpeStartAddress;
  //uint32 mpeEndAddress;
  uint32 mpeIndex;

  // these are only used to communicate data into the _LoadPixelAbsolute,_LoadPixelZAbsolute,_StorePixelAbsolute,_StorePixelZAbsolute calls, but never back to anything else
  uint32 ba_reg_offset;
  uint32 ba_control;
  // this one is used to pass data from GetBilinearAddress into the single 4bit case in _LoadPixelAbsolute, but never back to anything else
  uint32 ba_mipped_xoffset; // note that only the lowest or the highest bit is used in _LoadPixelAbsolute later-on

  InstructionCache *instructionCache;
  SuperBlock superBlock;
  NativeCodeCache nativeCodeCache;
  OverlayManager overlayManager;

  uint8 *bankPtrTable[16];

  static constexpr uint32 numCacheEntries[] = { 4096,2048,2048,262144 };
  //static constexpr uint32 numTLBEntries[] = {4096,2048,2048,98304};
  static constexpr uint32 overlayLengths[] = { 8192,4096,4096,4096 }; // Aries 2 MPE program RAM (real HW size in bytes)
  //static constexpr uint32 overlayLengths[] = {20*1024,16*1024,16*1024,20*1024}; // Aries 3

  void InitMPELocalMemory();
  uint8 DecodeSingleInstruction(const uint8 * const iPtr, InstructionCacheEntry * const entry, uint32 * const immExt, bool &bTerminating);
  uint32 GetPacketDelta(const uint8 *iPtr, uint32 numLevels);
  void DecompressPacket(const uint8 *iBuffer, InstructionCacheEntry &pICacheEntry, const uint32 options = 0);
  bool ChooseInstructionPairOrdering(const InstructionCacheEntry &entry, const uint32 slot1, const uint32 slot2);
#if 0
  uint32 ScoreInstructionTriplet(const InstructionCacheEntry &srcEntry, const uint32 slot1, const uint32 slot2, const uint32 slot3);
#endif
  void GetInstructionTripletDependencies(uint32& comboScalarDep, uint32& comboMiscDep, const InstructionCacheEntry &srcEntry, const uint32 slot1, const uint32 slot2, const uint32 slot3);
  void ScheduleInstructionTriplet(InstructionCacheEntry &destEntry, const uint32 baseSlot, const InstructionCacheEntry &srcEntry, const uint32 slot1, const uint32 slot2, const uint32 slot3);
  void ScheduleInstructionQuartet(InstructionCacheEntry &destEntry, const uint32 baseSlot, const InstructionCacheEntry &srcEntry);
  bool FetchDecodeExecute();
  void ExecuteSingleStep();

  void ExecuteNuances(const InstructionCacheEntry& entry)
  {
    if(entry.packetInfo & PACKETINFO_BREAKPOINT)
    {
      excepsrc |= 0x04;
      if(excephalten & 0x04)
      {
        //clear mpego bit
        mpectl &= ~MPECTRL_MPEGO;
      }
      else
      {
        //set exception bit in interrupt source register
        intsrc |= 0x01;
      }
    }

    if(!(entry.packetInfo & PACKETINFO_NOP))
    {
      //!! move parallelization here?!
      if(entry.packetInfo & PACKETINFO_DEPENDENCY_PRESENT)
        memcpy(tempreg_union, reg_union, sizeof(uint32) * 48);
      else
        tempCC = cc;

      //static uint64 nuanceHandlersCount[224] = {};
      //static uint64 counter = 0;
      //static uint64 oldcounter = 0;

      for(uint32 i = 0; i < entry.nuanceCount; i++)
      {
        //nuanceHandlersCount[entry.handlers[i]]++;
        //counter++;

        (nuanceHandlers[entry.handlers[i]])(*this,entry.pRegs,(Nuance &)(entry.nuances[FIXED_FIELD(i,0)]));
      }

      /*if (counter > oldcounter + 8000000)
      {
          FILE* f = fopen("op_stats.txt", "a");
          for (uint32 i = 0; i < 224; ++i)
              fprintf(f, "%3u %llu %s\n", i, nuanceHandlersCount[i], nuanceHandlersNames[i]);
          fprintf(f, "\n\n");
          fclose(f);
          oldcounter = counter;
      }*/
    }
  }

  static uint32 GetControlRegisterInputDependencies(const uint32 address, bool &bException);
  static uint32 GetControlRegisterOutputDependencies(const uint32 address, bool &bException);

  static void DecodeInstruction_RCU16(const uint8* const iPtr, InstructionCacheEntry* const entry,       uint32* const immExt);
  static void DecodeInstruction_ECU16(const uint8* const iPtr, InstructionCacheEntry* const entry, const uint32* const immExt);
  static void DecodeInstruction_ECU32(const uint8* const iPtr, InstructionCacheEntry* const entry, const uint32* const immExt);
  static void DecodeInstruction_ALU16(const uint8* const iPtr, InstructionCacheEntry* const entry, const uint32* const immExt);
  static void DecodeInstruction_ALU32(const uint8* const iPtr, InstructionCacheEntry* const entry, const uint32* const immExt);
  void DecodeInstruction_MEM16(const uint8* const iPtr, InstructionCacheEntry* const entry, const uint32* const immExt);
  void DecodeInstruction_MEM32(const uint8* const iPtr, InstructionCacheEntry* const entry, const uint32* const immExt);
  static void DecodeInstruction_MUL16(const uint8* const iPtr, InstructionCacheEntry* const entry, const uint32* const immExt);
  static void DecodeInstruction_MUL32(const uint8* const iPtr, InstructionCacheEntry* const entry, const uint32* const immExt);

  bool TestConditionCode(const uint32 whichCondition) const;
  
  MPE() : superBlock(SuperBlock(this)) {} //!! meh
  void Init(const uint32 index, uint8* mainBusPtr, uint8* systemBusPtr, uint8* flashEEPROMPtr);
  ~MPE();

#if 0
  bool LoadBinaryFile(uchar *filename, bool bIRAM);
#endif

#if 0
  bool ExecuteUntilAddress(const uint32 address)
  {
    breakpointAddress = address;
    const bool status = FetchDecodeExecute();
    breakpointAddress = 0;

    return status;
  }
#endif

  inline void InvalidateICache()
  {
    numInterpreterCacheFlushes++;
    instructionCache->Invalidate();
  }

  inline void InvalidateICacheRegion(const uint32 start, const uint32 end)
  {
    numInterpreterCacheFlushes++;
    instructionCache->InvalidateRegion(start, end);
  }

  void WriteControlRegister(const uint32 address, const uint32 data);

  void SaveRegisters()
  {
    const uint32 tmp = tempCC;
    memcpy(tempreg_union,reg_union,sizeof(uint32)*48);
    tempCC = tmp; //!! ?? this was what happened with the old code, but was this intended??
  }

  uint32 ReadControlRegister(const uint32 address, const uint32 entrypRegs[48]);

  inline void Halt()
  {
    mpectl &= ~MPECTRL_MPEGO;
  }

  inline void Go()
  {
    mpectl |= MPECTRL_MPEGO;
  }

  inline void TriggerInterrupt(const uint32 which)
  {
    intsrc |= which;
    Syscall_InterruptTriggered(*this);
  }

  inline void *GetPointerToMemory() const
  {
    return (void*)dtrom;
  }

  inline uint8 *GetPointerToMemoryBank(const uint32 address) const
  {
    return bankPtrTable[address >> 28] + (address & MPE_VALID_MEMORY_MASK);
  }

  void UpdateInvalidateRegion(const uint32 start, const uint32 length);

  void PrintInstructionCachePacket(char *buffer, size_t bufSize, const uint32 address);
  void PrintInstructionCachePacket(char *buffer, size_t bufSize, const InstructionCacheEntry &entry);

  void Reset();

  bool LoadCoffFile(const char * const filename, bool bSetEntryPoint = true, int handle = -1);
  bool LoadNuonRomFile(const char * const filename);
};

#endif
