//---------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include "audio.h"
#include "external\fmod-3.75\api\inc\fmod.h"
#include "basetypes.h"
#include "bios.h"
#include "file.h"
#include "memory.h"
#include "mpe.h"
#include "Timer.h"
#include "video.h"
#include "GLWindow.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "Syscall.h"

extern VidDisplay structMainDisplay;
extern bool bSingleStepThreadExecuting;
extern bool bProcessorThreadExecuting;
extern GLWindow videoDisplayWindow;

extern NuonEnvironment *nuonEnv;
extern char **pArgs;

bool bFMODInitialized = false;
FSOUND_STREAM *audioStream;
int audioChannel = 0;

void NuonEnvironment::TriggerAudioInterrupt(void)
{
  if(bAudioInterruptsEnabled)
  {
    mpe[0]->TriggerInterrupt(INT_AUDIO);
    mpe[1]->TriggerInterrupt(INT_AUDIO);
    mpe[2]->TriggerInterrupt(INT_AUDIO);
    mpe[3]->TriggerInterrupt(INT_AUDIO);
  }
}

void NuonEnvironment::TriggerVideoInterrupt(void)
{
  mpe[0]->TriggerInterrupt(INT_VIDTIMER);
  mpe[1]->TriggerInterrupt(INT_VIDTIMER);
  mpe[2]->TriggerInterrupt(INT_VIDTIMER);
  mpe[3]->TriggerInterrupt(INT_VIDTIMER);
}

class AudioCallbacks
{
public:

  void *pBuf1WriteStart;
  unsigned long pBuf1WriteBytes;
  void *pBuf2WriteStart;
  unsigned long pBuf2WriteBytes;

  static void ConvertNuonAudioData(uint8 *pNuonAudioBuffer, uint8 *pPCAudioBuffer, uint32 numBytes)
  {
    uint32 byteCount = 0;

    while(byteCount < numBytes)
    {
      pPCAudioBuffer[0] = pNuonAudioBuffer[1];
      pPCAudioBuffer[1] = pNuonAudioBuffer[0];
      pPCAudioBuffer[2] = pNuonAudioBuffer[3];
      pPCAudioBuffer[3] = pNuonAudioBuffer[2];
      byteCount += 4;
      pNuonAudioBuffer += 4;
      pPCAudioBuffer += 4;
    }
  }

  static schar F_CALLBACKAPI StreamCallback(FSOUND_STREAM *stream, void *buff, int len, void* userdata)
  {
    static uint32 position = 0;

    if(!buff) 
    {
      return FALSE;
    }

    switch(position)
    {
      case 0:
        if(nuonEnv->pNuonAudioBuffer && (nuonEnv->nuonAudioBufferSize >= 1024))
        {
          ConvertNuonAudioData((uint8 *)&nuonEnv->pNuonAudioBuffer[nuonEnv->nuonAudioBufferSize >> 1], (uint8 *)buff, nuonEnv->nuonAudioBufferSize >> 1);
          if((nuonEnv->nuonAudioChannelMode & ENABLE_WRAP_INT) && !nuonEnv->bUseCycleBasedTiming)
          {
            nuonEnv->TriggerAudioInterrupt();
          }
        }
        position = 1 - position;
        break;
      case 1:
        if(nuonEnv->pNuonAudioBuffer && (nuonEnv->nuonAudioBufferSize >= 1024))
        {
          ConvertNuonAudioData((uint8 *)nuonEnv->pNuonAudioBuffer,(uint8 *)buff,nuonEnv->nuonAudioBufferSize >> 1);
          if((nuonEnv->nuonAudioChannelMode & ENABLE_HALF_INT) && !nuonEnv->bUseCycleBasedTiming)
          {
            nuonEnv->TriggerAudioInterrupt();
          }
        }
        position = 1 - position;
        break;
    }

    return TRUE;
  };
};

AudioCallbacks audioCallbacks;

//InitAudio: Initialize sound library, create audio stream
//Playback rate is 22050 Hz, Format is signed 16 bit stereo samples
//Nuon sound samples must be byte-swapped for use by libraries which require
//little-endian byte ordering

#define MIX_RATE (44100)
#define MAX_SOFTWARE_CHANNELS (6)
#define INIT_FLAGS FSOUND_INIT_GLOBALFOCUS
#define DEFAULT_SAMPLE_FORMAT (FSOUND_16BITS | FSOUND_STEREO | FSOUND_SIGNED)
#define USER_PARAM (0)

void NuonEnvironment::InitAudio(void)
{
  bool bEnumerateSuccess = false;
  
  if(!bFMODInitialized)
  {
    //Select default sound driver
    FSOUND_SetDriver(0);
    //FSOUND_SetMixer(FSOUND_MIXER_QUALITY_FPU);
    //FSOUND_SetMixer(FSOUND_MIXER_QUALITY_MMXP5);
    //FSOUND_SetMixer(FSOUND_MIXER_QUALITY_MMXP6);
    //Initialize FMOD
    if(FSOUND_Init(MIX_RATE, MAX_SOFTWARE_CHANNELS, INIT_FLAGS))
    {
      bFMODInitialized = true;
    }
    else
    {
      return;
    }
  }

  if(audioStream)
  {
    FSOUND_Stream_Close(audioStream);
  }
  //Create stream
  audioStream = FSOUND_Stream_Create(AudioCallbacks::StreamCallback, nuonAudioBufferSize,
    (DEFAULT_SAMPLE_FORMAT | FSOUND_LOOP_NORMAL | FSOUND_NONBLOCKING), nuonAudioPlaybackRate, USER_PARAM);
    
  if(audioStream)
  {
    audioChannel = FSOUND_Stream_Play(FSOUND_FREE,audioStream);
  }
}

void NuonEnvironment::CloseAudio()
{
  if(bFMODInitialized)
  {
    MuteAudio(0xFFFFFFFFUL);
    StopAudio();
    if(audioStream)
    {
      FSOUND_Stream_Stop(audioStream);
      FSOUND_Stream_Close(audioStream);
      audioStream = 0;
    }
    FSOUND_Close();
    bFMODInitialized = false;
  }
}

void NuonEnvironment::MuteAudio(uint32 param)
{
  if(bFMODInitialized)
  {
    if(param)
    {
      FSOUND_SetMute(FSOUND_ALL,TRUE);
    }
    else
    {
      FSOUND_SetMute(FSOUND_ALL,FALSE);
    }
  }
}

void NuonEnvironment::SetAudioPlaybackRate(uint32 rate)
{
  if(rate > 96000)
  {
    rate = 96000;
  }
  else if(rate < 16000)
  {
    rate = 16000;
  }

  if(bFMODInitialized && audioStream)
  {
    if(FSOUND_IsPlaying(audioChannel))
    {
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
  {
    FSOUND_Stream_Stop(audioStream);
  }
}

uint32 lastLinearVolumeSetting = 0;

void NuonEnvironment::SetAudioVolume(uint32 volume)
{
  if(bFMODInitialized)
  {
    if(volume > 255)
    {
      volume = 255;
    }

    FSOUND_SetVolume(FSOUND_ALL,volume);
    lastLinearVolumeSetting = volume;
  }
}

void *NuonEnvironment::GetPointerToMemory(MPE *the_mpe, uint32 address, bool bCheckAddress)
{
  static char textBuf[1024];

  if(address < MAIN_BUS_BASE)
  {
    if(bCheckAddress)
    {
      if((address < MPE_ADDR_SPACE_BASE) || (address >= MPE1_ADDR_BASE))
      {
        sprintf(textBuf,"MPE%d Illegal Memory Address Operand %8.8X\nMPE0 pcexec: %8.8X\nMPE1 pcexec: %8.8X\nMPE2 pcexec: %8.8X\nMPE3 pcexec: %8.8X\n",
          the_mpe->mpeIndex,
          address,
          mpe[0]->pcexec,
          mpe[1]->pcexec,
          mpe[2]->pcexec,
          mpe[3]->pcexec);

        MessageBox( NULL, textBuf, "GetPointerToMemory error", MB_OK);
      }
    }

    return (void *)(((uint8 *)the_mpe->dtrom) + (address & MPE_VALID_MEMORY_MASK));
  }
  else if(address < SYSTEM_BUS_BASE)
  {
    if(bCheckAddress)
    {
      if((address > (MAIN_BUS_BASE + MAIN_BUS_VALID_MEMORY_MASK)))
      {
        sprintf(textBuf,"MPE%d Illegal Memory Address Operand %8.8X\nMPE0 pcexec: %8.8X\nMPE1 pcexec: %8.8X\nMPE2 pcexec: %8.8X\nMPE3 pcexec: %8.8X\n",
          the_mpe->mpeIndex,
          address,
          mpe[0]->pcexec,
          mpe[1]->pcexec,
          mpe[2]->pcexec,
          mpe[3]->pcexec);

        MessageBox( NULL, textBuf, "GetPointerToMemory error", MB_OK);
      }
    }

    return &mainBusDRAM[address & MAIN_BUS_VALID_MEMORY_MASK];
  }
  else if(address < ROM_BIOS_BASE)
  {
    if(bCheckAddress)
    {
      if((address > (SYSTEM_BUS_BASE + SYSTEM_BUS_VALID_MEMORY_MASK)))
      {
        sprintf(textBuf,"MPE%d Illegal Memory Address Operand %8.8X\nMPE0 pcexec: %8.8X\nMPE1 pcexec: %8.8X\nMPE2 pcexec: %8.8X\nMPE3 pcexec: %8.8X\n",
          the_mpe->mpeIndex,
          address,
          mpe[0]->pcexec,
          mpe[1]->pcexec,
          mpe[2]->pcexec,
          mpe[3]->pcexec);

        MessageBox( NULL, textBuf, "GetPointerToMemory error", MB_OK);
      }
    }
    return &systemBusDRAM[address & SYSTEM_BUS_VALID_MEMORY_MASK];
  }
  else
  {
    return flashEEPROM->GetBasePointer() + ((address - ROM_BIOS_BASE) & (DEFAULT_EEPROM_SIZE - 1));
  }
}

void NuonEnvironment::WriteFile(MPE *pMPE, uint32 fd, uint32 buf, uint32 len)
{
  char *pBuf = (char *)GetPointerToMemory(pMPE, buf, false);
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
  {
    mpe[i] = new MPE(i);
    mpe[i]->InitializeBankTable(mainBusDRAM,systemBusDRAM,flashEEPROM->GetBasePointer());
  }

  mpe[0]->mpeIndex = 0;
  mpe[0]->mpeStartAddress = 0x20000000;
  mpe[0]->mpeEndAddress = 0x207FFFFF;
  mpe[1]->mpeIndex = 1;
  mpe[1]->mpeStartAddress = 0x20800000;
  mpe[1]->mpeEndAddress = 0x20FFFFFF;
  mpe[2]->mpeIndex = 2;
  mpe[2]->mpeStartAddress = 0x21000000;
  mpe[2]->mpeEndAddress = 0x217FFFFF;
  mpe[3]->mpeIndex = 3;
  mpe[3]->mpeStartAddress = 0x21800000;
  mpe[3]->mpeEndAddress = 0x21FFFFFF;

  bProcessorStartStopChange = false;
  pendingCommRequests = 0;

  structMainDisplay.dispwidth = 720;
  structMainDisplay.dispheight = 480;
  bInterlaced = false;
  vblank_frequency = 60;
  fps = 40;
  InitializeColorSpaceTables();
  mainChannelLowerLimit = 0;
  overlayChannelLowerLimit = 0;
  mainChannelUpperLimit = 0;
  overlayChannelUpperLimit = 0;
  bMainBufferModified = true;
  bOverlayBufferModified = true;
  bRenderVideo = false;

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

  bSoundDeviceChosen = false;
  bUseCycleBasedTiming = false;
  cyclesPerAudioInterrupt = 1728000;
  cyclesPerVideoDisplay = 120000;
  audioInterruptCycleCount = cyclesPerAudioInterrupt;
  videoDisplayCycleCount = cyclesPerVideoDisplay;
  whichAudioInterrupt = 0;

  if(!pArgs)
  {
    LoadConfigFile("nuance.cfg");
  }
  else
  {
    LoadConfigFile(pArgs[1]);
  }
}

uint32 NuonEnvironment::GetBufferSize(uint32 channelMode)
{
  uint32 bufferSize;

  if((channelMode & BUFFER_SIZE_64K) == 0)
  {
    bufferSize = 8192;
  }
  else
  {
    bufferSize = 512UL << (((channelMode & BUFFER_SIZE_64K) >> 5) & 0x7UL);
  }

  return bufferSize;
}

void NuonEnvironment::InitBios()
{
  ::InitBios(mpe[3]);
}

NuonEnvironment::~NuonEnvironment()
{
  uint32 i;

  //Close stream and shut down audio library
  CloseAudio();

  //Stop the processor thread
  for(i = 0; i < 4; i++)
  {
    mpe[i]->Halt();
  }

  //Delete the MPE objects
  for(i = 0; i < 4; i++)
  {
    delete mpe[i];
  }

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
    {
      break;
    }

    index++;
  }

  line[index] = replaceChar;
}

void NuonEnvironment::SetDVDBaseFromFileName(char *filename)
{
  uint32 i;

  delete [] dvdBase;
  i = strlen(filename);
  dvdBase = new char[i+1];
  strncpy(dvdBase,filename,i+1);
  while(i >= 0)
  {
     if(dvdBase[i] == '\\')
     {
       break;
     }
   
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
  {
    return CONFIG_EOF;
  }

  fscanf(file,"%s",buf);
  firstChar = buf[0];

  if(firstChar == CONFIG_COMMENT_CHAR)
  {
    return CONFIG_COMMENT;
  }
  else if(firstChar == CONFIG_VARIABLE_START_CHAR)
  {
    return CONFIG_VARIABLE_START;
  }
  else if(firstChar == CONFIG_VARIABLE_FINISH_CHAR)
  {
    return CONFIG_VARIABLE_FINISH;
  }  
  else if((firstChar == '<') || (firstChar == '>') || (firstChar == '"'))
  {
    return CONFIG_RESERVED;
  }
  else
  {
    return CONFIG_STRING;
  }
}

char *dvdWriteBase;
char *dvdReadBase;
char *dvdReadWriteBase;

bool NuonEnvironment::LoadConfigFile(char *fileName)
{
  char line[1025];
  FILE *configFile = 0;
  ConfigTokenType tokenType;

  configFile = fopen(fileName,"r");
  if(!configFile)
  {
    return false;
  }

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
        else if(_strnicmp(&line[1],"PixelShaders]",sizeof("PixelShaders]")) == 0)
        {
          tokenType = ReadConfigLine(configFile,line);
          videoOptions.bUseShaders = !_stricmp(line,"Enabled");
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
          {
            fps = 1;
          }
          else if(fps > 240)
          {
            fps = 240;
          }
        }

        break;
      default:
        break;
    }
  }

  fclose(configFile);
  return true;
}
