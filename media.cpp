#include "basetypes.h"
#include <windows.h>
#include <string>
#include "byteswap.h"
#include "Bios.h"
#include "mpe.h"
#include "media.h"
#include "mpe_alloc.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"

extern NuonEnvironment nuonEnv;
extern uint32 mpeFlags[];

bool bCallingMediaCallback = false; // globally used

uint32 media_mpe_allocated = 0;
uint32 media_mpe = 0;

static MediaDevInfo DeviceInfo[] = {
  {0,0,0,0,0,0},
  {0,0,0,0,0,0},
  {0,0,0,0,0,0},
  {(int32)eMedia::MEDIA_BOOT_DEVICE,0,BLOCK_SIZE_DVD,0,0,DATA_RATE_DVD},
  {(int32)eMedia::MEDIA_BOOT_DEVICE,0,BLOCK_SIZE_DVD,0,0,DATA_RATE_DVD}};

void MediaShutdownMPE(MPE &mpe)
{
  if(media_mpe_allocated)
  {
    //Halt the media processor
    nuonEnv.mpe[media_mpe].Halt();
    //Disable commrecv interrupts
    nuonEnv.mpe[media_mpe].inten1 &= ~(INT_COMMRECV);
    //Mark as free and clear Mini BIOS flag
    mpeFlags[media_mpe] &= ~(MPE_ALLOC_USER | MPE_ALLOC_BIOS | MPE_HAS_MINI_BIOS);
    nuonEnv.bProcessorStartStopChange = true;
  }

  media_mpe_allocated = 0;
}

void MediaInitMPE(const uint32 i)
{
  bool loadStatus;

  mpeFlags[i] |= (MPE_ALLOC_BIOS | MPE_HAS_MINI_BIOS);

  //nuonEnv.mpe[i]->inten1 = INT_COMMRECV;

  //Load the minibios code
  if(i == 0)
  {
    loadStatus = nuonEnv.mpe[i].LoadCoffFile("minibios.cof",false);

    if(!loadStatus)
    {
      char tmp[1024];
      GetModuleFileName(NULL, tmp, 1024);
      string tmps(tmp);
      size_t idx = tmps.find_last_of('\\');
      if (idx != string::npos)
        tmps = tmps.substr(0, idx+1);
      loadStatus = nuonEnv.mpe[i].LoadCoffFile((tmps+"minibios.cof").c_str(),false);
      if(!loadStatus)
        MessageBox(NULL,"Missing File!","Could not load minibios.cof",MB_OK);
    }

    nuonEnv.mpe[i].intvec1 = MINIBIOS_INTVEC1_HANDLER_ADDRESS;
    nuonEnv.mpe[i].intvec2 = MINIBIOS_INTVEC2_HANDLER_ADDRESS;
  }
  else
  {
    loadStatus = nuonEnv.mpe[i].LoadCoffFile("minibiosX.cof",false);

    if(!loadStatus)
    {
      char tmp[1024];
      GetModuleFileName(NULL, tmp, 1024);
      string tmps(tmp);
      size_t idx = tmps.find_last_of('\\');
      if (idx != string::npos)
        tmps = tmps.substr(0, idx+1);
      loadStatus = nuonEnv.mpe[i].LoadCoffFile((tmps+"minibiosX.cof").c_str(),false);
      if(!loadStatus)
        MessageBox(NULL,"Missing File!","Could not load minibiosX.cof",MB_OK);
    }

    nuonEnv.mpe[i].intvec1 = MINIBIOSX_INTVEC1_HANDLER_ADDRESS;
    nuonEnv.mpe[i].intvec2 = MINIBIOSX_INTVEC2_HANDLER_ADDRESS;
  }

  //Mask interrupts
  nuonEnv.mpe[i].intctl = 0xAA;
  nuonEnv.mpe[i].UpdateInvalidateRegion(MPE_IRAM_BASE, MPE::overlayLengths[i]);

  //Set pcexec to the minibios spinwait routine.
  if(i == 0)
  {
    nuonEnv.mpe[i].sp = 0x20101BE0;
    nuonEnv.mpe[i].pcexec = MINIBIOS_ENTRY_POINT;
  }
  else
  {
    nuonEnv.mpe[i].pcexec = MINIBIOSX_SPINWAIT_ADDRESS;
  }

  media_mpe = i;
  media_mpe_allocated = 1;

  if(!(nuonEnv.mpe[i].mpectl & MPECTRL_MPEGO))
  {
    nuonEnv.mpe[i].mpectl |= MPECTRL_MPEGO;
    nuonEnv.bProcessorStartStopChange = true;
  }
}

void MediaInitMPE(MPE &mpe)
{
  //Check to see if the media code is already running on an MPE
  //in which case don't reallocate it and simply return the index
  //of the MPE that it is running on

  if(media_mpe_allocated)
  {
    mpe.regs[0] = media_mpe;
    return;
  }

  int32 which = -1;

  for(uint32 i = 0; i < 4; i++)
  {
    if((mpeFlags[i] & (MPE_ALLOC_BIOS | MPE_ALLOC_USER)) == 0)
    {
        which = i;
        MediaInitMPE(i);
        break;
    }
  }

  mpe.regs[0] = which;
}

static std::string fileNameArray[] = {"stdin","stdout","stderr","","","","","","","","","","","","","","","","",""};
static uint32 fileModeArray[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

#define FIRST_DVD_FD (3)
#define LAST_DVD_FD (19)

#define NUON_FD_BOOT_DEVICE (3)
#define NUON_FD_DVD (4)
#define NUON_FD_REMOTE (5)
#define NUON_FD_FLASH (6)
#define NUON_FD_SBMEM (7)

static int FindFirstFreeHandle()
{
  for(int i = FIRST_DVD_FD; i <= LAST_DVD_FD; i++)
    if(fileNameArray[i].empty())
      return i;

  return 0;
}

void MediaOpen(MPE &mpe)
{
  const uint32 device = mpe.regs[0];
  const uint32 mode = mpe.regs[2];
  int32 handle = 0;

  if(((eMedia)device == eMedia::MEDIA_BOOT_DEVICE) || ((eMedia)device == eMedia::MEDIA_DVD))
  {
    handle = FindFirstFreeHandle();

    if(handle)
    {    
      uint32 * const pBlockSize = (uint32 *)nuonEnv.GetPointerToMemory(mpe,mpe.regs[3]);
      if(pBlockSize)
      {
        *pBlockSize = SwapBytes((uint32)BLOCK_SIZE_DVD);
      }

      const char * const baseString = nuonEnv.GetDVDBase();

      //Treat iso9660 device reads as DVD device reads
      const char * name = (char *)nuonEnv.GetPointerToMemory(mpe, mpe.regs[1]);
      if(!strncmp("/iso9660/",name,9))
      {
        name = &name[9];
      }
      else if(!strncmp("/udf/",name,5))
      {
        name = &name[5];
      }

      fileNameArray[handle] = baseString;
      fileNameArray[handle] += name;
      fileModeArray[handle] = mode;
    }
  }

  mpe.regs[0] = handle;
}

void MediaClose(MPE &mpe)
{
  const int32 handle = mpe.regs[0];

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
    fileNameArray[handle].clear();
}

void MediaGetDevicesAvailable(MPE &mpe)
{
  mpe.regs[0] = MEDIA_DEVICES_AVAILABLE;
}

void MediaGetInfo(MPE &mpe)
{
  const int32 handle = mpe.regs[0];
  const uint32 devInfo = mpe.regs[1];
  int32 result = -1;

  if(devInfo)
  {
    if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
    {
      MediaDevInfo * const pDevInfo = (MediaDevInfo*)nuonEnv.GetPointerToMemory(mpe,devInfo);

      const uint32 id = SwapBytes((uint32)handle);
      const uint32 datarate = SwapBytes((uint32)DATA_RATE_DVD);
      const uint32 sectorsize = SwapBytes((uint32)BLOCK_SIZE_DVD);

      pDevInfo->sectorsize = BLOCK_SIZE_DVD;
      pDevInfo->datarate = DATA_RATE_DVD;
      pDevInfo->id = handle;
      pDevInfo->type = 0;
      pDevInfo->bus = 0;
      pDevInfo->state = 0;
      result = 0;
    }
  }

  mpe.regs[0] = result;
}

void MediaRead(MPE &mpe)
{
  const int32 handle = mpe.regs[0];
  const int32 mode = mpe.regs[1];
  const uint32 startblock = mpe.regs[2];
  const uint32 blockcount = mpe.regs[3];
  const uint32 buffer = mpe.regs[4];
  const uint32 callback = mpe.regs[5];

  mpe.regs[0] = -1;

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
  {
    if(!fileNameArray[handle].empty() && buffer && ((eMedia)fileModeArray[handle] != eMedia::MEDIA_WRITE))
    {
      FILE* inFile;
      if(fopen_s(&inFile,fileNameArray[handle].c_str(),"rb") == 0)
      {
        void* pBuf = nuonEnv.GetPointerToMemory(mpe,buffer);
        fseek(inFile,startblock*BLOCK_SIZE_DVD,SEEK_SET);
        const uint32 readCount = (uint32)fread(pBuf,BLOCK_SIZE_DVD,blockcount,inFile);
        if(readCount >= (blockcount - 1))
        {
          mpe.regs[0] = mode;
          mpe.regs[1] = blockcount;
          if(callback)
          {
            mpe.pcexec = callback;
            bCallingMediaCallback = true;
          }
        }
        fclose(inFile);
      }
    }
  }
}

void MediaWrite(MPE &mpe)
{
  const int32 handle = mpe.regs[0];
  const int32 mode = mpe.regs[1];
  const uint32 startblock = mpe.regs[2];
  const uint32 blockcount = mpe.regs[3];
  const uint32 buffer = mpe.regs[4];
  const uint32 callback = mpe.regs[5];

  mpe.regs[0] = -1;

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
  {
    if(!fileNameArray[handle].empty() && buffer && ((eMedia)fileModeArray[handle] != eMedia::MEDIA_READ))
    {
      //try to open the existing file for read/write without erasing the contents
      FILE* outFile;
      if(fopen_s(&outFile,fileNameArray[handle].c_str(),"r+b") != 0)
      {
        //try to create the file
        if(fopen_s(&outFile,fileNameArray[handle].c_str(),"w+b") != 0)
          outFile = nullptr;
      }

      if(outFile)
      {
        const void* pBuf = nuonEnv.GetPointerToMemory(mpe,buffer);
        fseek(outFile,startblock*BLOCK_SIZE_DVD,SEEK_SET);
        const uint32 writeCount = (uint32)fwrite(pBuf,BLOCK_SIZE_DVD,blockcount,outFile);
        if(writeCount >= (blockcount - 1))
        {
          mpe.regs[0] = mode;
          mpe.regs[1] = blockcount;
          if(callback)
          {
            mpe.pcexec = callback;
            bCallingMediaCallback = true;
          }
        }
        fclose(outFile);
      }
    }
  }
}

void MediaIoctl(MPE &mpe)
{
  const int32 handle = mpe.regs[0];
  const int32 ctl = mpe.regs[1];
  const uint32 value = mpe.regs[2];
  //char ctlStr[2];

  mpe.regs[0] = -1;
  //ctlStr[0] = '0'+ctl;
  //ctlStr[1] = 0;

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
  {
    mpe.regs[0] = 0;

    if(!fileNameArray[handle].empty())
    {
      switch((eMedia)ctl)
      {
        case eMedia::MEDIA_IOCTL_SET_MODE:
          break;
        case eMedia::MEDIA_IOCTL_GET_MODE:
          break;
        case eMedia::MEDIA_IOCTL_EJECT:
          break;
        case eMedia::MEDIA_IOCTL_RETRACT:
          break;
        case eMedia::MEDIA_IOCTL_FLUSH:
          break;
        case eMedia::MEDIA_IOCTL_GET_DRIVETYPE:
          //return kTypeDvdSingle for now, but should really allow the user to declare the disk type
          mpe.regs[0] = kTypeDVDSingle;
          break;
        case eMedia::MEDIA_IOCTL_READ_BCA:
          break;
        case eMedia::MEDIA_IOCTL_GET_START:
          break;
        case eMedia::MEDIA_IOCTL_SET_START:
          break;
        case eMedia::MEDIA_IOCTL_SET_END:
          break;
        case eMedia::MEDIA_IOCTL_GET_PHYSICAL:
          if(value)
          {
            uint32* const ptr = (uint32 *)nuonEnv.GetPointerToMemory(mpe,value);
            //!! For now, return physical sector zero, but in the future there needs to be some sort
            //of TOC to allow for loading from image files in which case the base file sector will
            //be non-zero
            *ptr = SwapBytes(0u);
          }
          //return DVD layer 0 (should this be zero-based or one-based?)
          mpe.regs[0] = 0;
          break;
        case eMedia::MEDIA_IOCTL_OVERWRITE:
          break;
        case eMedia::MEDIA_IOCTL_ERASE:
          break;
        case eMedia::MEDIA_IOCTL_SIZE:
          break;
        case eMedia::MEDIA_IOCTL_CDDATA_OFFSET:
          break;
        default:
          mpe.regs[0] = -1;
          break;
      }
    }
  }
}

void SpinWait(MPE &mpe)
{
  const uint32 status = mpe.regs[0];
  uint32 * const pMediaWaiting = (uint32 *)nuonEnv.GetPointerToMemory(mpe,MEDIAWAITING_ADDRESS,false);

  uint32 result = 0;
  if((status >> 30) == 0x03)
    result = SwapBytes(status);

  *pMediaWaiting = result;
}
