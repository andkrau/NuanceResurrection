#ifndef NuonEnvironmentH
#define NuonEnvironmentH

#include "basetypes.h"
#include "audio.h"
#include "mpe.h"
#include "NuonMemoryManager.h"
#include "SuperBlock.h"
#include "video.h"
#include "FlashEEPROM.h"

enum ConfigTokenType
{
  CONFIG_EOF,
  CONFIG_UNKNOWN,
  CONFIG_RESERVED,
  CONFIG_COMMENT,
  CONFIG_VARIABLE_START,
  CONFIG_VARIABLE_FINISH,
  CONFIG_STRING
};

class NuonEnvironment
{
public:
  NuonEnvironment() {}
  void Init();
  ~NuonEnvironment();

  void WriteFile(MPE &mpe, uint32 fd, uint32 buf, uint32 len);
  void *GetPointerToMemory(const MPE &mpe, const uint32 address, const bool bCheckAddress = true);
  void *GetPointerToSystemMemory(const uint32 address, const bool bCheckAddress = true);
  void InitBios(void);
  void InitAudio(void);
  void CloseAudio(void);
  void MuteAudio(const bool mute);
  void StopAudio(void);
  void RestartAudio(void);
  void SetAudioVolume(uint32 volume);
  void SetAudioPlaybackRate(uint32 rate);

  bool IsAudioHalfInterruptEnabled(void) const
  {
    return (nuonAudioChannelMode & ENABLE_HALF_INT);
  }
  bool IsAudioWrapInterruptEnabled(void) const
  {
    return (nuonAudioChannelMode & ENABLE_WRAP_INT);
  }
  bool IsAudioSampleInterruptEnabled(void) const
  {
    return (nuonAudioChannelMode & ENABLE_SAMP_INT);
  }

  char *GetDVDBase() const
  {
    return dvdBase;
  }

  void TriggerAudioInterrupt(void);
  void TriggerVideoInterrupt(void);

  void SetDVDBaseFromFileName(const char* const filename);

  MPE mpe[4];
  NuonMemoryManager nuonMemoryManager;
  uint8 *mainBusDRAM;
  uint8 *systemBusDRAM;
  FlashEEPROM *flashEEPROM;
  uint32 cycleCounter;
  uint32 pendingCommRequests;
  uint32 mainChannelUpperLimit, mainChannelLowerLimit;
  uint32 overlayChannelUpperLimit, overlayChannelLowerLimit;

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
  bool bUseCycleBasedTiming; //!! unfinished
  bool whichAudioInterrupt;

  volatile bool trigger_render_video;

  uint32 cyclesPerAudioInterrupt;
  uint32 audioInterruptCycleCount;
  uint64 videoDisplayCycleCount;
  uint32 GetBufferSize(uint32 channelMode);
  CompilerOptions compilerOptions;

private:
  ConfigTokenType ReadConfigLine(FILE *file, char *buf);
  bool LoadConfigFile(const char * const fileName);

  char *dvdBase;
  bool bAudioInterruptsEnabled;
};

#endif
