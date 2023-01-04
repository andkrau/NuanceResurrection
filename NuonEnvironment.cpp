#include "basetypes.h"
#ifdef ENABLE_EMULATION_MESSAGEBOXES
#include <windows.h>
#endif
#include <combaseapi.h>

#include "GLWindow.h"
#include "audio.h"
#include "Bios.h"
#include "file.h"
#include "mpe.h"
#include "timer.h"
#include "video.h"
#include "joystick.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"

extern GLWindow display;

extern VidDisplay structMainDisplay;

extern char **pArgs;

  inline void ConvertNuonAudioData(const uint8 * const __restrict pNuonAudioBuffer, uint8 * const __restrict pPCAudioBuffer, const uint32 numBytes)
  {
    assert((numBytes % 4) == 0); //!! only handles 16bit stereo at the moment

    for(uint32 byteCount = 0; byteCount < numBytes; byteCount += 4)
    {
      pPCAudioBuffer[byteCount  ] = pNuonAudioBuffer[byteCount+1];
      pPCAudioBuffer[byteCount+1] = pNuonAudioBuffer[byteCount  ];
      pPCAudioBuffer[byteCount+2] = pNuonAudioBuffer[byteCount+3];
      pPCAudioBuffer[byteCount+3] = pNuonAudioBuffer[byteCount+2];
    }
  }

  static schar F_CALLBACKAPI StreamCallback(FSOUND_STREAM *stream, void *buff, int len, void* userdata)
  {
    NuonEnvironment * const nuonEnv = (NuonEnvironment*)userdata;
    const uint8* const pNuonAudioBuffer = nuonEnv->pNuonAudioBuffer;

    if(!buff || !pNuonAudioBuffer)
      return FALSE;

    assert(len == (nuonEnv->nuonAudioBufferSize>>1));

    _InterlockedExchange(&nuonEnv->audio_buffer_played, 1); // signal the NuanceMain loop that we did do some sound update
    ConvertNuonAudioData(pNuonAudioBuffer + nuonEnv->audio_buffer_offset, (uint8*)buff, len);
    return TRUE;
  }

//InitAudio: Initialize sound library, create audio stream
//Playback rate is the initially requested one (to avoid a resampling step), or 48000 Hz if exceeding supported ones,
//Format is signed 16 bit stereo samples (for now, due to most apps using NISE)
//Nuon sound samples must be byte-swapped for use by libraries which require
//little-endian byte ordering

#define MIX_RATE (48000) // all tested games so far are actually using 32k due to NISE
#define MAX_SOFTWARE_CHANNELS (2)
#define INIT_FLAGS FSOUND_INIT_GLOBALFOCUS
#define DEFAULT_SAMPLE_FORMAT (FSOUND_16BITS | FSOUND_STEREO | FSOUND_SIGNED)

void NuonEnvironment::InitAudio()
{
  if(nuonAudioPlaybackRate == 0 || nuonAudioBufferSize == 0) // delay FMOD init until SetAudioPlaybackRate and AudioSetChannelMode has been called so that we can set requested rate and buffer size here (also avoids one resampling step then!)
    return;

  if(!bFMODInitialized)
  {
    //Select default sound driver
    FSOUND_SetDriver(0);
    FSOUND_SetMixer(FSOUND_MIXER_QUALITY_AUTODETECT);
    //Initialize FMOD
    if(FSOUND_Init(nuonAudioPlaybackRate <= 65535 ? nuonAudioPlaybackRate : MIX_RATE, MAX_SOFTWARE_CHANNELS, INIT_FLAGS)) // 65535 = max rate of FMOD
      bFMODInitialized = true;
    else
      return;
  }

  if(audioStream)
  {
    FSOUND_Stream_Stop(audioStream);
    FSOUND_Stream_Close(audioStream);
  }

  MuteAudio(false);

  //Create stream
  audioStream = FSOUND_Stream_Create(StreamCallback, (nuonAudioBufferSize>>1), // >>1: see callback, kinda double buffering in there
    DEFAULT_SAMPLE_FORMAT, nuonAudioPlaybackRate, this);
    
  if(audioStream)
    audioChannel = FSOUND_Stream_Play(FSOUND_FREE,audioStream);
}

void NuonEnvironment::CloseAudio()
{
  if(bFMODInitialized)
  {
    MuteAudio(true);
    if(audioStream)
    {
      FSOUND_Stream_Stop(audioStream);
      FSOUND_Stream_Close(audioStream);
      audioStream = 0;
    }
  }
}

void NuonEnvironment::MuteAudio(const bool mute)
{
  if(bFMODInitialized)
    FSOUND_SetMute(FSOUND_ALL,mute ? TRUE : FALSE);
}

void NuonEnvironment::SetAudioPlaybackRate()
{
  if (nuonAudioChannelMode & ENABLE_SAMP_INT)
    cyclesPerAudioInterrupt = 54000000 / nuonAudioPlaybackRate;

  if(!bFMODInitialized)
    InitAudio();

  if(bFMODInitialized && audioStream)
  {
    if(FSOUND_IsPlaying(audioChannel))
    {
      // both should never happen
      uint32 rate = nuonAudioPlaybackRate;
      if (rate > 96000)
        rate = 96000;
      else if (rate < 16000)
        rate = 16000;
        
      FSOUND_SetFrequency(audioChannel,rate);
    }
  }
}

void NuonEnvironment::RestartAudio()
{
  //Stop audio and restart it
  CloseAudio();
  InitAudio();
}

void NuonEnvironment::StopAudio()
{
  if(bFMODInitialized && audioStream)
    FSOUND_Stream_Stop(audioStream);
}

//static uint32 lastLinearVolumeSetting = 0;

void NuonEnvironment::SetAudioVolume(uint32 volume)
{
  if(bFMODInitialized)
  {
    if(volume > 255)
      volume = 255;

    FSOUND_SetVolume(FSOUND_ALL,volume);
    //lastLinearVolumeSetting = volume;
  }
}

void *NuonEnvironment::GetPointerToMemory(const MPE &_mpe, const uint32 address, const bool bCheckAddress)
{
  if(address < MAIN_BUS_BASE)
  {
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    if(bCheckAddress)
    {
      if((address < MPE_ADDR_SPACE_BASE) || (address >= MPE1_ADDR_BASE))
      {
        char textBuf[1024];
        sprintf_s(textBuf, sizeof(textBuf),"MPE%u Illegal Memory Address Operand (MAIN) %8.8X\nMPE0 pcexec: %8.8X\nMPE1 pcexec: %8.8X\nMPE2 pcexec: %8.8X\nMPE3 pcexec: %8.8X\n",
          _mpe.mpeIndex,
          address,
          mpe[0].pcexec,
          mpe[1].pcexec,
          mpe[2].pcexec,
          mpe[3].pcexec);

        MessageBox( NULL, textBuf, "GetPointerToMemory error", MB_OK);
      }
    }
#endif
    return (void *)(((uint8 *)_mpe.dtrom) + (address & MPE_VALID_MEMORY_MASK));
  }
  else if(address < SYSTEM_BUS_BASE)
  {
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    if(bCheckAddress)
    {
      if((address > (MAIN_BUS_BASE + MAIN_BUS_VALID_MEMORY_MASK)))
      {
        char textBuf[1024];
        sprintf_s(textBuf, sizeof(textBuf), "MPE%u Illegal Memory Address Operand (SYSTEM) %8.8X\nMPE0 pcexec: %8.8X\nMPE1 pcexec: %8.8X\nMPE2 pcexec: %8.8X\nMPE3 pcexec: %8.8X\n",
          _mpe.mpeIndex,
          address,
          mpe[0].pcexec,
          mpe[1].pcexec,
          mpe[2].pcexec,
          mpe[3].pcexec);

        MessageBox( NULL, textBuf, "GetPointerToMemory error", MB_OK);
      }
    }
#endif
    return &mainBusDRAM[address & MAIN_BUS_VALID_MEMORY_MASK];
  }
  else if(address < ROM_BIOS_BASE)
  {
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    if(bCheckAddress)
    {
      if((address > (SYSTEM_BUS_BASE + SYSTEM_BUS_VALID_MEMORY_MASK)))
      {
        char textBuf[1024];
        sprintf_s(textBuf,sizeof(textBuf), "MPE%u Illegal Memory Address Operand (ROM_BIOS) %8.8X\nMPE0 pcexec: %8.8X\nMPE1 pcexec: %8.8X\nMPE2 pcexec: %8.8X\nMPE3 pcexec: %8.8X\n",
          _mpe.mpeIndex,
          address,
          mpe[0].pcexec,
          mpe[1].pcexec,
          mpe[2].pcexec,
          mpe[3].pcexec);

        MessageBox( NULL, textBuf, "GetPointerToMemory error", MB_OK);
      }
    }
#endif
    return &systemBusDRAM[address & SYSTEM_BUS_VALID_MEMORY_MASK];
  }
  else
  {
    return flashEEPROM.GetBasePointer() + ((address - ROM_BIOS_BASE) & (DEFAULT_EEPROM_SIZE - 1));
  }
}

// copy of above, specialized to not need a MPE
void *NuonEnvironment::GetPointerToSystemMemory(const uint32 address, const bool bCheckAddress)
{
  if(address < MAIN_BUS_BASE)
  {
    assert(false);
    return NULL;
  }
  else if(address < SYSTEM_BUS_BASE)
  {
#ifdef ENABLE_EMULATION_MESSAGEBOXES
    if(bCheckAddress)
    {
      if((address > (MAIN_BUS_BASE + MAIN_BUS_VALID_MEMORY_MASK)))
      {
        char textBuf[1024];
        sprintf_s(textBuf,sizeof(textBuf),"Illegal Memory Address Operand (SYSTEM) %8.8X\n",address);

        MessageBox( NULL, textBuf, "GetPointerToSystemMemory error", MB_OK);
      }
    }
#endif
    return &mainBusDRAM[address & MAIN_BUS_VALID_MEMORY_MASK];
  }
  else if(address < ROM_BIOS_BASE)
  {
    assert(false);
    return NULL;
  }
  else
  {
    assert(false);
    return NULL;
  }
}


void NuonEnvironment::WriteFile(MPE &MPE, uint32 fd, uint32 buf, uint32 len)
{
  char * const pBuf = (char *)GetPointerToMemory(MPE, buf, false);
  const char tempChar = pBuf[len];
  pBuf[len] = '\0';

  switch(fd)
  {
    case NUON_FD_STDOUT:
      //if(!pStdOutEdit)
      //{
      //    return;
      //}
      //int windowTextLength = pStdOutEdit->length();
      //if((windowTextLength != 0) && ((windowTextLength + len) < 65536))
      //{
      //    pStdOutEdit->append(QString(pBuf));
      //}
      //else
      //{
        //pStdOutEdit->setText(QString(pBuf));
      //}
      break;
    case NUON_FD_STDERR:
      //if(!pStdErrEdit)
      //{
      //    return;
      //}
      //int windowTextLength = pStdErrEdit->length();
      //if((windowTextLength != 0) && ((windowTextLength + len) < 65536))
      //{
        //pStdErrEdit->append(QString(pBuf));
      //}
      //else
      //{
        //pStdErrEdit->setText(QString(pBuf));
      //}
      break;
    default:
      break;
  }

  pBuf[len] = tempChar;
}

void NuonEnvironment::Init()
{
  init_nuon_mem(mainBusDRAM, MAIN_BUS_SIZE);
  init_nuon_mem(systemBusDRAM, SYSTEM_BUS_SIZE);

  dvdBase = new char[1];
  dvdBase[0] = 0;
  bAudioInterruptsEnabled = true;

  bAutomaticLoadPopup = true;

  trigger_render_video = false;

  timer_rate[0] = -1;
  timer_rate[1] = -1;
  timer_rate[2] = -1;

  schedule_intsrc = 0;

  for(uint32 i = 0; i < 4; i++)
    mpe[i].Init(i, mainBusDRAM, systemBusDRAM, flashEEPROM.GetBasePointer());

  //mpe[0].mpeStartAddress = 0x20000000;
  //mpe[0].mpeEndAddress = 0x207FFFFF;
  //mpe[1].mpeStartAddress = 0x20800000;
  //mpe[1].mpeEndAddress = 0x20FFFFFF;
  //mpe[2].mpeStartAddress = 0x21000000;
  //mpe[2].mpeEndAddress = 0x217FFFFF;
  //mpe[3].mpeStartAddress = 0x21800000;
  //mpe[3].mpeEndAddress = 0x21FFFFFF;

  bProcessorStartStopChange = false;
  pendingCommRequests = 0;

  memset(&structMainDisplay,0,sizeof(VidDisplay));
  structMainDisplay.dispwidth = VIDEO_WIDTH;
  structMainDisplay.dispheight = VIDEO_HEIGHT;

  bInterlaced = false;
  InitializeColorSpaceTables();
  mainChannelLowerLimit = 0;
  overlayChannelLowerLimit = 0;
  mainChannelUpperLimit = 0;
  overlayChannelUpperLimit = 0;

  //TIMER RELATED INITIALIZATION: chooses the timing method used in TimeElapsed
  //and initializes the boot timestamp
  InitializeTimingMethod();

  pNuonAudioBuffer = 0;
  audio_buffer_offset = 0;
  audio_buffer_played = 0;
  oldNuonAudioChannelMode = nuonAudioChannelMode = 0;
  nuonAudioPlaybackRate = 0; //!! was 32000
  nuonAudioBufferSize = 0; //!! was 1024
  nuonSupportedPlaybackRates =
    RATE_16_KHZ |
    RATE_32_KHZ |
    RATE_64_KHZ |
    RATE_22_05_KHZ |
    RATE_44_1_KHZ |
    RATE_88_2_KHZ |
    RATE_24_KHZ |
    RATE_48_KHZ |
    RATE_96_KHZ;

  cyclesPerAudioInterrupt = 1728000;
  audioInterruptCycleCount = cyclesPerAudioInterrupt;
  videoDisplayCycleCount = 120000;
  whichAudioInterrupt = false;

  const char* tmpName = !pArgs ? "nuance.cfg" : pArgs[1];
  const size_t tmpNameLen = strlen(tmpName) + 1;

  cfgFileName = new char[tmpNameLen];
  strcpy_s(cfgFileName, tmpNameLen, tmpName);

  LoadConfigFile(cfgFileName);

  cycleCounter = 0;

  //Initialize the BIOS
  InitBios();
}

uint32 NuonEnvironment::GetBufferSize(uint32 channelMode)
{
  if (channelMode & BUFFER_SIZE_1K) {
    return 1024;
  } else if (channelMode & BUFFER_SIZE_2K) {
    return 2048;
  } else if (channelMode & BUFFER_SIZE_4K) {
    return 4096;
  } else if (channelMode & BUFFER_SIZE_8K) {
    return 8192;
  } else if (channelMode & BUFFER_SIZE_16K) {
    return 16384;
  } else if (channelMode & BUFFER_SIZE_32K) {
    return 32768;
  } else if (channelMode & BUFFER_SIZE_64K) {
    return 65536;
  }
  return 0;
}

void NuonEnvironment::InitBios()
{
  ::InitBios(mpe[3]);
}

NuonEnvironment::~NuonEnvironment()
{
  //Stop the processor thread
  for(uint32 i = 0; i < 4; i++)
    mpe[i].Halt();

  //Close stream and shut down audio library
  CloseAudio();
  FSOUND_Close();
  bFMODInitialized = false;

  //Free up string memory
  delete [] dvdBase;
  delete [] cfgFileName;

  DeInitTimingMethod();
}

constexpr char CONFIG_COMMENT_CHAR = ';';
constexpr char CONFIG_VARIABLE_START_CHAR = '[';
constexpr char CONFIG_VARIABLE_FINISH_CHAR = ']';

void ReplaceNewline(char * const line, const char replaceChar, const uint32 maxIndex)
{
  uint32 index = 0;

  while(index <= maxIndex)
  {
    if(line[index] == '\n')
      break;

    index++;
  }

  line[index] = replaceChar;
}

void NuonEnvironment::SetDVDBaseFromFileName(const char * const filename)
{
  delete [] dvdBase;
  int i = (int)strlen(filename);
  dvdBase = new char[i+1];
  strcpy_s(dvdBase,i+1,filename);
  while(i >= 0)
  {
     if(dvdBase[i] == '\\')
       break;
   
     dvdBase[i] = '\0';
     i--;
  }

  if(dvdBase[0] == '\0')
  {
    dvdBase[0] = '.';
    dvdBase[1] = '/';
    dvdBase[2] = '\0';
  }
}


static void TrimWhitespace(char *buf, size_t bufLength)
{
  const char* firstReal = buf;
  const char* lastReal = buf + strlen(buf);
  while (*firstReal && isspace(*firstReal)) firstReal++;
  while ((lastReal > firstReal) && isspace(*(lastReal - 1))) lastReal--;
  if (firstReal != buf)
  {
    memmove_s(buf, bufLength, firstReal, (lastReal + 1) - firstReal);
  }
  buf[lastReal - firstReal] = '\0';
}

bool NuonEnvironment::StrToCtrlrBitnum(const char* str, unsigned int* bitnum)
{
#define CHECK_STR(s0, s1, i) \
do { \
  if (!_stricmp((s0), (s1))) \
  { \
    *bitnum = (i); \
    return true; \
  } \
} while (0)

  if (!_strnicmp(str, "CPAD_", sizeof("CPAD_") - 1))
  {
    const char* dir = str + sizeof("CPAD_") - 1;
    CHECK_STR(dir, "UP", CTRLR_BITNUM_BUTTON_C_UP);
    CHECK_STR(dir, "DOWN", CTRLR_BITNUM_BUTTON_C_DOWN);
    CHECK_STR(dir, "LEFT", CTRLR_BITNUM_BUTTON_C_LEFT);
    CHECK_STR(dir, "RIGHT", CTRLR_BITNUM_BUTTON_C_RIGHT);
  }
  else if (!_strnicmp(str, "DPAD_", sizeof("DPAD_") - 1))
  {
    const char* dir = str + sizeof("DPAD_") - 1;
    CHECK_STR(dir, "UP", CTRLR_BITNUM_DPAD_UP);
    CHECK_STR(dir, "DOWN", CTRLR_BITNUM_DPAD_DOWN);
    CHECK_STR(dir, "LEFT", CTRLR_BITNUM_DPAD_LEFT);
    CHECK_STR(dir, "RIGHT", CTRLR_BITNUM_DPAD_RIGHT);
  }
  else
  {
    CHECK_STR(str, "A", CTRLR_BITNUM_BUTTON_A);
    CHECK_STR(str, "B", CTRLR_BITNUM_BUTTON_B);
    CHECK_STR(str, "L", CTRLR_BITNUM_BUTTON_L);
    CHECK_STR(str, "R", CTRLR_BITNUM_BUTTON_R);
    CHECK_STR(str, "NUON", CTRLR_BITNUM_BUTTON_NUON);
    CHECK_STR(str, "START", CTRLR_BITNUM_BUTTON_START);
  }

  return false;
}

// Format is <CPAD_[UP,DOWN,LEFT,RIGHT]>|<DPAD_[UP,DOWN,LEFT,RIGHT]>|<A,B,L,R,NUON,START> = <JOYBUT,JOYAXIS,JOYPOV>_<idx>_<subIdx>
bool NuonEnvironment::ParseJoyButtonConf(char buf[1025], unsigned int *bitnum, ControllerButtonMapping *mapping)
{
  char tmpBuf[1025];

  // strtok is destructive. Don't harm the original string.
  strcpy_s(tmpBuf, buf);

  // Split on the equals sign.
  char *ctx = NULL;
  char *nuonName = strtok_s(tmpBuf, "=", &ctx);

  if (!nuonName) return false;

  char* mappingName = strtok_s(NULL, "=", &ctx);

  if (!mappingName) return false;

  // Trim any whitespace
  TrimWhitespace(nuonName, strlen(nuonName) + 1);
  TrimWhitespace(mappingName, strlen(mappingName) + 1);

  if (!StrToCtrlrBitnum(nuonName, bitnum)) return false;

  return ControllerButtonMapping::fromString(mappingName, mapping);
}

ConfigTokenType NuonEnvironment::ReadConfigLine(FILE *file, char buf[1025])
{
  while (true)
  {
    if (feof(file))
      return ConfigTokenType::CONFIG_EOF;

    // Get a whitespace-trimmed line from the file. Note this logic is broken
    // for lines longer than 1025 characters, but that's way better than the
    // previous logic, which was broken for lines containing spaces.
    fgets(buf, 1025, file);
    TrimWhitespace(buf, 1025);

    const char firstChar = buf[0];

    if (firstChar == '\0')
      continue; // Skip empty lines
    else if (firstChar == CONFIG_COMMENT_CHAR)
      return ConfigTokenType::CONFIG_COMMENT;
    else if (firstChar == CONFIG_VARIABLE_START_CHAR)
      return ConfigTokenType::CONFIG_VARIABLE_START;
    else if (firstChar == CONFIG_VARIABLE_FINISH_CHAR)
      return ConfigTokenType::CONFIG_VARIABLE_FINISH;
    else if ((firstChar == '<') || (firstChar == '>') || (firstChar == '"'))
      return ConfigTokenType::CONFIG_RESERVED;
    else
      return ConfigTokenType::CONFIG_STRING;
  }
}

bool NuonEnvironment::SaveConfigFile(const char* const fileName)
{
  FILE* configFile;

  if (fopen_s(&configFile, fileName ? fileName : cfgFileName, "w") != 0)
    return false;

  // Don't save DVDBase. AFAICT, it is always overriden when loading a file, so no point in saving/loading it.
  //fprintf_s(configFile, "[DVDBase]\n");
  //fprintf_s(configFile, "%s\n\n", dvdBase);

  fprintf_s(configFile, "[AudioInterrupts]\n");
  fprintf_s(configFile, "%s\n\n", bAudioInterruptsEnabled ? "Enabled" : "Disabled");

  fprintf_s(configFile, "[DynamicCompiler]\n");
  fprintf_s(configFile, "%s\n\n", compilerOptions.bAllowCompile ? "Enabled" : "Disabled");

#ifdef ENABLE_EMULATION_MESSAGEBOXES
  fprintf_s(configFile, "[DumpCompiledBlocks]\n");
  fprintf_s(configFile, "%s\n\n", compilerOptions.bDumpBlocks ? "Enabled" : "Disabled");
#endif

  fprintf_s(configFile, "[CompilerDeadCodeElimination]\n");
  fprintf_s(configFile, "%s\n\n", compilerOptions.bDeadCodeElimination ? "Enabled" : "Disabled");

  fprintf_s(configFile, "[CompilerConstantPropagation]\n");
  fprintf_s(configFile, "%s\n\n", compilerOptions.bConstantPropagation ? "Enabled" : "Disabled");

  fprintf_s(configFile, "[T3KCompilerHack]\n");
  fprintf_s(configFile, "%s\n\n", compilerOptions.bT3KCompilerHack ? "Enabled" : "Disabled");

  fprintf_s(configFile, "[AutomaticLoadPopup]\n");
  fprintf_s(configFile, "%s\n\n", bAutomaticLoadPopup ? "Enabled" : "Disabled");

  fprintf_s(configFile, "[Controller1Mappings]\n");
  for (size_t i = 0; i < _countof(controller1Mapping); i++)
  {
    const char* ctrlrStr;

    switch (i)
    {
    case CTRLR_BITNUM_BUTTON_C_UP:
      ctrlrStr = "CPAD_UP";
      break;
    case CTRLR_BITNUM_BUTTON_C_RIGHT:
      ctrlrStr = "CPAD_RIGHT";
      break;
    case CTRLR_BITNUM_BUTTON_C_DOWN:
      ctrlrStr = "CPAD_DOWN";
      break;
    case CTRLR_BITNUM_BUTTON_C_LEFT:
      ctrlrStr = "CPAD_LEFT";
      break;
    case CTRLR_BITNUM_DPAD_UP:
      ctrlrStr = "DPAD_UP";
      break;
    case CTRLR_BITNUM_DPAD_RIGHT:
      ctrlrStr = "DPAD_RIGHT";
      break;
    case CTRLR_BITNUM_DPAD_DOWN:
      ctrlrStr = "DPAD_DOWN";
      break;
    case CTRLR_BITNUM_DPAD_LEFT:
      ctrlrStr = "DPAD_LEFT";
      break;
    case CTRLR_BITNUM_BUTTON_A:
      ctrlrStr = "A";
      break;
    case CTRLR_BITNUM_BUTTON_B:
      ctrlrStr = "B";
      break;
    case CTRLR_BITNUM_BUTTON_L:
      ctrlrStr = "L";
      break;
    case CTRLR_BITNUM_BUTTON_R:
      ctrlrStr = "R";
      break;
    case CTRLR_BITNUM_BUTTON_NUON:
      ctrlrStr = "NUON";
      break;
    case CTRLR_BITNUM_BUTTON_START:
      ctrlrStr = "START";
      break;
    case CTRLR_BITNUM_UNUSED_1:
      // Fall through
    case CTRLR_BITNUM_UNUSED_2:
      //Fall through
    default:
      continue;
    }

    char mappingStr[ControllerButtonMapping::MAPPING_STRING_SIZE];
    controller1Mapping[i].toString(mappingStr, _countof(mappingStr));
    fprintf_s(configFile, "%s = %s\n", ctrlrStr, mappingStr);
  }
  fprintf_s(configFile, "\n");

  fprintf_s(configFile, "[Controller1GUID]\n");

  LPOLESTR guidWStr;
  char guidStr[100];
  StringFromCLSID(controller1Di8Dev, &guidWStr);
  size_t convSize;
  wcstombs_s(&convSize, guidStr, _countof(guidStr), guidWStr, _countof(guidStr) - 1);
  CoTaskMemFree(guidWStr);

  fprintf_s(configFile, "%s\n", guidStr);

  fclose(configFile);
  return true;
}

bool NuonEnvironment::LoadConfigFile(const char * const fileName)
{
  FILE* configFile;
  if (fopen_s(&configFile, fileName, "r") != 0 )
    return false;


  bool useExistingLine = false;
  ConfigTokenType tokenType;

  while(!feof(configFile))
  {
    char line[1025];
    if (!useExistingLine)
    {
      tokenType = ReadConfigLine(configFile, line);
    }
    else
    {
      useExistingLine = false;
    }

    switch(tokenType)
    {
      case ConfigTokenType::CONFIG_COMMENT:
        break;
      case ConfigTokenType::CONFIG_VARIABLE_START:
        if(_strnicmp(&line[1],"DVDBase]",sizeof("DVDBase]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          ReplaceNewline(line,0,1024);
          delete [] dvdBase;
          size_t i = strlen(line);
          dvdBase = new char[i+1];
          strcpy_s(dvdBase,i+1,line);
        }
        else if(_strnicmp(&line[1],"AudioInterrupts]",sizeof("AudioInterrupts]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          bAudioInterruptsEnabled = !_stricmp(line,"Enabled");
        }
        else if(_strnicmp(&line[1],"DynamicCompiler]",sizeof("DynamicCompiler]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          compilerOptions.bAllowCompile = !_stricmp(line,"Enabled");
        }
#ifdef ENABLE_EMULATION_MESSAGEBOXES
        else if(_strnicmp(&line[1],"DumpCompiledBlocks]",sizeof("DumpCompiledBlocks]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          compilerOptions.bDumpBlocks = !_stricmp(line,"Enabled");
        }
#endif
        else if(_strnicmp(&line[1],"CompilerDeadCodeElimination]",sizeof("CompilerDeadCodeElimination]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          compilerOptions.bDeadCodeElimination = !_stricmp(line,"Enabled");
        }
        else if(_strnicmp(&line[1],"CompilerConstantPropagation]",sizeof("CompilerConstantPropagation]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          compilerOptions.bConstantPropagation = !_stricmp(line,"Enabled");
        }
        else if(_strnicmp(&line[1],"T3KCompilerHack]",sizeof("T3KCompilerHack]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          compilerOptions.bT3KCompilerHack = !_stricmp(line,"Enabled");
        }
        else if(_strnicmp(&line[1],"AutomaticLoadPopup]",sizeof("AutomaticLoadPopup]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          bAutomaticLoadPopup = !_stricmp(line,"Enabled");
        }
        else if(_strnicmp(&line[1],"Controller1Mappings]",sizeof("Controller1Mappings]")) == 0)
        {
          while (true)
          {
            ControllerButtonMapping mapping;
            unsigned int bitnum;
            tokenType = ReadConfigLine(configFile, line);
            if (tokenType == ConfigTokenType::CONFIG_COMMENT) continue;

            if ((tokenType == ConfigTokenType::CONFIG_STRING) &&
              ParseJoyButtonConf(line, &bitnum, &mapping))
            {
              controller1Mapping[bitnum] = mapping;
            }
            else
            {
              useExistingLine = true;
              break;
            }
          }
        }
        else if (_strnicmp(&line[1], "Controller1GUID]", sizeof("Controller1GUID]")) == 0)
        {
          wchar_t guidStr[100];
          tokenType = ReadConfigLine(configFile, line);
          size_t convLen;
          mbstowcs_s(&convLen, guidStr, line, _countof(guidStr) - 1);
          CLSIDFromString(guidStr, &controller1Di8Dev);
        }

#ifdef _WIN64
        compilerOptions.bAllowCompile = false; //!!
#endif
        break;
      default:
        break;
    }
  }

  fclose(configFile);
  return true;
}

void NuonEnvironment::SetController1Joystick(const GUID& guid)
{
  controller1Di8Dev = guid;
}

void NuonEnvironment::SetControllerButtonMapping(unsigned int ctrlrBitnum, const ControllerButtonMapping& mapping)
{
  if (ctrlrBitnum < 16)
  {
    controller1Mapping[ctrlrBitnum] = mapping;
  }
}
const GUID& NuonEnvironment::GetController1Joystick() const
{
  return controller1Di8Dev;
}

int NuonEnvironment::GetCTRLRBitnumFromMapping(const ControllerButtonMapping& mapping) const
{
  InputManager* im = display.GetInputManager();
  size_t numJoysticks;
  im->EnumJoysticks(&numJoysticks);

  for (int i = 0; i < (int)_countof(controller1Mapping); i++)
  {
    const ControllerButtonMapping* cbm = (numJoysticks == 0 && controller1Mapping[i].type != InputManager::KEY) ? controllerDefaultMapping : controller1Mapping; // if no joypad/joystick connected, but button is configured, fall back to default key mapping
    if (cbm[i] == mapping) return i;
  }

  return -1;
}

const ControllerButtonMapping& NuonEnvironment::GetMappingForCTRLRBitnum(unsigned int bitnum) const
{
  static const ControllerButtonMapping invalidMapping(InputManager::JOYBUT, INT_MAX, INT_MAX);

  if (bitnum >= _countof(controller1Mapping)) return invalidMapping;

  return controller1Mapping[bitnum];
}

bool ControllerButtonMapping::fromString(char* strIn, ControllerButtonMapping* mapping)
{
  // Split the button mapping into three parts on '_'
  char* ctx = NULL;
  const char* type = strtok_s(strIn, "_", &ctx);

  if (!type) return false;

  const char* idxStr = strtok_s(NULL, "_", &ctx);

  if (!idxStr) return false;

  const char* subIdxStr = strtok_s(NULL, "_", &ctx);

  if (!subIdxStr) return false;

  // Convert each part to its appropriate mapping field
  if (!InputManager::StrToInputType(type, &mapping->type)) return false;

  char* endConv;
  errno = 0;
  mapping->idx = strtoul(idxStr, &endConv, 10);
  if ((errno != 0) || (*endConv != '\0')) return false;
  mapping->subIdx = strtoul(subIdxStr, &endConv, 10);
  if ((errno != 0) || (*endConv != '\0')) return false;

  return true;
}