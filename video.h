#ifndef video_h
#define video_h

#include "basetypes.h"
#include "mpe.h"
#include "external\glew-2.1.0\include\GL\glew.h"

#define ALLOCATED_TEXTURE_WIDTH VIDEO_WIDTH
#define ALLOCATED_TEXTURE_HEIGHT (480)

class VideoOptions
{
public:
  VideoOptions()
  {
    bAlwaysUpdateVideo = false;
  }
  
  bool bAlwaysUpdateVideo;
};

struct vidTexInfo
{
  GLuint displayListName[4];
  GLuint mainTexName;
  GLuint osdTexName;
  GLuint borderTexName;
  GLuint LUTTexName;
  GLuint transparencyTexName;
  GLfloat borderColor[4];
  GLfloat transColor[4];
  GLfloat mainTexCoords[8];
  GLfloat osdTexCoords[8];
  bool bUpdateDisplayList;
};

enum eVideoRequest
{
  VIDEOREQUEST_NONE = 0,
  VIDEOREQUEST_INITGLEW,
  VIDEOREQUEST_VIDCONFIG,
  VIDEOREQUEST_VIDSETUP,
  VIDEOREQUEST_SETDEFAULTCOLOR,
  VIDEOREQUEST_SETVIDEOMODE,
  VIDEOREQUEST_VIDCHANGEBASE,
  VIDEOREQUEST_VIDCHANGESCROLL,
  VIDEOREQUEST_VIDSETCLUTRANGE,
  VIDEOREQUEST_LAST
};

struct VidDisplay
{
  int32 dispwidth;       /* width of display (-1 for default) */
  int32 dispheight;      /* height of display (-1 for default) */
  int32 bordcolor;       /* border color (24bpp) */
  int32 progressive;     /* flag for interlace (0) or progressive (1) */
  int32 fps;             /* fields per second (16.16 fixed point) */
  int16 pixel_aspect_x;   /* pixel aspect ratio (read only) */
  int16 pixel_aspect_y;
  int16 screen_aspect_x;  /* screen aspect ratio (read only) */
  int16 screen_aspect_y;
  int32 reserved[3];     /* reserved for future expansion; set to 0 */
  /* WARNING: reserved[3] may have to remain reserved forever:
     the Extiva2 BIOS does not set it (it sets only the first 36 bytes
     of the structure */
};

/* structure for configuring a specific channel */
struct VidChannel
{
  int32 dmaflags;       /* DMA flags for writing to or reading from a channel */
  void *base;           /* base address for the channel */
  int32 dest_xoff;      /* x offset for screen image (integer; -1 == center automatically) */
  int32 dest_yoff;      /* y offset for screen image (integer; -1 == center automatically) */
  int32 dest_width;     /* width of the output on screen  (integer) */
  int32 dest_height;    /* height of the output on screen (integer) */
  int32 src_xoff;       /* x offset within source data (16.16 fixed point) */
  int32 src_yoff;       /* y offset within source data (16.16 fixed point) */
  int32 src_width;      /* width of source material (16.16 fixed point) */
  int32 src_height;     /* height of source material (16.16 fixed point */
  uint8 clut_select;    /* (for 4bpp only): which 16 CLUT entries to use */
  uint8 alpha;          /* (for 16bpp only): default ALPHA to use on channel */
  uint8 vfilter;        /* vertical filter to apply */
  uint8 hfilter;        /* horizontal filter to apply */
  int32 reserved[5];    /* reserved for future expansion */
};

#define VID_HFILTER_NONE 0
#define VID_HFILTER_4TAP 4

#define VID_VFILTER_NONE 0
#define VID_VFILTER_2TAP 2
#define VID_VFILTER_4TAP 4

#define VID_CHANNEL_MAIN 0
#define VID_CHANNEL_OSD  1

/* Video mode defines */

#define VIDEO_WIDTH	(720)
#define VIDEO_HEIGHT_NTSC (480)
#define VIDEO_HEIGHT_PAL (576)

#define VIDEO_MODE_NTSC	(1)
#define VIDEO_MODE_PAL (2)

uint32 *AllocateTextureMemory32(uint32 size, const bool bOverlay);
void FreeTextureMemory(uint32 *pBuffer, const bool bOverlay);
void VidConfig(MPE &mpe);
void VidQueryConfig(MPE &mpe);
void VidSetup(MPE &mpe);
void VidChangeScroll(MPE &mpe);
void VidChangeBase(MPE &mpe);
void SetDefaultColor(MPE &mpe);
void VidSetCLUTRange(MPE &mpe);
void SetVideoMode(void);
void InitializeColorSpaceTables(void);
void RenderVideo(int width, int height);
void VideoCleanup(void);
void IncrementVideoFieldCounter(void);

#endif
