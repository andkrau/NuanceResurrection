#ifndef NuonEnvironmentH
#define NuonEnvironmentH

#include "audio.h"
#include "basetypes.h"
#include "mpe.h"
#include "NuonMemoryManager.h"
#include "SuperBlock.h"
#include "Video.h"
#include "FlashEEPROM.h"

enum ConfigTokenType
{
  CONFIG_EOF,
  CONFIG_UNKNOWN,
  CONFIG_RESERVED,
  CONFIG_COMMENT,
  CONFIG_VARIABLE_START,
  CONFIG_VARIABLE_FINISH,
  CONFIG_STRING,
};

class NuonEnvironment
{
public:
  NuonEnvironment();
  ~NuonEnvironment();

  void WriteFile(MPE *pMPE, uint32 fd, uint32 len, uint32 buf);
  void *GetPointerToMemory(MPE *mpe,uint32 address, bool bCheckAddress = true);
  void InitBios(void);
  void InitAudio(void);
  void CloseAudio(void);
  void MuteAudio(uint32 param);
  void StopAudio(void);
  void RestartAudio(void);
  void StartAudio(void);
  void SetAudioVolume(uint32 volume);
  void SetAudioPlaybackRate(uint32 rate);
  bool IsAudioHalfInterruptEnabled(void)
  {
    return (nuonAudioChannelMode & ENABLE_HALF_INT);
  }

  bool IsAudioWrapInterruptEnabled(void)
  {
    return (nuonAudioChannelMode & ENABLE_WRAP_INT);
  }

  bool IsAudioSampleInterruptEnabled(void)
  {
    return (nuonAudioChannelMode & ENABLE_SAMP_INT);
  }

  uint32 GetInterruptCycleRate(uint32 newMode);

  char *GetDVDBase()
  {
    return dvdBase;
  }


  void TriggerAudioInterrupt(void);
  void TriggerVideoInterrupt(void);

  MPE *mpe[4];
  NuonMemoryManager nuonMemoryManager;
  uint8 *mainBusDRAM;
  uint8 *systemBusDRAM;
  FlashEEPROM *flashEEPROM;
  uint32 cycleCounter;
  volatile uint32 vblank_frequency;
  uint32 pendingCommRequests;
  uint32 mainChannelUpperLimit, mainChannelLowerLimit;
  uint32 overlayChannelUpperLimit, overlayChannelLowerLimit;

  void RegisterMainWindowHandle(uint32 handle)
  {
    mainWindowHandle = handle;
  }

  //Last accepted value stored using _AudioSetChannelMode
  uint32 nuonAudioChannelMode;
  //Nuon audio buffer size in bytes
  //Supported values are 1K/2K/4K/8K/16K/32K/64K
  uint32 nuonAudioBufferSize;
  //PC pointer to Nuon audio buffer in main bus or system bus DRAM
  uint8 *pNuonAudioBuffer;
  //Bitflag value passed back as return value in _AudioQuerySampleRates
  //The constructor initializes this variable.  Supported rates are
  //16000/22050/24000/32000/44100/48000/64000/88200/96000
  uint32 nuonSupportedPlaybackRates;
  //Selected playback rate stored as (channel) samples per second
  uint32 nuonAudioPlaybackRate;

  bool bInterlaced;
  bool bProcessorStartStopChange;
  bool bMainBufferModified;
  bool bOverlayBufferModified;
  bool bSoundDeviceChosen;
  bool bUseCycleBasedTiming;
  volatile bool bRenderVideo;
  uint32 cyclesPerAudioInterrupt;
  uint32 fps;
  volatile uint32 cyclesPerVideoDisplay;
  uint32 audioInterruptCycleCount;
  uint32 videoDisplayCycleCount;
  uint32 whichAudioInterrupt;
  uint32 GetBufferSize(uint32 channelMode);
  CompilerOptions compilerOptions;
  VideoOptions videoOptions;
  void SetDVDBaseFromFileName(char *filename);
private:
  char *dvdBase;
  bool bAudioInterruptsEnabled;
  uint32 mainWindowHandle;
  ConfigTokenType ReadConfigLine(FILE *file, char *buf);
  bool LoadConfigFile(char *fileName);
};

#endif
