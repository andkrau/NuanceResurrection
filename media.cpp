#include "basetypes.h"
#include <windows.h>
#include "byteswap.h"
#include "Bios.h"
#include "mpe.h"
#include "media.h"
#include "mpe_alloc.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"

extern NuonEnvironment nuonEnv;
extern uint32 mpeFlags[];

uint32 media_mpe_allocated = 0;
uint32 media_mpe = 0;

MediaDevInfo DeviceInfo[] = {
  {0,0,0,0,0,0},
  {0,0,0,0,0,0},
  {0,0,0,0,0,0},
  {MEDIA_BOOT_DEVICE,0,BLOCK_SIZE_DVD,0,0,DATA_RATE_DVD},
  {MEDIA_BOOT_DEVICE,0,BLOCK_SIZE_DVD,0,0,DATA_RATE_DVD}};

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
  bool loadStatus = false;

  mpeFlags[i] |= (MPE_ALLOC_BIOS | MPE_HAS_MINI_BIOS);

  //nuonEnv.mpe[i]->inten1 = INT_COMMRECV;

  //Load the minibios code
  if(i == 0)
  {
    loadStatus = nuonEnv.mpe[i].LoadCoffFile("minibios.cof",false);

    if(!loadStatus)
    {
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
      MessageBox(NULL,"Missing File!","Could not load minibiosX.cof",MB_OK);
    }

    nuonEnv.mpe[i].intvec1 = MINIBIOSX_INTVEC1_HANDLER_ADDRESS;
    nuonEnv.mpe[i].intvec2 = MINIBIOSX_INTVEC2_HANDLER_ADDRESS;
  }

  //Mask interrupts
  nuonEnv.mpe[i].intctl = 0xAA;
  nuonEnv.mpe[i].UpdateInvalidateRegion(MPE_IRAM_BASE, OVERLAY_SIZE);

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
  int32 which;
  //Check to see if the media code is already running on an MPE
  //in which case don't reallocate it and simply return the index
  //of the MPE that it is running on

  if(media_mpe_allocated)
  {
    mpe.regs[0] = media_mpe;
    return;
  }

  which = -1;

  for(uint32 i = 0; i < 4; i++)
  {
    if((mpeFlags[i] & (MPE_ALLOC_BIOS|MPE_ALLOC_USER)) == 0)
    {
        which = i;
        MediaInitMPE(i);
        break;
    }
  }

  mpe.regs[0] = which;
}

char *fileNameArray[] = {"stdin","stdout","stderr",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint32 fileModeArray[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool bCableFileOpen;
bool bEmulatorFileOpen;
bool bNvramFileOpen;

#define FIRST_DVD_FD (3)
#define LAST_DVD_FD (19)

#define NUON_FD_BOOT_DEVICE (3)
#define NUON_FD_DVD (4)
#define NUON_FD_REMOTE (5)
#define NUON_FD_FLASH (6)
#define NUON_FD_SBMEM (7)

int FindFirstFreeHandle()
{
  int i = 0;

  for(i = FIRST_DVD_FD; i <= LAST_DVD_FD; i++)
  {
    if(!fileNameArray[i])
    {
      return i;
    }
  }

  return 0;
}

void MediaOpen(MPE &mpe)
{
  uint32 device = mpe.regs[0];
  char *name = (char *)nuonEnv.GetPointerToMemory(mpe,mpe.regs[1]);
  uint32 mode = mpe.regs[2];
  uint32 *pBlockSize = (uint32 *)nuonEnv.GetPointerToMemory(mpe,mpe.regs[3]);
  int32 handle = 0;
  uint32 bufLength;
  char *baseString;

  if((device == MEDIA_BOOT_DEVICE) || (device == MEDIA_DVD))
  {
    handle = FindFirstFreeHandle();

    if(handle)
    {    
      if(pBlockSize)
      {
        *pBlockSize = BLOCK_SIZE_DVD; 
        SwapScalarBytes(pBlockSize);
      }

      baseString = nuonEnv.GetDVDBase();

      //Treat iso9660 device reads as DVD device reads
      if(!strncmp("/iso9660/",name,9))
      {
        name = &name[9];
      }
      else if(!strncmp("/udf/",name,5))
      {
        name = &name[5];
      }

      bufLength = strlen(name)+strlen(baseString)+1;
      fileNameArray[handle] = new char[bufLength];
      strcpy(fileNameArray[handle],baseString);
      strcat(fileNameArray[handle],name);
      fileModeArray[handle] = mode;
    }
  }

  mpe.regs[0] = handle;
}

void MediaClose(MPE &mpe)
{
  int32 handle = mpe.regs[0];

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
  {
    if(fileNameArray[handle])
    {
      delete [] fileNameArray[handle];
      fileNameArray[handle] = 0;
    }
  }
}

void MediaGetDevicesAvailable(MPE &mpe)
{
  mpe.regs[0] = MEDIA_DEVICES_AVAILABLE;
}

void MediaGetInfo(MPE &mpe)
{
  int32 handle = mpe.regs[0];
  void *pDevInfo = (void *)mpe.regs[1];
  int32 result = -1;
  uint32 sectorsize, datarate, id;

  if(pDevInfo)
  {
    if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
    {
      pDevInfo = nuonEnv.GetPointerToMemory(mpe,(uint32)pDevInfo);

      id = handle;
      datarate = DATA_RATE_DVD;
      sectorsize = BLOCK_SIZE_DVD;

      SwapScalarBytes(&sectorsize);
      SwapScalarBytes(&datarate);
      SwapScalarBytes(&id);          

      ((MediaDevInfo *)pDevInfo)->sectorsize = BLOCK_SIZE_DVD;
      ((MediaDevInfo *)pDevInfo)->datarate = DATA_RATE_DVD;
      ((MediaDevInfo *)pDevInfo)->id = handle;
      ((MediaDevInfo *)pDevInfo)->type = 0;
      ((MediaDevInfo *)pDevInfo)->bus = 0;
      ((MediaDevInfo *)pDevInfo)->state = 0;
      result = 0;
    }
  }
}

bool bCallingMediaCallback = false;

void MediaRead(MPE &mpe)
{
  FILE *inFile;
  int32 handle = mpe.regs[0];
  int32 mode = mpe.regs[1];
  uint32 startblock = mpe.regs[2];
  uint32 blockcount = mpe.regs[3];
  uint32 buffer = mpe.regs[4];
  uint32 callback = mpe.regs[5];
  uint32 readCount;
  void *pBuf;

  mpe.regs[0] = -1;

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
  {
    if(fileNameArray[handle] && buffer && (fileModeArray[handle] != MEDIA_WRITE))
    {
      inFile = fopen(fileNameArray[handle],"rb");
      if(inFile)
      {
        pBuf = nuonEnv.GetPointerToMemory(mpe,buffer);
        fseek(inFile,startblock*BLOCK_SIZE_DVD,SEEK_SET);
        readCount = fread(pBuf,BLOCK_SIZE_DVD,blockcount,inFile);
        if(readCount >= (blockcount - 1))
        {
          fclose(inFile);
          mpe.regs[0] = mode;
          mpe.regs[1] = blockcount;
          if(callback)
          {
            mpe.pcexec = callback;
            bCallingMediaCallback = true;
          }
        }
      }
    }
  }
}

void MediaWrite(MPE &mpe)
{
  FILE *outFile;
  int32 handle = mpe.regs[0];
  int32 mode = mpe.regs[1];
  uint32 startblock = mpe.regs[2];
  uint32 blockcount = mpe.regs[3];
  uint32 buffer = mpe.regs[4];
  uint32 callback = mpe.regs[5];
  uint32 writeCount;
  void *pBuf;

  mpe.regs[0] = -1;

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
  {
    if(fileNameArray[handle] && buffer && (fileModeArray[handle] != MEDIA_READ))
    {
      //try to open the existing file for read/write without erasing the contents
      outFile = fopen(fileNameArray[handle],"r+b");
      
      if(!outFile)
      {
        //try to create the file
        outFile = fopen(fileNameArray[handle],"w+b");
      }

      if(outFile)
      {
        pBuf = nuonEnv.GetPointerToMemory(mpe,buffer);
        fseek(outFile,startblock*BLOCK_SIZE_DVD,SEEK_SET);
        writeCount = fwrite(pBuf,BLOCK_SIZE_DVD,blockcount,outFile);
        if(writeCount >= (blockcount - 1))
        {
          fclose(outFile);
          mpe.regs[0] = mode;
          mpe.regs[1] = blockcount;
          if(callback)
          {
            mpe.pcexec = callback;
            bCallingMediaCallback = true;
          }
        }
      }
    }
  }
}

void MediaIoctl(MPE &mpe)
{
  int32 handle = mpe.regs[0];
  int32 ctl = mpe.regs[1];
  uint32 value = mpe.regs[2];
  uint32 *ptr;
  char ctlStr[2];

  mpe.regs[0] = -1;
  ctlStr[0] = '0'+ctl;
  ctlStr[1] = 0;

  if((handle >= FIRST_DVD_FD) && (handle <= LAST_DVD_FD))
  {
    mpe.regs[0] = 0;

    if(fileNameArray[handle])
    {
      switch(ctl)
      {
        case MEDIA_IOCTL_SET_MODE:
          break;
        case MEDIA_IOCTL_GET_MODE:
          break;
        case MEDIA_IOCTL_EJECT:
          break;
        case MEDIA_IOCTL_RETRACT:
          break;
        case MEDIA_IOCTL_FLUSH:
          break;
        case MEDIA_IOCTL_GET_DRIVETYPE:
          //return kTypeDvdSingle for now, but should really allow the user to declare the disk type
          mpe.regs[0] = kTypeDVDSingle;
          break;
        case MEDIA_IOCTL_READ_BCA:
          break;
        case MEDIA_IOCTL_GET_START:
          break;
        case MEDIA_IOCTL_SET_START:
          break;
        case MEDIA_IOCTL_SET_END:
          break;
        case MEDIA_IOCTL_GET_PHYSICAL:
          if(value)
          {
            ptr = (uint32 *)nuonEnv.GetPointerToMemory(mpe,value);
            //For now, return physical sector zero, but in the future there needs to be some sort
            //of TOC to allow for loading from image files in which case the base file sector will
            //be non-zero
            *ptr = 0;
            SwapScalarBytes(ptr);
          }
          //return DVD layer 0 (should this be zero-based or one-based?)
          mpe.regs[0] = 0;
          break;
        case MEDIA_IOCTL_OVERWRITE:
          break;
        case MEDIA_IOCTL_ERASE:
          break;
        case MEDIA_IOCTL_SIZE:
          break;
        case MEDIA_IOCTL_CDDATA_OFFSET:
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
  uint32 result = 0;
  const uint32 status = mpe.regs[0];
  uint32 * const pMediaWaiting = (uint32 *)nuonEnv.GetPointerToMemory(mpe,MEDIAWAITING_ADDRESS,false);

  if((status >> 30) == 0x03)
    result = status;

  SwapScalarBytes(&result);
  *pMediaWaiting = result;
}
