#ifndef mediaH
#define mediaH

#include "basetypes.h"

class MPE;

enum
{
    MEDIA_BOOT_DEVICE=1,
    MEDIA_DVD,
    MEDIA_REMOTE,
    MEDIA_FLASH,
    MEDIA_SBMEM,

/* now for open modes */
    MEDIA_READ = 0,
    MEDIA_WRITE,
    MEDIA_RW,

/* ioctls */
    MEDIA_IOCTL_SET_MODE = 0,
    MEDIA_IOCTL_GET_MODE,
    MEDIA_IOCTL_EJECT,
    MEDIA_IOCTL_RETRACT,
    MEDIA_IOCTL_FLUSH,
    MEDIA_IOCTL_GET_DRIVETYPE,
    MEDIA_IOCTL_READ_BCA,
    MEDIA_IOCTL_GET_START,
    MEDIA_IOCTL_SET_START,
    MEDIA_IOCTL_SET_END,
    MEDIA_IOCTL_GET_PHYSICAL,
    MEDIA_IOCTL_OVERWRITE,
    MEDIA_IOCTL_ERASE,
    MEDIA_IOCTL_SIZE,
    MEDIA_IOCTL_CDDATA_OFFSET,

/* now for read/write modes (callback types) */
    MCB_END     = 0x1<<30,
    MCB_EVERY   = 0x2<<30,
    MCB_ERROR   = 0x3<<30,

/* DVD modes */
    MEDIA_IOCTL_MODE_DATA=0,
    MEDIA_IOCTL_MODE_AUDIO,
    MEDIA_IOCTL_MODE_SUBCH,
    MEDIA_IOCTL_MODE_AUDIO_FUZZY,
};

#define HAVE_BOOT_MEDIA (1<<(MEDIA_BOOT_DEVICE - MEDIA_BOOT_DEVICE))
#define HAVE_DVD_MEDIA (1<<(MEDIA_DVD - MEDIA_BOOT_DEVICE))
#define HAVE_REMOTE_MEDIA (1<<(MEDIA_REMOTE - MEDIA_BOOT_DEVICE))
#define HAVE_FLASH_MEDIA (1<<(MEDIA_FLASH - MEDIA_BOOT_DEVICE))

#define kTypeUnknown (0)
#define kTypeCD (1)
#define kTypeDVDSingle (2)
#define kTypeDVDDouble (3)

struct MediaDevInfo
{
 int32 type;
 int32 state;
 int32 sectorsize;
 int32 bus;
 int32 id;
 int32 datarate;
};

struct MediaHandleInfo
{
  MediaDevInfo *devInfo;
  char *name;
};

#define BLOCK_SIZE_DVD (2048)
#define DATA_RATE_DVD ((uint32)(1024.0*1024.0*1.5))
#define MEDIA_DEVICES_AVAILABLE ((HAVE_DVD_MEDIA) | (HAVE_BOOT_MEDIA))

//LONG HANDLE, LONG CTL, LONG *VALUE
void MediaIoctl(MPE &mpe);
void MediaGetDevicesAvailable(MPE &mpe);
void MediaGetInfo(MPE &mpe);
void MediaOpen(MPE &mpe);
void MediaClose(MPE &mpe);
void MediaRead(MPE &mpe);
void MediaWrite(MPE &mpe);
void MediaShutdownMPE(MPE &mpe);
void MediaInitMPE(MPE &mpe);
void MediaInitMPE(const uint32 i);
void SpinWait(MPE &mpe);

#endif
