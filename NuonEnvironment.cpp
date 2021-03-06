#include "basetypes.h"
#ifdef ENABLE_EMULATION_MESSAGEBOXES
#include <windows.h>
#endif

#include "audio.h"
#include "Bios.h"
#include "file.h"
#include "mpe.h"
#include "timer.h"
#include "video.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"

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

  LoadConfigFile(!pArgs ? "nuance.cfg" : pArgs[1]);

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

ConfigTokenType NuonEnvironment::ReadConfigLine(FILE *file, char buf[1025])
{
  if(feof(file))
    return ConfigTokenType::CONFIG_EOF;

  fscanf_s(file,"%s",buf,1025);
  const char firstChar = buf[0];

  if(firstChar == CONFIG_COMMENT_CHAR)
    return ConfigTokenType::CONFIG_COMMENT;
  else if(firstChar == CONFIG_VARIABLE_START_CHAR)
    return ConfigTokenType::CONFIG_VARIABLE_START;
  else if(firstChar == CONFIG_VARIABLE_FINISH_CHAR)
    return ConfigTokenType::CONFIG_VARIABLE_FINISH;
  else if((firstChar == '<') || (firstChar == '>') || (firstChar == '"'))
    return ConfigTokenType::CONFIG_RESERVED;
  else
    return ConfigTokenType::CONFIG_STRING;
}

bool NuonEnvironment::LoadConfigFile(const char * const fileName)
{
    FILE* configFile;
    if (fopen_s(&configFile, fileName, "r") != 0 )
        return false;

  while(!feof(configFile))
  {
    char line[1025];
    ConfigTokenType tokenType = ReadConfigLine(configFile,line);
    switch(tokenType)
    {
      case CONFIG_COMMENT_CHAR:
        break;
      case ConfigTokenType::CONFIG_VARIABLE_START:
        if(_strnicmp(&line[1],"DVDBase]",sizeof("DVDBase]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          ReplaceNewline(line,0,1024);
          delete [] dvdBase;
          int i = (int)strlen(line);
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
