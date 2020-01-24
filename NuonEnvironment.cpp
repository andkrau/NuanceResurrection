#include <assert.h>
#include <windows.h>
#include <stdio.h>
#include "audio.h"
#include "external\fmod-3.75\api\inc\fmod.h"
#include "basetypes.h"
#include "Bios.h"
#include "file.h"
#include "memory.h"
#include "mpe.h"
#include "timer.h"
#include "video.h"
#include "GLWindow.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "Syscall.h"

extern VidDisplay structMainDisplay;

extern NuonEnvironment nuonEnv;
extern char **pArgs;

static bool bFMODInitialized = false;
static FSOUND_STREAM *audioStream;
static int audioChannel = 0;

void NuonEnvironment::TriggerAudioInterrupt(void)
{
  if(bAudioInterruptsEnabled)
  {
    mpe[0].TriggerInterrupt(INT_AUDIO);
    mpe[1].TriggerInterrupt(INT_AUDIO);
    mpe[2].TriggerInterrupt(INT_AUDIO);
    mpe[3].TriggerInterrupt(INT_AUDIO);
  }
}

void NuonEnvironment::TriggerVideoInterrupt(void)
{
  mpe[0].TriggerInterrupt(INT_VIDTIMER);
  mpe[1].TriggerInterrupt(INT_VIDTIMER);
  mpe[2].TriggerInterrupt(INT_VIDTIMER);
  mpe[3].TriggerInterrupt(INT_VIDTIMER);
}

class AudioCallbacks
{
public:

  void *pBuf1WriteStart;
  unsigned long pBuf1WriteBytes;
  void *pBuf2WriteStart;
  unsigned long pBuf2WriteBytes;

  static void ConvertNuonAudioData(const uint8 * const pNuonAudioBuffer, uint8 * const pPCAudioBuffer, const uint32 numBytes)
  {
    assert((numBytes % 4) == 0);

    uint32 byteCount = 0;
    while(byteCount < numBytes)
    {
      pPCAudioBuffer[byteCount  ] = pNuonAudioBuffer[byteCount+1];
      pPCAudioBuffer[byteCount+1] = pNuonAudioBuffer[byteCount  ];
      pPCAudioBuffer[byteCount+2] = pNuonAudioBuffer[byteCount+3];
      pPCAudioBuffer[byteCount+3] = pNuonAudioBuffer[byteCount+2];
      byteCount += 4;
    }
  }

  static schar F_CALLBACKAPI StreamCallback(FSOUND_STREAM *stream, void *buff, int len, void* userdata)
  {
    static bool position = false;

    if(!buff)
      return FALSE;

    assert(len == nuonEnv.nuonAudioBufferSize>>1);

    if (nuonEnv.pNuonAudioBuffer && (nuonEnv.nuonAudioBufferSize >= 1024))
    {
    if(!position)
    {
      ConvertNuonAudioData(&nuonEnv.pNuonAudioBuffer[nuonEnv.nuonAudioBufferSize>>1], (uint8 *)buff, nuonEnv.nuonAudioBufferSize>>1); // kinda double buffering in here
      if((nuonEnv.nuonAudioChannelMode & ENABLE_WRAP_INT) && !nuonEnv.bUseCycleBasedTiming)
        nuonEnv.TriggerAudioInterrupt();
    }
    else
    {
      ConvertNuonAudioData(nuonEnv.pNuonAudioBuffer, (uint8 *)buff, nuonEnv.nuonAudioBufferSize>>1); // kinda double buffering in here
      if((nuonEnv.nuonAudioChannelMode & ENABLE_HALF_INT) && !nuonEnv.bUseCycleBasedTiming)
        nuonEnv.TriggerAudioInterrupt();
    }
    position = !position;
    return TRUE;
    }

    return FALSE;
  }
};

AudioCallbacks audioCallbacks;

//InitAudio: Initialize sound library, create audio stream
//Playback rate is 44100 Hz, Format is signed 16 bit stereo samples
//Nuon sound samples must be byte-swapped for use by libraries which require
//little-endian byte ordering

#define MIX_RATE (44100)
#define MAX_SOFTWARE_CHANNELS (6)
#define INIT_FLAGS FSOUND_INIT_GLOBALFOCUS
#define DEFAULT_SAMPLE_FORMAT (FSOUND_16BITS | FSOUND_STEREO | FSOUND_SIGNED)
#define USER_PARAM (0)

void NuonEnvironment::InitAudio(void)
{
  if(!bFMODInitialized)
  {
    //Select default sound driver
    FSOUND_SetDriver(0);
    //FSOUND_SetMixer(FSOUND_MIXER_QUALITY_FPU);
    //FSOUND_SetMixer(FSOUND_MIXER_QUALITY_MMXP5);
    //FSOUND_SetMixer(FSOUND_MIXER_QUALITY_MMXP6);
    //Initialize FMOD
    if(FSOUND_Init(MIX_RATE, MAX_SOFTWARE_CHANNELS, INIT_FLAGS))
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
  audioStream = FSOUND_Stream_Create(AudioCallbacks::StreamCallback, nuonAudioBufferSize>>1, // >>1: see callback, kinda double buffering in there
    (DEFAULT_SAMPLE_FORMAT | FSOUND_LOOP_NORMAL | FSOUND_NONBLOCKING), nuonAudioPlaybackRate, USER_PARAM);
    
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

void NuonEnvironment::SetAudioPlaybackRate(uint32 rate)
{
  if(bFMODInitialized && audioStream)
  {
    if(FSOUND_IsPlaying(audioChannel))
    {
        // both should never happen
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

void *NuonEnvironment::GetPointerToMemory(const MPE &mpe, const uint32 address, const bool bCheckAddress)
{
  if(address < MAIN_BUS_BASE)
  {
#if 1//def _DEBUG
    if(bCheckAddress)
    {
      if((address < MPE_ADDR_SPACE_BASE) || (address >= MPE1_ADDR_BASE))
      {
        char textBuf[1024];
        sprintf(textBuf,"MPE%d Illegal Memory Address Operand (MAIN) %8.8X\nMPE0 pcexec: %8.8X\nMPE1 pcexec: %8.8X\nMPE2 pcexec: %8.8X\nMPE3 pcexec: %8.8X\n",
          mpe.mpeIndex,
          address,
          this->mpe[0].pcexec,
          this->mpe[1].pcexec,
          this->mpe[2].pcexec,
          this->mpe[3].pcexec);

        MessageBox( NULL, textBuf, "GetPointerToMemory error", MB_OK);
      }
    }
#endif
    return (void *)(((uint8 *)mpe.dtrom) + (address & MPE_VALID_MEMORY_MASK));
  }
  else if(address < SYSTEM_BUS_BASE)
  {
#if 1//def _DEBUG
    if(bCheckAddress)
    {
      if((address > (MAIN_BUS_BASE + MAIN_BUS_VALID_MEMORY_MASK)))
      {
        char textBuf[1024];
        sprintf(textBuf,"MPE%d Illegal Memory Address Operand (SYSTEM) %8.8X\nMPE0 pcexec: %8.8X\nMPE1 pcexec: %8.8X\nMPE2 pcexec: %8.8X\nMPE3 pcexec: %8.8X\n",
          mpe.mpeIndex,
          address,
          this->mpe[0].pcexec,
          this->mpe[1].pcexec,
          this->mpe[2].pcexec,
          this->mpe[3].pcexec);

        MessageBox( NULL, textBuf, "GetPointerToMemory error", MB_OK);
      }
    }
#endif
    return &mainBusDRAM[address & MAIN_BUS_VALID_MEMORY_MASK];
  }
  else if(address < ROM_BIOS_BASE)
  {
#if 1//def _DEBUG
    if(bCheckAddress)
    {
      if((address > (SYSTEM_BUS_BASE + SYSTEM_BUS_VALID_MEMORY_MASK)))
      {
        char textBuf[1024];
        sprintf(textBuf,"MPE%d Illegal Memory Address Operand (ROM_BIOS) %8.8X\nMPE0 pcexec: %8.8X\nMPE1 pcexec: %8.8X\nMPE2 pcexec: %8.8X\nMPE3 pcexec: %8.8X\n",
          mpe.mpeIndex,
          address,
          this->mpe[0].pcexec,
          this->mpe[1].pcexec,
          this->mpe[2].pcexec,
          this->mpe[3].pcexec);

        MessageBox( NULL, textBuf, "GetPointerToMemory error", MB_OK);
      }
    }
#endif
    return &systemBusDRAM[address & SYSTEM_BUS_VALID_MEMORY_MASK];
  }
  else
  {
    return flashEEPROM->GetBasePointer() + ((address - ROM_BIOS_BASE) & (DEFAULT_EEPROM_SIZE - 1));
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
#if 1//def _DEBUG
    if(bCheckAddress)
    {
      if((address > (MAIN_BUS_BASE + MAIN_BUS_VALID_MEMORY_MASK)))
      {
        char textBuf[1024];
        sprintf(textBuf,"Illegal Memory Address Operand (SYSTEM) %8.8X\n",address);

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
  char tempChar = pBuf[len];
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

NuonEnvironment::NuonEnvironment()
{
  mainBusDRAM = new uint8[MAIN_BUS_SIZE];
  systemBusDRAM = new uint8[SYSTEM_BUS_SIZE];
  flashEEPROM = new FlashEEPROM;

  dvdBase = new char[1];
  dvdBase[0] = 0;
  bAudioInterruptsEnabled = true;

  for(uint32 i = 0; i < 4; i++)
    mpe[i].Init(i, mainBusDRAM, systemBusDRAM, flashEEPROM->GetBasePointer());

  mpe[0].mpeStartAddress = 0x20000000;
  mpe[0].mpeEndAddress = 0x207FFFFF;
  mpe[1].mpeStartAddress = 0x20800000;
  mpe[1].mpeEndAddress = 0x20FFFFFF;
  mpe[2].mpeStartAddress = 0x21000000;
  mpe[2].mpeEndAddress = 0x217FFFFF;
  mpe[3].mpeStartAddress = 0x21800000;
  mpe[3].mpeEndAddress = 0x21FFFFFF;

  bProcessorStartStopChange = false;
  pendingCommRequests = 0;

  structMainDisplay.dispwidth = VIDEO_WIDTH;
  structMainDisplay.dispheight = 480;
  bInterlaced = false;
  fps = 40;
  InitializeColorSpaceTables();
  mainChannelLowerLimit = 0;
  overlayChannelLowerLimit = 0;
  mainChannelUpperLimit = 0;
  overlayChannelUpperLimit = 0;
  bMainBufferModified = true;
  bOverlayBufferModified = true;

  //TIMER RELATED INITIALIZATION: chooses the timing method used in TimeElapsed
  //and initializes the boot timestamp
  InitializeTimingMethod();

  pNuonAudioBuffer = 0;
  nuonAudioChannelMode = 0;
  nuonAudioPlaybackRate = 32000;
  nuonAudioBufferSize = 1024;
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

  bUseCycleBasedTiming = false;
  cyclesPerAudioInterrupt = 1728000;
  audioInterruptCycleCount = cyclesPerAudioInterrupt;
  videoDisplayCycleCount = 120000;
  whichAudioInterrupt = false;

  if(!pArgs)
  {
    LoadConfigFile("nuance.cfg");
  }
  else
  {
    LoadConfigFile(pArgs[1]);
  }

  cycleCounter = 0;
}

uint32 NuonEnvironment::GetBufferSize(uint32 channelMode)
{
  uint32 bufferSize;

  if((channelMode & BUFFER_SIZE_64K) == 0)
    bufferSize = 8192;
  else
    bufferSize = 512UL << (((channelMode & BUFFER_SIZE_64K) >> 5) & 0x7UL);

  return bufferSize;
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

  //Free up allocated RAM 
  delete [] mainBusDRAM;
  mainBusDRAM = 0;
  delete [] systemBusDRAM;
  systemBusDRAM = 0;

  delete flashEEPROM;

  //Free up string memory
  delete [] dvdBase;
}

char line[1024];
const char CONFIG_COMMENT_CHAR = ';';
const char CONFIG_VARIABLE_START_CHAR = '[';
const char CONFIG_VARIABLE_FINISH_CHAR = ']';

void ReplaceNewline(char *line, char replaceChar, uint32 maxIndex)
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
  int i = strlen(filename);
  dvdBase = new char[i+1];
  strncpy(dvdBase,filename,i+1);
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

ConfigTokenType NuonEnvironment::ReadConfigLine(FILE *file, char *buf)
{
  char firstChar;

  if(feof(file))
    return CONFIG_EOF;

  fscanf(file,"%s",buf);
  firstChar = buf[0];

  if(firstChar == CONFIG_COMMENT_CHAR)
    return CONFIG_COMMENT;
  else if(firstChar == CONFIG_VARIABLE_START_CHAR)
    return CONFIG_VARIABLE_START;
  else if(firstChar == CONFIG_VARIABLE_FINISH_CHAR)
    return CONFIG_VARIABLE_FINISH;
  else if((firstChar == '<') || (firstChar == '>') || (firstChar == '"'))
    return CONFIG_RESERVED;
  else
    return CONFIG_STRING;
}

char *dvdWriteBase;
char *dvdReadBase;
char *dvdReadWriteBase;

bool NuonEnvironment::LoadConfigFile(const char * const fileName)
{
  char line[1025];
  FILE *configFile = 0;
  ConfigTokenType tokenType;

  configFile = fopen(fileName,"r");
  if(!configFile)
    return false;

  while(!feof(configFile))
  {
    tokenType = ReadConfigLine(configFile,line);
    switch(tokenType)
    {
      case CONFIG_COMMENT_CHAR:
        break;
      case CONFIG_VARIABLE_START:

        if(_strnicmp(&line[1],"DVDBase]",sizeof("DVDBase]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          ReplaceNewline(line,0,1024);
          delete [] dvdBase;
          dvdBase = new char[strlen(line)+1];
          strcpy(dvdBase,line);
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
        else if(_strnicmp(&line[1],"DumpCompiledBlocks]",sizeof("DumpCompiledBlocks]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          compilerOptions.bDumpBlocks = !_stricmp(line,"Enabled");
        }
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
        else if(_strnicmp(&line[1],"AlwaysUpdateVideo]",sizeof("AlwaysUpdateVideo]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          videoOptions.bAlwaysUpdateVideo = !_stricmp(line,"Enabled");
        }
        else if(_strnicmp(&line[1],"CycleBasedTiming]",sizeof("CycleBasedTiming]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          bUseCycleBasedTiming = !_stricmp(line,"Enabled");
        }
        else if(_strnicmp(&line[1],"FieldsPerSecond]",sizeof("FieldsPerSecond]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          sscanf(line,"%lu",&fps);
          if(fps < 1)
            fps = 1;
          else if(fps > 240)
            fps = 240;
        }

        break;
      default:
        break;
    }
  }

  fclose(configFile);
  return true;
}
