#include "basetypes.h"
#ifdef ENABLE_EMULATION_MESSAGEBOXES
#include <windows.h>
#endif

#include <string>
#include "byteswap.h"
#include "media.h"
#include "mpe.h"
#include "audio.h"
#include "Bios.h"
#include "dma.h"
#include "file.h"
#include "joystick.h"
#include "memory.h"
#include "mpe_alloc.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "PresentationEngine.h"
#include "timer.h"
#include "video.h"

extern NuonEnvironment nuonEnv;
extern VidChannel structOverlayChannel;
extern VidChannel structMainChannel;
extern VidChannel structMainChannelPrev;
extern VidChannel structOverlayChannelPrev;

void KPrintf(MPE &mpe);

const char *BiosRoutineNames[512] = {
"CommSend",
"CommSendInfo",
"CommRecvInfo",
"CommRecvInfoQuery",
"CommSendRecv",
"CommSendRecvInfo",
"ControllerInitialize",
"ControllerExtendedInfo",
"TimeOfDay",
"DCacheSyncRegion",
"DCacheSync",
"DCacheInvalidateRegion",
"DCacheFlush",
"TimerInit",
"TimeElapsed",
"TimeToSleep",
"MPEAlloc",
"MPEAllocSpecific",
"MPEFree",
"MPEsAvailable",
"IntSetVectorX",
"IntGetVector",
"VidSync",
"VidSetup",
"VidConfig",
"VidQueryConfig",
"VidChangeBase",
"VidChangeScroll",
"VidSetCLUTRange",
"BiosInit",
"BiosExit",
"BiosReboot",
"BiosPoll",
"BiosPauseMsg",
"AudioQueryChannelMode",
"AudioSetChannelMode",
"AudioQuerySampleRate",
"AudioSetSampleRate",
"AudioReset",
"AudioMute",
"AudioSetDMABuffer",
"MemInit",
"MemAdd",
"MemAlloc",
"MemFree",
"MemLocalScratch",
"MemLoadCoffX",
"DownloadCoff",
"StreamLoadCoff",
"DMALinear",
"DMABiLinear",
"FileOpen",
"FileClose",
"FileRead",
"FileWrite",
"FileIoctl",
"FileFstat",
"FileStat",
"FileIsatty",
"FileLseek",
"FileLink",
"FileLstat",
"FileUnlink",
"NetAccept",
"NetBind",
"NetConnect",
"NetGethostname",
"NetGetpeername",
"NetGetsockname",
"NetGetsockopt",
"NetListen",
"NetRecv",
"NetRecvfrom",
"NetRecvmsg",
"NetSend",
"NetSendmsg",
"NetSendto",
"NetSethostname",
"NetSetsockopt",
"NetShutdown",
"NetSocket",
"CommSendDirect",
"comm_recv",
"comm_query",
"_serial_delay",
"_serial_read",
"_serial_write",
"_serial_write_direct",
"MediaOpen",
"MediaClose",
"MediaGetDevicesAvailable",
"MediaGetInfo",
"MediaGetStatus",
"MediaRead",
"MediaWrite",
"MediaIoctl",
"spinwait",
"CacheConfigX",
"LoadGame",
"LoadPE",
"Dma_wait",
"Dma_do",
"PatchJumptable",
"BiosResume",
"MPEStop",
"MPERun",
"MPEWait",
"MPEReadRegister",
"MPEWriteRegister",
"SetParentalControl",
"GetParentalControl",
"BiosGetInfo",
"LoadTest",
"MPELoad",
"MPEAllocThread",
"MediaInitMPE",
"MediaShutdownMPE",
"SecureForPE",
"StartImageValid",
"SetStartImage",
"GetStartImage",
"FindName",
"DeviceDetect",
"MPERunThread",
"BiosIRMask",
"DiskChange",
"DiskGetTotalSlots",
"pf_add_driver",
"VidSetBorderColor",
"DisplayBootImage",
"serial_write_cmd",
"GetMemDevice",
"WriteMemDevSector",
"ReadMemDev",
"AttachFsDevice",
"DiskEject",
"DiskRetract",
"GetSystemSettingsB",
"GetSystemSetting?",
"SetSystemSetting??",
"GetSystemSettingLe",
"LoadSystemSettings",
"StoreSystemSetting",
"mount",
"MPEStatus",
"kprintf",
"ControllerPollRate",
"VidSetOutputType",
"LoadDefaultSystemSettings",
"SetISRExitHook",
"CompatibilityMode"
};

void UnimplementedFileHandler(MPE &mpe)
{
  //::MessageBox(NULL,"This BIOS Handler does nothing","Unimplemented File Routine",MB_OK);
}

void UnimplementedMediaHandler(MPE &mpe)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL,"This BIOS Handler does nothing","Unimplemented Media Routine",MB_OK);
#endif
}

void UnimplementedCacheHandler(MPE &mpe)
{
  //::MessageBox(NULL,"This BIOS Handler does nothing","Unimplemented Cache Routine",MB_OK);
}

void UnimplementedCommHandler(MPE &mpe)
{
#ifdef ENABLE_EMULATION_MESSAGEBOXES
  MessageBox(NULL,"This BIOS Handler does nothing","Unimplemented Comm Routine",MB_OK);
#endif
}

void NullBiosHandler(MPE &mpe)
{
  //char msg[512];
  //sprintf(msg,"This BIOS Handler does nothing: %ld",(mpe->pcexec >> 1) & 0xFFUL);
  //::MessageBox(NULL,msg,"Unimplemented BIOS Routine",MB_OK);
}

void AssemblyBiosHandler(MPE &mpe)
{
}

void WillNotImplement(MPE &mpe)
{
  //char msg[512];
  //sprintf(msg,"This BIOS Handler does nothing: %ld",(mpe->pcexec >> 1) & 0xFFUL);
  //::MessageBox(NULL,msg,"Unimplemented BIOS Routine",MB_OK);
}

void SetISRExitHook(MPE &mpe)
{
  uint32 newvec = mpe.regs[0];
  SwapScalarBytes(&newvec);
  *((uint32 *)nuonEnv.GetPointerToMemory(mpe,ISR_EXIT_HOOK_ADDRESS)) = newvec;
}

bool InstallCommHandler(MPE &mpe, uint32 address, uint32 *handlerList, uint32 *nHandlers)
{
  uint32 numHandlers = *((uint32 *)nuonEnv.GetPointerToMemory(mpe,NUM_INSTALLED_COMMRECV_HANDLERS_ADDRESS));
  uint32 *pHandlers = handlerList;
  bool bFound = false;

  SwapScalarBytes(&address);
  SwapScalarBytes(&numHandlers);

  uint32 i;
  for(i = 0; i < numHandlers; i++)
  {
    if(address == *pHandlers)
    {
      bFound = true;
      break;
    }
    pHandlers++;
  }

  //Didn't find it and the list is full
  if(i >= MAX_RECV_HANDLERS)
  {
    *nHandlers = MAX_RECV_HANDLERS;
    return false;
  }

  //Didn't find it in the list and there is room so install it
  if(!bFound)
  {
    *pHandlers = address;
    i++;
    *nHandlers = i;
    SwapScalarBytes(&i);
    *((uint32 *)nuonEnv.GetPointerToMemory(mpe,NUM_INSTALLED_COMMRECV_HANDLERS_ADDRESS)) = i;
    return true;
  }

  //Found it in the list so uninstall it, shifting remaining handlers by one
  while(i < (MAX_RECV_HANDLERS - 1))
  {
    *pHandlers = *(pHandlers+1);
    pHandlers++;
    i++;
  }
  *pHandlers = 0;
  
  numHandlers--;
  *nHandlers = numHandlers;
  SwapScalarBytes(&numHandlers);
  *((uint32 *)nuonEnv.GetPointerToMemory(mpe,NUM_INSTALLED_COMMRECV_HANDLERS_ADDRESS)) = numHandlers;
  return false;
}

void IntGetVector(MPE &mpe)
{
  const uint32 which = mpe.regs[0];

  if(which < 32)
  {
    const uint32* const InterruptVectors = (uint32 *)nuonEnv.GetPointerToMemory(mpe,INTERRUPT_VECTOR_ADDRESS);
    mpe.regs[0] = InterruptVectors[which];
    SwapScalarBytes(&(mpe.regs[0]));
  }
  else
    mpe.regs[0] = 0;
}

void IntSetVector(MPE &mpe)
{
  const uint32 which = mpe.regs[0];
  const uint32 newvec = mpe.regs[1];

  mpe.regs[0] = 0;

  // special handling of kIntrVideo (which==31) and kIntrAudio (which==27): handled via the trigger interrupts due to the vid timer and the audio stream callback!

  if(which < 32)
  {
    if(which == 4)
    {
      if(newvec < MPE_ADDR_SPACE_BASE)
      {
        return;
      }

      uint32 * const recvHandlers = ((uint32 *)nuonEnv.GetPointerToMemory(mpe,COMMRECV_HANDLER_LIST_ADDRESS));
      uint32 numHandlers;
      if(InstallCommHandler(mpe, newvec, recvHandlers, &numHandlers))
      {
        mpe.regs[0] = newvec;
      }
    }
    else
    {
      uint32* const InterruptVectors = (uint32 *)nuonEnv.GetPointerToMemory(mpe,INTERRUPT_VECTOR_ADDRESS);
      mpe.regs[0] = InterruptVectors[which];

      if(!newvec)
      {
        //disable the interrupt
        mpe.inten1 &= (~(1UL << which));
      }
      else
      {
        //Not needed in this implementation, but needed if this IntSetVector is moved to aries assembly
        mpe.intsrc &= (~(1UL << which));
        //Enable the interrupt in case it was previously disabled
        mpe.inten1 |= (1UL << which);
      }

      InterruptVectors[which] = newvec;
      SwapScalarBytes(&InterruptVectors[which]);
    }
  }
}

void BiosExit(MPE &mpe)
{
  //const uint32 return_value = mpe.regs[0];
  mpe.Halt();
}

uint32 PatchJumptable(const uint32 vectorAddress, uint32 newEntry)
{
  uint32 oldEntryImmExt;
  uint32 oldEntryInst;
  uint32 immExt;
  uint32 inst;

  newEntry >>= 1;

  //create 64 bit JMP <newEntry>, nop instruction
  inst = 0x9220BA00UL | ((newEntry & 0x1FUL) << 16) | ((newEntry & 0x1FE0UL) >> 5);
  immExt = 0x88000000UL | ((newEntry & 0x7FFFE000UL) >> 4);
  //get the old entry stored in the BIOS vector address
  oldEntryImmExt = *((uint32 *)(&(nuonEnv.systemBusDRAM[vectorAddress - SYSTEM_BUS_BASE + 0])));
  oldEntryInst = *((uint32 *)(&(nuonEnv.systemBusDRAM[vectorAddress - SYSTEM_BUS_BASE + 4])));
  SwapScalarBytes(&inst);
  SwapScalarBytes(&immExt);
  SwapScalarBytes(&oldEntryImmExt);
  SwapScalarBytes(&oldEntryInst);
  //load the new entry into the BIOS vector
  *((uint32 *)(&(nuonEnv.systemBusDRAM[vectorAddress - SYSTEM_BUS_BASE + 0]))) = immExt;
  *((uint32 *)(&(nuonEnv.systemBusDRAM[vectorAddress - SYSTEM_BUS_BASE + 4]))) = inst;

  //extract the old BIOS function address from the previous entry's JMP instruction
  const uint32 oldAddress = (((oldEntryImmExt & 0x7FFFE00UL) << 4) | ((oldEntryInst & 0xFFUL) << 5) | ((oldEntryInst & 0x1F0000) >> 16)) << 1;

  return oldAddress;
}

void PatchJumptable(MPE &mpe)
{
  const uint32 vectorAddress = mpe.regs[0];
  const uint32 newAddress = mpe.regs[1];

  mpe.regs[0] = PatchJumptable(vectorAddress, newAddress);
}

void BiosGetInfo(MPE &mpe)
{
  mpe.regs[0] = 0x80760000;
}

NuonBiosHandler BiosJumpTable[256] = {
AssemblyBiosHandler, //_CommSend (0)
AssemblyBiosHandler, //_CommSendInfo (1)
AssemblyBiosHandler, //_CommRecvInfo (2)
AssemblyBiosHandler, //_CommRecvInfoQuery (3)
AssemblyBiosHandler, //_CommSendRecv (4)
AssemblyBiosHandler, //_CommSendRecvInfo (5)
ControllerInitialize, //_ControllerInitialize (6)
NullBiosHandler, //_ControllerExtendedInfo (7)
TimeOfDay, //_TimeOfDay (8)
UnimplementedCacheHandler, //_DCacheSyncRegion (9)
UnimplementedCacheHandler, //_DCacheSync (10)
UnimplementedCacheHandler, //_DCacheInvalidateRegion (11)
UnimplementedCacheHandler, //_DCacheFlush (12)
TimerInit, //_TimerInit (13)
TimeElapsed, //_TimeElapsed (14)
AssemblyBiosHandler, //_TimeToSleep (15)
MPEAlloc, //_MPEAlloc (16)
MPEAllocSpecific, //_MPEAllocSpecific (17)
MPEFree, //_MPEFree (18)
MPEsAvailable, //_MPEsAvailable (19)
IntSetVector, //_IntSetVector (20)
IntGetVector, //_IntGetVector (21)
VidSync, //_VidSync (22)
VidSetup, //_VidSetup (23)
VidConfig, //_VidConfig (24)
VidQueryConfig, //_VidQueryConfig (25)
VidChangeBase, //_VidChangeBase (26)
VidChangeScroll, //_VidChangeScroll (27)
VidSetCLUTRange, //_VidSetCLUTRange (28)
InitBios, //_BiosInit (29)
BiosExit, //_BiosExit (30)
NullBiosHandler, //_BiosReboot (31)
BiosPoll, //_BiosPoll (32)
BiosPauseMsg, //_BiosPauseMsg (33)
AudioQueryChannelMode, //_AudioQueryChannelMode (34)
AudioSetChannelMode, //_AudioSetChannelMode (35)
AudioQuerySampleRates, //_AudioQuerySampleRates (36)
AudioSetSampleRate, //_AudioSetSampleRate (37)
AudioReset, //_AudioReset (38)
AudioMute, //_AudioMute (39)
AudioSetDMABuffer, //_AudioSetDMABuffer (40)
MemInit, //_MemInit (41)
WillNotImplement, //_MemAdd (42)
MemAlloc, //_MemAlloc (43)
MemFree, //_MemFree (44)
MemLocalScratch, //_MemLocalScratch (45)
NullBiosHandler, //_MemLoadCoffX (46)
NullBiosHandler, //_DownloadCoff (47)
NullBiosHandler, //_StreamLoadCoff (48)
DMALinear, //_DMALinear (49)
DMABiLinear, //_DMABiLinear (50)
FileOpen, //_FileOpen (51)
FileClose, //_FileClose (52)
FileRead, //_FileRead (53)
FileWrite, //_FileWrite (54)
FileIoctl, //_FileIoctl (55)
FileFstat, //_FileFstat (56)
FileStat, //_FileStat (57)
FileIsatty, //_FileIsatty (58)
FileLseek, //_FileLseek (59)
FileLink, //_FileLink (60)
FileLstat, //_FileLstat (61)
FileUnlink, //_FileUnlink (62)
NullBiosHandler, //_NetAccept (63)
NullBiosHandler, //_NetBind (64)
NullBiosHandler, //_NetConnect (65)
NullBiosHandler, //_NetGethostname (66)
NullBiosHandler, //_NetGetpeername (67)
NullBiosHandler, //_NetGetsockname (68)
NullBiosHandler, //_NetGetsockopt (69)
NullBiosHandler, //_NetListen (70)
NullBiosHandler, //_NetRecv (71)
NullBiosHandler, //_NetRecvfrom (72)
NullBiosHandler, //_NetRecvmsg (73)
NullBiosHandler, //_NetSend (74)
NullBiosHandler, //_NetSendmsg (75)
NullBiosHandler, //_NetSendto (76)
NullBiosHandler, //_NetSethostname (77)
NullBiosHandler, //_NetSetsockopt (78)
NullBiosHandler, //_NetShutdown (79)
NullBiosHandler, //_NetSocket (80)
AssemblyBiosHandler, //_comm_send (CommSendDirect) (81)
AssemblyBiosHandler, //_comm_recv (82)
AssemblyBiosHandler, //_comm_query (83)
WillNotImplement, //_serial_delay (84)
WillNotImplement, //_serial_read (85)
WillNotImplement, //_serial_write (86)
WillNotImplement, //_serial_write_direct (87)
MediaOpen, //_MediaOpen (88)
MediaClose, //_MediaClose (89)
MediaGetDevicesAvailable, //_MediaGetDevicesAvailable (90)
MediaGetInfo, //_MediaGetInfo (91)
UnimplementedMediaHandler, //_MediaGetStatus (92)
MediaRead, //_MediaRead (93)
MediaWrite, //_MediaWrite (94)
MediaIoctl, //_MediaIoctl (95)
SpinWait, //_spinwait (96)
UnimplementedCacheHandler, //_CacheConfigX (97)
NullBiosHandler, //_LoadGame (98)
NullBiosHandler, //_LoadPE (99)
DMAWait, //_Dma_wait (100)
DMADo, //_Dma_do (101)
PatchJumptable, //_PatchJumptable (102)
NullBiosHandler, //_BiosResume (103)
MPEStop, //_MPEStop (104)
MPERun, //_MPERun (105)
AssemblyBiosHandler, //_MPEWait (106)
MPEReadRegister, //_MPEReadRegister (107)
MPEWriteRegister, //_MPEWriteRegister (108)
NullBiosHandler, //_SetParentalControl (109)
NullBiosHandler, //_GetParentalControl (110)
AssemblyBiosHandler, //_BiosGetInfo (111)
NullBiosHandler, //_LoadTest (112)
MPELoad, //_MPELoad (113)
NullBiosHandler, //_MPEAllocThread (114)
MediaInitMPE, //_MediaInitMPE (115)
MediaShutdownMPE, //_MediaShutdownMPE (116)
NullBiosHandler, //_SecureForPE (117)
NullBiosHandler, //_StartImageValid (118)
NullBiosHandler, //_SetStartImage (119)
NullBiosHandler, //_GetStartImage (120)
NullBiosHandler, //_FindName (121)
DeviceDetect, //_DeviceDetect (122)
MPERunThread, //_MPERunThread (123)
NullBiosHandler, //_BiosIRMask (124)
NullBiosHandler, //_DiskChange (125)
NullBiosHandler, //_DiskGetTotalSlots (126)
NullBiosHandler, //_pf_add_driver (127)
VidSetBorderColor, //_VidSetBorderColor (128)
NullBiosHandler, //_DisplayBootImage (129)
WillNotImplement, //serial_write_cmd (130)
NullBiosHandler, //_GetMemDevice (131)
NullBiosHandler, //_WriteMemDevSector (132)
NullBiosHandler, //_ReadMemDev (133)
NullBiosHandler, //_AttachFsDevice (134)
NullBiosHandler, //_DiskEject (135)
NullBiosHandler, //_DiskRetract (136)
NullBiosHandler, //_GetSystemSettingsB (137)
NullBiosHandler, //_GetSystemSetting (138)
NullBiosHandler, //_SetSystemSetting (139)
NullBiosHandler, //_GetSystemSettingLength (140)
NullBiosHandler, //_LoadSystemSettings (141)
NullBiosHandler, //_StoreSystemSetting (142)
NullBiosHandler, //_mount (143)
MPEStatus, //_MPEStatus (144)
KPrintf, //_kprintf (145)
NullBiosHandler, //_ControllerPollRate (146)
WillNotImplement, //_VidSetOutputType (147)
NullBiosHandler, //_LoadDefaultSystemSettings (148)
SetISRExitHook, //_SetISRExitHook (149)
NullBiosHandler //_CompatibilityMode (150)
};


void BiosPauseMsg(MPE &mpe)
{
  //const uint32 rval = mpe.regs[0];
  //char *msg = (char *)nuonEnv.GetPointerToMemory(mpe,mpe.regs[1]);
  //uint8 *framebuffer = (uint8 *)nuonEnv.GetPointerToMemory(mpe,mpe.regs[2]);

  //allow application to continue
  mpe.regs[0] = kPollContinue;
}

void BiosPoll(MPE &mpe)
{
  //no events
  mpe.regs[0] = 0;
}

void InitBios(MPE &mpe)
{
  bool loadStatus = nuonEnv.mpe[3].LoadCoffFile("bios.cof",false);

  if(!loadStatus)
  {
    char tmp[1024];
    GetModuleFileName(NULL, tmp, 1024);
    string tmps(tmp);
    size_t idx = tmps.find_last_of('\\');
    if (idx != string::npos)
      tmps = tmps.substr(0, idx+1);
    loadStatus = nuonEnv.mpe[3].LoadCoffFile((tmps+"bios.cof").c_str(),false);
    if(!loadStatus)
      ::MessageBox(NULL,"Missing File!","Could not load bios.cof",MB_OK);
  }

  //Reset MPEAlloc flags to reset values
  ResetMPEFlags(mpe);

  //MEMORY MANAGEMENT INITIALIZATION
  MemInit(mpe);

  //HAL Setup
  //HalSetup();

  for(uint32 i = 0; i < 4; i++)
  {
    nuonEnv.mpe[i].WriteControlRegister(0xB0UL, INTVEC1_HANDLER_ADDRESS);
    nuonEnv.mpe[i].WriteControlRegister(0xC0UL, INTVEC2_HANDLER_ADDRESS);

    if(i == 3)
    {
      nuonEnv.mpe[i].WriteControlRegister(0x110UL, 0);
      //Commrecv needs to be enabled immediately as level2 because some programs use CommRecv and and CommRecvQuery to obtain comm packets
      //rather than installing a user comm ISR
      nuonEnv.mpe[i].WriteControlRegister(0x130UL, kIntrCommRecv);
    }
    else if(i == 0)
    {
      //Don't need to set anything for level1... InitMediaMPE will enable commrecv when minibios is loaded
      //nuonEnv.mpe[i].WriteControlRegister(0x110UL, INT_COMMRECV);
      nuonEnv.mpe[i].WriteControlRegister(0x130UL, kIntrHost);
    }
    else
    {
      nuonEnv.mpe[i].WriteControlRegister(0x110UL, 0);
      nuonEnv.mpe[i].WriteControlRegister(0x130UL, kIntrHost);
    }
  }

  //Patch the jump table for the first 151 entries
  for(uint32 i = 0; i < ((0x4B0UL >> 3) + 1); i++)
  {
    if(BiosJumpTable[i] != AssemblyBiosHandler)
    {
      PatchJumptable(SYSTEM_BUS_BASE + (i << 3UL), ROM_BIOS_BASE + (i << 1));
    }
  }

  //Fill Bios Handler entries from 151 to 255 to NullBiosHandler
  for(uint32 i = ((0x4B0UL >> 3) + 1); i <= 255; i++)
  {
    BiosJumpTable[i] = NullBiosHandler;
  }

  //DVD JUMP TABLE INITIALIZATION
  InitDVDJumpTable();

  //DEFAULT VIDCHANNEL INITIALIZATION
  memset(&structMainChannel,0,sizeof(VidChannel));
  structMainChannel.base = 0x40000000;
  structMainChannel.src_width = VIDEO_WIDTH;
  structMainChannel.src_height = VIDEO_HEIGHT;
  structMainChannel.dest_width = VIDEO_WIDTH;
  structMainChannel.dest_height = VIDEO_HEIGHT;
  structMainChannel.dmaflags = (4 << 4);

  memset(&structOverlayChannel,0,sizeof(VidChannel));
  structOverlayChannel.base = 0x40000000;
  structOverlayChannel.src_width = VIDEO_WIDTH;
  structOverlayChannel.src_height = VIDEO_HEIGHT;
  structOverlayChannel.dest_width = VIDEO_WIDTH;
  structOverlayChannel.dest_height = VIDEO_HEIGHT;
  structOverlayChannel.dmaflags = (4 << 4);
  structOverlayChannel.alpha = 0xFF;

  structMainChannelPrev.base = 0;
  structMainChannelPrev.src_width = 0;
  structOverlayChannelPrev.base = 0;
  structOverlayChannelPrev.src_width = 0;
  
  //MINIBIOS INITIALIZATION

  //Start up the minibios on MPE0
  MediaInitMPE(0);

  //TIMER INITIALIZATION
  TimerInit(0,1000*1000/200);      // triggers sys0 int at 200Hz (according to BIOS doc)
  TimerInit(1,0);
  TimerInit(2,1000*1000/VIDEO_HZ); // triggers video int at ~50 or 60Hz
}


void KPrintf(MPE &mpe)
{ 
#ifdef ENABLE_EMULATION_MESSAGEBOXES // kprintf will do nothing if not compiled with ENABLE_EMULATION_MESSAGEBOXES
  uint32 pStr = *((uint32 *)(nuonEnv.GetPointerToMemory(mpe,mpe.regs[31],true)));

  SwapScalarBytes(&pStr);
  if(pStr)
  {
    const char* const str = (const char*)(nuonEnv.GetPointerToMemory(mpe, pStr, true));
   // MessageBox(NULL, str, "kprintf", MB_OK);
    
    extern int kprintfDebug; // config file defined with [kprintf] 0 none, 1 popup, 2 kprintf.txt, 3 both 1 & 2
    extern FILE* kprintf_log_fp;//both externs are from NounEnvironment.cpp

    if (kprintfDebug == 0)
        return;
    
    if (kprintfDebug == 1 || kprintfDebug == 3)
        MessageBox(NULL,str,"kprintf",MB_OK);
    
    if (kprintfDebug == 2 || kprintfDebug == 3)
    {
        if (!kprintf_log_fp)
        {
            kprintf_log_fp = fopen("kprintf.txt", "w");

        }
        fwrite(str, strlen(str), 1, kprintf_log_fp);
    }
  }
#endif
}


/*
  
// Note: kprintf (on Nuance at least) doesn't parse the string variables, so it has to be done before calling kprintf, example usage


#define DEBUG

#ifdef DEBUG
extern void kprintf(const char *, ...);
#include <Nuon/msprintf.h>

void debug(const char *format, ...)
{
	char buff[1024];
	va_list args;

	va_start(args, format);
	mvsprintf(buff, format, args);
	va_end(args);

	kprintf(buff);
}
#else
#define debug(...)
#endif


//config instructions:

; Enables kprintf for printf debugging (via console/TTY/serial)
; To use kprintf you must define the prototype as:
; extern void kprintf(const char *, ...);
; And link with -lrombios
; kprint values are 0-3
; 0 does nothing
; 1 halts and popup message for each kprintf
; 2 writes log file to kprintf.txt in the folder nuance.exe was called in
; 3 does both 2 and 3
; 4+ does nothing
[kprintf]
2

*/
