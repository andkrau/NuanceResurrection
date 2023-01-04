#ifndef NuonEnvironmentH
#define NuonEnvironmentH

#include "basetypes.h"
#include <limits.h>
#include <windows.h>
#include "external\fmod-3.75\api\inc\fmod.h"
#include "audio.h"
#include "mpe.h"
#include "InputManager.h"
#include "NuonMemoryManager.h"
#include "SuperBlock.h"
#include "FlashEEPROM.h"

enum class ConfigTokenType
{
  CONFIG_EOF,
  CONFIG_UNKNOWN,
  CONFIG_RESERVED,
  CONFIG_COMMENT,
  CONFIG_VARIABLE_START,
  CONFIG_VARIABLE_FINISH,
  CONFIG_STRING
};

class ControllerButtonMapping
{
public:
  ControllerButtonMapping(InputManager::InputType t = InputManager::JOYBUT, int i = 0, int si = 0) :
    type(t), idx(i), subIdx(si) {}

  InputManager::InputType type;
  int idx;
  int subIdx;

  bool operator==(const ControllerButtonMapping& o) const { return (type == o.type) && (idx == o.idx) && (subIdx == o.subIdx); }

  // Longest type name is "JOYAXIS" = 7 chars + 2 underscores + 2 ints + terminator
  static const size_t MAPPING_STRING_SIZE = 7 + 2 + (2 * 10) + 1;
  void toString(char* strOut, size_t len) const
  {
    sprintf_s(strOut, len, "%s_%d_%d", InputManager::InputTypeToStr(type), idx, subIdx);
  }

  static bool fromString(char* strIn, ControllerButtonMapping* mapping);
};

class NuonEnvironment
{
public:
  NuonEnvironment() : bFMODInitialized(false), audioChannel(0), dvdBase(nullptr), cfgFileName(nullptr),
    controllerDefaultMapping{ // Note that the default mapping should always map to keyboard as it will also be used if no other joypad/stick is connected!
        // Original key mappings included with Nuance
        ControllerButtonMapping(InputManager::KEY, 'R', 0), // CTRLR_BITNUM_BUTTON_C_RIGHT
        ControllerButtonMapping(InputManager::KEY, '3', 0), // CTRLR_BITNUM_BUTTON_C_UP
        ControllerButtonMapping(InputManager::KEY, 'W', 0), // CTRLR_BITNUM_BUTTON_C_LEFT
        ControllerButtonMapping(InputManager::KEY, 'F', 0), // CTRLR_BITNUM_BUTTON_B
        ControllerButtonMapping(InputManager::KEY, 'T', 0), // CTRLR_BITNUM_BUTTON_R
        ControllerButtonMapping(InputManager::KEY, 'Q', 0), // CTRLR_BITNUM_BUTTON_L
        ControllerButtonMapping(InputManager::JOYBUT, INT_MAX, INT_MAX), // CTRLR_BITNUM_UNUSED_1
        ControllerButtonMapping(InputManager::JOYBUT, INT_MAX, INT_MAX), // CTRLR_BITNUM_UNUSED_2
        ControllerButtonMapping(InputManager::KEY, VK_RIGHT, 0), // CTRLR_BITNUM_DPAD_RIGHT 8
        ControllerButtonMapping(InputManager::KEY, VK_UP, 0), // CTRLR_BITNUM_DPAD_UP
        ControllerButtonMapping(InputManager::KEY, VK_LEFT, 0), // CTRLR_BITNUM_DPAD_LEFT
        ControllerButtonMapping(InputManager::KEY, VK_DOWN, 0), // CTRLR_BITNUM_DPAD_DOWN
        ControllerButtonMapping(InputManager::KEY, 'S', 0), // CTRLR_BITNUM_BUTTON_NUON
        ControllerButtonMapping(InputManager::KEY, 'A', 0), // CTRLR_BITNUM_BUTTON_START
        ControllerButtonMapping(InputManager::KEY, 'D', 0), // CTRLR_BITNUM_BUTTON_A
        ControllerButtonMapping(InputManager::KEY, 'E', 0), // CTRLR_BITNUM_BUTTON_C_DOWN

        // Mapping to a Jag-Dapter with non-pro-controller.
        // Uses Keypad 2/6/8/4 for C-Pad U/R/D/L and l/3 for L/R.
        /*ControllerButtonMapping(InputManager::JOYBUT, 13, 0), // CTRLR_BITNUM_BUTTON_C_RIGHT
        ControllerButtonMapping(InputManager::JOYBUT,  9, 0), // CTRLR_BITNUM_BUTTON_C_UP
        ControllerButtonMapping(InputManager::JOYBUT, 11, 0), // CTRLR_BITNUM_BUTTON_C_LEFT
        ControllerButtonMapping(InputManager::JOYBUT,  1, 0), // CTRLR_BITNUM_BUTTON_B
        ControllerButtonMapping(InputManager::JOYBUT, 10, 0), // CTRLR_BITNUM_BUTTON_R
        ControllerButtonMapping(InputManager::JOYBUT,  8, 0), // CTRLR_BITNUM_BUTTON_L
        ControllerButtonMapping(InputManager::JOYBUT, INT_MAX, INT_MAX), // CTRLR_BITNUM_UNUSED_1
        ControllerButtonMapping(InputManager::JOYBUT, INT_MAX, INT_MAX), // CTRLR_BITNUM_UNUSED_2
        ControllerButtonMapping(InputManager::JOYAXIS, 0, 1), // CTRLR_BITNUM_DPAD_RIGHT
        ControllerButtonMapping(InputManager::JOYAXIS, 1, 0), // CTRLR_BITNUM_DPAD_UP
        ControllerButtonMapping(InputManager::JOYAXIS, 0, 0), // CTRLR_BITNUM_DPAD_LEFT
        ControllerButtonMapping(InputManager::JOYAXIS, 1, 1), // CTRLR_BITNUM_DPAD_DOWN
        ControllerButtonMapping(InputManager::JOYBUT,  4, 0), // CTRLR_BITNUM_BUTTON_NUON
        ControllerButtonMapping(InputManager::JOYBUT,  3, 0), // CTRLR_BITNUM_BUTTON_START
        ControllerButtonMapping(InputManager::JOYBUT,  2, 0), // CTRLR_BITNUM_BUTTON_A
        ControllerButtonMapping(InputManager::JOYBUT, 15, 0), // CTRLR_BITNUM_BUTTON_C_DOWN*/
    }
  {
    for (size_t i = 0; i < _countof(controller1Mapping); i++)
      controller1Mapping[i] = controllerDefaultMapping[i];
  }

  void Init();
  ~NuonEnvironment();

  void WriteFile(MPE &mpe, uint32 fd, uint32 buf, uint32 len);
  void *GetPointerToMemory(const MPE &mpe, const uint32 address, const bool bCheckAddress = true);
  void *GetPointerToSystemMemory(const uint32 address, const bool bCheckAddress = true);
  void InitBios();
  void InitAudio();
  void CloseAudio();
  void MuteAudio(const bool mute);
  void StopAudio();
  void RestartAudio();
  void SetAudioVolume(uint32 volume);
  void SetAudioPlaybackRate();

  bool IsAudioHalfInterruptEnabled() const
  {
    return (nuonAudioChannelMode & ENABLE_HALF_INT);
  }
  bool IsAudioWrapInterruptEnabled() const
  {
    return (nuonAudioChannelMode & ENABLE_WRAP_INT);
  }
  bool IsAudioSampleInterruptEnabled() const
  {
    return (nuonAudioChannelMode & ENABLE_SAMP_INT);
  }

  char *GetDVDBase() const
  {
    return dvdBase;
  }

  void TriggerAudioInterrupt()
  {
    if(bAudioInterruptsEnabled) {
      ScheduleInterrupt(INT_AUDIO);
    }
  }

  void TriggerVideoInterrupt()
  {
    ScheduleInterrupt(INT_VIDTIMER);
  }

  LONG schedule_intsrc;

  inline void ScheduleInterrupt(const uint32 which)
  {
    _InterlockedOr(&schedule_intsrc,(LONG)which);
  }

  inline void TriggerScheduledInterrupts()
  {
    const uint32 which = _InterlockedExchange(&schedule_intsrc,(LONG)0);
    if(which)
    {
      for(int i = 0; i < 4; ++i)
        mpe[i].TriggerInterrupt(which);
    }
  }

  void SetDVDBaseFromFileName(const char* const filename);

  void SetController1Joystick(const GUID& guid);
  void SetControllerButtonMapping(unsigned int ctrlrBitnum, const ControllerButtonMapping& mapping);
  const GUID& GetController1Joystick() const;
  int GetCTRLRBitnumFromMapping(const ControllerButtonMapping& mapping) const;
  const ControllerButtonMapping& GetMappingForCTRLRBitnum(unsigned int bitnum) const;

  bool SaveConfigFile(const char* const filename);

  MPE mpe[4];
  NuonMemoryManager nuonMemoryManager;

  uint8 mainBusDRAM[MAIN_BUS_SIZE];
  uint8 systemBusDRAM[SYSTEM_BUS_SIZE];
  FlashEEPROM flashEEPROM;

  uint32 cycleCounter;
  uint32 pendingCommRequests;
  uint32 mainChannelUpperLimit, mainChannelLowerLimit;
  uint32 overlayChannelUpperLimit, overlayChannelLowerLimit;

  //Last accepted value stored using _AudioSetChannelMode
  uint32 nuonAudioChannelMode;
  uint32 oldNuonAudioChannelMode;
  //Nuon audio buffer size in bytes
  //Supported values are 1K/2K/4K/8K/16K/32K/64K
  uint32 nuonAudioBufferSize;
  //PC pointer to Nuon audio buffer in main bus or system bus DRAM
  uint8 * volatile pNuonAudioBuffer;
  //Current position in the audio buffer for the sound output callback
  uint32 audio_buffer_offset;
  //0 or 1, depending if the audio callback has played since the last emulation cycle
  uint32 audio_buffer_played;
  //Bitflag value passed back as return value in _AudioQuerySampleRates
  //The constructor initializes this variable.  Supported rates are
  //16000/22050/24000/32000/44100/48000/64000/88200/96000
  uint32 nuonSupportedPlaybackRates;
  //Selected playback rate stored as (channel) samples per second
  uint32 nuonAudioPlaybackRate;

  bool bInterlaced;
  bool bProcessorStartStopChange;
  bool whichAudioInterrupt;

  volatile bool trigger_render_video;

  uint32 cyclesPerAudioInterrupt;
  uint32 audioInterruptCycleCount;
  uint64 videoDisplayCycleCount;
  uint32 GetBufferSize(uint32 channelMode);
  CompilerOptions compilerOptions;

  uint32 MPE3wait_fieldCounter; // tells how many vsyncs the MPE3 emulation is stalled due to a VidSync call
  int32 timer_rate[3]; // sysTimer0, sysTimer1 and vidTimer

  bool bAutomaticLoadPopup;

  bool bUseCRTshader;

private:
  bool StrToCtrlrBitnum(const char* str, unsigned int *bitnum);
  bool ParseJoyButtonConf(char buf[1025], unsigned int* bitnum, ControllerButtonMapping* mapping);
  ConfigTokenType ReadConfigLine(FILE *file, char buf[1025]);
  bool LoadConfigFile(const char * const fileName);

  char *cfgFileName;
  char *dvdBase;
  bool bAudioInterruptsEnabled;

  ControllerButtonMapping controllerDefaultMapping[16]; // Indixed by CTRLR_BITNUM_* macros from joystick.h
  ControllerButtonMapping controller1Mapping[16];       // dto.
  GUID controller1Di8Dev;

  // FMOD specific stuff
  bool bFMODInitialized;
  FSOUND_STREAM* audioStream;
  int audioChannel;
};

#endif
