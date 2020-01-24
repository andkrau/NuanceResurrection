//---------------------------------------------------------------------------
#include <windows.h>
#include "external\glew-2.1.0\include\GL\glew.h"
#include "external\glew-2.1.0\include\GL\wglew.h"
#include <stdio.h>
#include "GLWindow.h"
#include "Bios.h"
#include "basetypes.h"
#include "byteswap.h"
#include "mpe.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "ShaderProgram.h"
#include "video.h"
//---------------------------------------------------------------------------

extern NuonEnvironment nuonEnv;

#define TEXTURE_TARGET (GL_TEXTURE_2D)

const GLint mainInternalTextureFormat32 = GL_RGBA8;
const GLint mainInternalTextureFormat16 = GL_RG8;
const GLint mainExternalTextureFormat32 = GL_BGRA;
const GLint mainExternalTextureFormat16 = GL_RG;
const GLint mainPixelType32 = GL_UNSIGNED_BYTE;
const GLint mainPixelType16 = GL_UNSIGNED_BYTE;
const GLint mainTextureUnit = GL_TEXTURE0;

const GLint osdInternalTextureFormat32 = GL_RGBA8;
const GLint osdInternalTextureFormat16 = GL_RG8;
const GLint osdExternalTextureFormat32 = GL_BGRA;
const GLint osdExternalTextureFormat16 = GL_RG;
const GLint osdPixelType32 = GL_UNSIGNED_BYTE;
const GLint osdPixelType16 = GL_UNSIGNED_BYTE;
const GLint osdTextureUnit = GL_TEXTURE1;

const GLint lutTextureUnit = GL_TEXTURE2;

const bool bUseBilinearFiltering = false;

static bool bTexturesInitialized = false;
static bool bShadersInstalled = false;
static bool bSetupViewport = false;

vidTexInfo videoTexInfo;

uint32 main_fetch_pixel_type;
uint32 main_write_pixel_type;
uint32 vdgCLUT[256];

uint32 overlay_fetch_pixel_type;
uint32 overlay_write_pixel_type;
uint32 overlay_pixel_type_width;

uint32 *mainDisplayBuffer = 0;
uint32 *mainChannelBuffer = 0;
uint32 *overlayChannelBuffer = 0;
uint8 *clutPtr = (uint8 *)vdgCLUT;

bool bMainChannelActive = false;
bool bOverlayChannelActive = false;

int bMainTexturePixType = -1;
int bOverlayTexturePixType = -1;

bool bCanDisplayVideo = false;

VidDisplay structMainDisplay;
VidChannel structMainChannel;
VidChannel structOverlayChannel;
VidDisplay structMainDisplayPrev;
VidChannel structMainChannelPrev;
VidChannel structOverlayChannelPrev;

#define CHANNELSTATE_OVERLAY_ACTIVE (0x02)
#define CHANNELSTATE_MAIN_ACTIVE (0x01)

uint32 channelState = 0;
uint32 channelStatePrev = 0;

float mainChannelScaleX = 1.0;
float mainChannelScaleY = 1.0;
float overlayChannelScaleX = 1.0;
float overlayChannelScaleY = 1.0;

uint32 LUT16[256][256];

ShaderProgram *shaderProgram;

static const GLubyte transparencyTexture[] = {0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF};
static GLubyte borderTexture[] = {0x10,0x80,0x80,0x00,0x10,0x80,0x80,0x00,0x10,0x80,0x80,0x00,0x10,0x80,0x80,0x00};
static const uint32 pixTypeToPixWidth[] = { 16,2,2,2,4,4,8 };

void InitializeColorSpaceTables(void)
{
  for(uint32 i = 0; i < 256; i++)
    vdgCLUT[i] = 0;

  for (uint32 i = 0; i < 256; ++i)
    for (uint32 i2 = 0; i2 < 256; ++i2)
    {
      const uint8 Y = (uint8)i & 0xFC;
      const uint8 CR = ((uint8)i << 6) | (((uint8)i2 >> 2) & 0x38);
      const uint8 CB = (uint8)i2 << 3;
      // Alpha can differ, so leave out!
      LUT16[i][i2] = ((uint32)CB << 16) | ((uint32)CR << 8) | (uint32)Y;
    }
}

uint32 *AllocateTextureMemory32(uint32 size, const bool bOverlay)
{
  uint32 *ptr;

  if(bOverlay)
  {
    ptr = overlayChannelBuffer;

    if(!overlayChannelBuffer)
    {
      ptr = new uint32[size];
      overlayChannelBuffer = ptr;
    }
  }
  else
  {
    ptr = mainChannelBuffer;

    if(!mainChannelBuffer)
    {
      ptr = new uint32[size];
      mainChannelBuffer = ptr;
    }
  }

  return ptr;
}

void FreeTextureMemory(uint32 *ptr, const bool bOverlay)
{
  if(bOverlay)
  {
    if(overlayChannelBuffer)
    {
      delete [] overlayChannelBuffer;
      overlayChannelBuffer = 0;
    }
  }
  else
  {
    if(mainChannelBuffer)
    {
      delete [] mainChannelBuffer;
      mainChannelBuffer = 0;
    }
  }
}

void UpdateTextureStates(void)
{
  GLint uniformLoc;
  const GLint filterType = bUseBilinearFiltering ? GL_LINEAR : GL_NEAREST;

  if(!bShadersInstalled)
  {
    shaderProgram = new ShaderProgram;

    shaderProgram->Initialize();
    shaderProgram->InstallShaderSourceFromFile("video_generic.vs",GL_VERTEX_SHADER);
    shaderProgram->InstallShaderSourceFromFile("video_m32_o32.fs",GL_FRAGMENT_SHADER);
    shaderProgram->AttachShader(GL_VERTEX_SHADER);
    shaderProgram->AttachShader(GL_FRAGMENT_SHADER);
    bool status = shaderProgram->CompileAndLinkShaders();
    bShadersInstalled = status;
    if(status)
    {
      status = shaderProgram->StartShaderProgram();
      if(status)
      {
        uniformLoc = glGetUniformLocation(shaderProgram->GetProgramObject(),"mainChannelSampler");
        glUniform1i(uniformLoc, mainTextureUnit-GL_TEXTURE0);
        uniformLoc = glGetUniformLocation(shaderProgram->GetProgramObject(),"overlayChannelSampler");
        glUniform1i(uniformLoc, osdTextureUnit-GL_TEXTURE0);
        uniformLoc = glGetUniformLocation(shaderProgram->GetProgramObject(),"LUTSampler");
        glUniform1i(uniformLoc, lutTextureUnit-GL_TEXTURE0);
      }
    }
  }

  uint32 pixType = (structOverlayChannel.dmaflags >> 4) & 0x0F;
  uniformLoc = glGetUniformLocation(shaderProgram->GetProgramObject(), "structOverlayChannelAlpha");
  glUniform1f(uniformLoc, (pixType == 2) ? structOverlayChannel.alpha : -1.0f);
  pixType = (structMainChannel.dmaflags >> 4) & 0x0F;
  uniformLoc = glGetUniformLocation(shaderProgram->GetProgramObject(), "mainIs16bit");
  glUniform1f(uniformLoc, (pixType == 2) ? 1.0f : 0.0f);

  glActiveTexture(mainTextureUnit);

  if(bMainChannelActive)
  {
    GLfloat x0, xf, y0, yf;
    x0 = (GLfloat)((-((double)structMainChannel.dest_xoff * (double)structMainChannel.src_width)/((double)structMainChannel.dest_width)) * (1.0 / ALLOCATED_TEXTURE_WIDTH));
    xf = (GLfloat)(((double)(structMainDisplay.dispwidth - structMainChannel.dest_xoff) * (double)structMainChannel.src_width/(double)structMainChannel.dest_width) * (1.0 / ALLOCATED_TEXTURE_WIDTH));
    y0 = (GLfloat)(((double)(structMainDisplay.dispheight - structMainChannel.dest_yoff) * (double)structMainChannel.src_height/(double)structMainChannel.dest_height) * (1.0 / ALLOCATED_TEXTURE_HEIGHT));
    yf = (GLfloat)((-((double)structMainChannel.dest_yoff * (double)structMainChannel.src_height)/((double)structMainChannel.dest_height)) * (1.0 / ALLOCATED_TEXTURE_HEIGHT));

    //uint8 *pBuffer = (uint8 *)mainChannelBuffer;
    //for(uint32 i = 0; i < (ALLOCATED_TEXTURE_WIDTH * ALLOCATED_TEXTURE_HEIGHT); i++)
    //{
    //  pBuffer[0] = videoTexInfo.borderColor[0];
    //  pBuffer[1] = videoTexInfo.borderColor[1];
    //  pBuffer[2] = videoTexInfo.borderColor[2];
    //  pBuffer[3] = videoTexInfo.borderColor[3];
    //  pBuffer += 4;
    //}

    videoTexInfo.mainTexCoords[0] = x0;
    videoTexInfo.mainTexCoords[1] = y0;
    videoTexInfo.mainTexCoords[2] = x0;
    videoTexInfo.mainTexCoords[3] = yf;
    videoTexInfo.mainTexCoords[4] = xf;
    videoTexInfo.mainTexCoords[5] = yf;
    videoTexInfo.mainTexCoords[6] = xf;
    videoTexInfo.mainTexCoords[7] = y0;

    glEnable(TEXTURE_TARGET);
    glBindTexture(TEXTURE_TARGET, videoTexInfo.mainTexName);
  }
  else
  {
    glBindTexture(TEXTURE_TARGET, videoTexInfo.borderTexName);
    glTexImage2D(TEXTURE_TARGET,0,mainInternalTextureFormat32,2,2,0,mainExternalTextureFormat32, mainPixelType32,borderTexture);
    glDisable(TEXTURE_TARGET);
  }

  glTexParameterfv(TEXTURE_TARGET, GL_TEXTURE_BORDER_COLOR, videoTexInfo.borderColor);
  glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, filterType);
  glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, filterType);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glActiveTexture(osdTextureUnit);

  if(bOverlayChannelActive)
  {
    GLfloat x0, xf, y0, yf;
    x0 = (GLfloat)((-((double)structOverlayChannel.dest_xoff * (double)structOverlayChannel.src_width)/((double)structOverlayChannel.dest_width)) * (1.0 / ALLOCATED_TEXTURE_WIDTH));
    xf = (GLfloat)((((double)(structMainDisplay.dispwidth - structOverlayChannel.dest_xoff) * (double)structOverlayChannel.src_width/(double)structOverlayChannel.dest_width)) * (1.0 / ALLOCATED_TEXTURE_WIDTH));
    y0 = (GLfloat)((((double)(structMainDisplay.dispheight - structOverlayChannel.dest_yoff) * (double)structOverlayChannel.src_height)/((double)structOverlayChannel.dest_height)) * (1.0 / ALLOCATED_TEXTURE_HEIGHT));
    yf = (GLfloat)((-((double)structOverlayChannel.dest_yoff * (double)structOverlayChannel.src_height)/((double)structOverlayChannel.dest_height)) * (1.0 / ALLOCATED_TEXTURE_HEIGHT));

    videoTexInfo.osdTexCoords[0] = x0;
    videoTexInfo.osdTexCoords[1] = y0;
    videoTexInfo.osdTexCoords[2] = x0;
    videoTexInfo.osdTexCoords[3] = yf;
    videoTexInfo.osdTexCoords[4] = xf;
    videoTexInfo.osdTexCoords[5] = yf;
    videoTexInfo.osdTexCoords[6] = xf;
    videoTexInfo.osdTexCoords[7] = y0;

    //uint8 *pBuffer = (uint8 *)overlayChannelBuffer;
    //for(uint32 i = 0; i < (ALLOCATED_TEXTURE_WIDTH * ALLOCATED_TEXTURE_HEIGHT); i++)
    //{
    //  pBuffer[0] = videoTexInfo.transColor[0];
    //  pBuffer[1] = videoTexInfo.transColor[1];
    //  pBuffer[2] = videoTexInfo.transColor[2];
    //  pBuffer[3] = videoTexInfo.transColor[3];
    //  pBuffer += 4;
    //}

    glEnable(TEXTURE_TARGET);
    glBindTexture(TEXTURE_TARGET, videoTexInfo.osdTexName);
  }
  else
  {
    glBindTexture(TEXTURE_TARGET, videoTexInfo.transparencyTexName);
    glTexImage2D(TEXTURE_TARGET,0,osdInternalTextureFormat32,2,2,0,osdExternalTextureFormat32, osdPixelType32,transparencyTexture);
    glDisable(TEXTURE_TARGET);
  }

  glTexParameterfv(TEXTURE_TARGET, GL_TEXTURE_BORDER_COLOR, videoTexInfo.transColor);
  glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, filterType);
  glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, filterType);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  glActiveTexture(lutTextureUnit);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, videoTexInfo.LUTTexName);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void UpdateDisplayList(void)
{
  if(!glIsList(videoTexInfo.displayListName[0]))
  {
    videoTexInfo.displayListName[0] = glGenLists(1);
    videoTexInfo.displayListName[1] = glGenLists(1);
    videoTexInfo.displayListName[2] = glGenLists(1);
    videoTexInfo.displayListName[3] = glGenLists(1);
  }

  if(channelState == (CHANNELSTATE_MAIN_ACTIVE|CHANNELSTATE_OVERLAY_ACTIVE))
  {
    glNewList(videoTexInfo.displayListName[CHANNELSTATE_MAIN_ACTIVE|CHANNELSTATE_OVERLAY_ACTIVE],GL_COMPILE);
    glBegin(GL_QUADS);
    glMultiTexCoord2fv(mainTextureUnit, &videoTexInfo.mainTexCoords[0]);
    glMultiTexCoord2fv(osdTextureUnit, &videoTexInfo.osdTexCoords[0]);
    glVertex2f(0.0,0.0);
    glMultiTexCoord2fv(mainTextureUnit, &videoTexInfo.mainTexCoords[2]);
    glMultiTexCoord2fv(osdTextureUnit, &videoTexInfo.osdTexCoords[2]);
    glVertex2f(0.0,1.0);
    glMultiTexCoord2fv(mainTextureUnit, &videoTexInfo.mainTexCoords[4]);
    glMultiTexCoord2fv(osdTextureUnit, &videoTexInfo.osdTexCoords[4]);
    glVertex2f(1.0,1.0);
    glMultiTexCoord2fv(mainTextureUnit, &videoTexInfo.mainTexCoords[6]);
    glMultiTexCoord2fv(osdTextureUnit, &videoTexInfo.osdTexCoords[6]);
    glVertex2f(1.0,0.0);
    glEnd();
    glEndList();
  }
  else if(channelState == CHANNELSTATE_OVERLAY_ACTIVE)
  {
    glNewList(videoTexInfo.displayListName[CHANNELSTATE_OVERLAY_ACTIVE],GL_COMPILE);
    glBegin(GL_QUADS);
    glMultiTexCoord2fv(osdTextureUnit, &videoTexInfo.osdTexCoords[0]);
    glVertex2f(0.0,0.0);
    glMultiTexCoord2fv(osdTextureUnit, &videoTexInfo.osdTexCoords[2]);
    glVertex2f(0.0,1.0);
    glMultiTexCoord2fv(osdTextureUnit, &videoTexInfo.osdTexCoords[4]);
    glVertex2f(1.0,1.0);
    glMultiTexCoord2fv(osdTextureUnit, &videoTexInfo.osdTexCoords[6]);
    glVertex2f(1.0,0.0);
    glEnd();
    glEndList();
  }
  else if(channelState == CHANNELSTATE_MAIN_ACTIVE)
  {
    glNewList(videoTexInfo.displayListName[CHANNELSTATE_MAIN_ACTIVE],GL_COMPILE);
    glBegin(GL_QUADS);
    glMultiTexCoord2fv(mainTextureUnit, &videoTexInfo.mainTexCoords[0]);
    glVertex2f(0.0,0.0);
    glMultiTexCoord2fv(mainTextureUnit, &videoTexInfo.mainTexCoords[2]);
    glVertex2f(0.0,1.0);
    glMultiTexCoord2fv(mainTextureUnit, &videoTexInfo.mainTexCoords[4]);
    glVertex2f(1.0,1.0);
    glMultiTexCoord2fv(mainTextureUnit, &videoTexInfo.mainTexCoords[6]);
    glVertex2f(1.0,0.0);
    glEnd();
    glEndList();
  }
  else if(channelState == 0)
  {
    glNewList(videoTexInfo.displayListName[0],GL_COMPILE);
    glBegin(GL_QUADS);
    glEnd();
    glEndList();
  }
}

void InitTextures(void)
{
  videoTexInfo.borderColor[0] = (GLfloat)(borderTexture[0] / 255.0);
  videoTexInfo.borderColor[1] = (GLfloat)(borderTexture[1] / 255.0);
  videoTexInfo.borderColor[2] = (GLfloat)(borderTexture[2] / 255.0);
  videoTexInfo.borderColor[3] = (GLfloat)(borderTexture[3] / 255.0);
  videoTexInfo.transColor[0] = (GLfloat)(transparencyTexture[0] / 255.0);
  videoTexInfo.transColor[1] = (GLfloat)(transparencyTexture[1] / 255.0);
  videoTexInfo.transColor[2] = (GLfloat)(transparencyTexture[2] / 255.0);
  videoTexInfo.transColor[3] = (GLfloat)(transparencyTexture[3] / 255.0);
  glGenTextures(1,&videoTexInfo.mainTexName);
  glGenTextures(1,&videoTexInfo.osdTexName);
  glGenTextures(1,&videoTexInfo.borderTexName);
  glGenTextures(1,&videoTexInfo.LUTTexName);
  glGenTextures(1,&videoTexInfo.transparencyTexName);
  
  glActiveTexture(mainTextureUnit);
  glBindTexture(TEXTURE_TARGET, videoTexInfo.borderTexName);
  glTexImage2D(TEXTURE_TARGET, 0, mainInternalTextureFormat32, 2, 2, 0, mainExternalTextureFormat32, mainPixelType32, borderTexture);
  glActiveTexture(osdTextureUnit);
  glBindTexture(TEXTURE_TARGET, videoTexInfo.transparencyTexName);
  glTexImage2D(TEXTURE_TARGET, 0, osdExternalTextureFormat32, 2, 2, 0, osdExternalTextureFormat32, osdPixelType32, transparencyTexture);
  glActiveTexture(lutTextureUnit);
  glBindTexture(GL_TEXTURE_2D, videoTexInfo.LUTTexName);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(LUT16[0][0]));

  UpdateTextureStates();

  UpdateDisplayList();
  videoTexInfo.bUpdateDisplayList = false;
}

void IncrementVideoFieldCounter(void)
{
  if(nuonEnv.systemBusDRAM)
  {
    uint32 fieldCounter = *((uint32 *)&nuonEnv.systemBusDRAM[VIDEO_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK]);
    SwapScalarBytes(&fieldCounter);
    fieldCounter++;
    SwapScalarBytes(&fieldCounter);
    *((uint32 *)&nuonEnv.systemBusDRAM[VIDEO_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK]) = fieldCounter;
  }
}

void RenderVideo(const int winwidth, const int winheight)
{
  uint8 *ptrNuonFrameBuffer;
  uint32 activeChannels;
  uint32 pixWidthShift;
  uint32 pixWidth;
  uint32 rowWidth;
  uint8 *pMainChannelBuffer;
  uint8 *pOverlayChannelBuffer;

  if(!bCanDisplayVideo)
    return;

  if(nuonEnv.videoOptions.bAlwaysUpdateVideo)
  {
    nuonEnv.bMainBufferModified = true;
    nuonEnv.bOverlayBufferModified = true;
  }
  else if(!nuonEnv.bMainBufferModified && !nuonEnv.bOverlayBufferModified)
    return;
   
  if(!bTexturesInitialized)
  {
    InitTextures();
    bTexturesInitialized = true;
  }

  if(!bSetupViewport)
  {
    glViewport(0,0,winwidth,winheight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0,1.0,0.0,1.0,-1.0,1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    bSetupViewport = true;
  }

  if(bMainChannelActive)
  {
    const uint32 pixType = (structMainChannel.dmaflags >> 4) & 0x0F;
    const uint32 maxCol = structMainChannel.src_width;
    const uint32 maxRow = structMainChannel.src_height;
    bool useCannedAlpha = false;

    switch(pixType)
    {
      case 0:
        //mpeg
        break;
      case 1:
        //4 bit
        break;
      case 2:
        //16 bit
        pixWidthShift = 1;
        pixWidth = 2;
        useCannedAlpha = true;
        break;
      case 3:
        //8 bit
        pixWidth = 1;
        pixWidthShift = 0;
        break;
      case 4:
        //32 bit
        pixWidthShift = 2;
        pixWidth = 4;
        break;
      case 5:
        //16 bit+16z
        pixWidthShift = 1;
        pixWidth = 2;
        useCannedAlpha = true;
        break;
      case 6:
        //32 bit+32z
        pixWidthShift = 3;
        pixWidth = 8;
        break;
      default:
        pixWidthShift = 2;
        pixWidth = 4;
        break;
    }

    uint32* ptrMainDisplayBuffer = mainChannelBuffer;
    ptrNuonFrameBuffer =
      (uint8 *)(nuonEnv.GetPointerToSystemMemory((uint32)structMainChannel.base + (((structMainChannel.src_yoff * structMainChannel.src_width) + structMainChannel.src_xoff) << pixWidthShift)));
    pMainChannelBuffer = ptrNuonFrameBuffer;

    rowWidth = (structMainChannel.src_width << pixWidthShift);

    if(!nuonEnv.bMainBufferModified
        || pixType == 4 || pixType == 2) // pixel shader does the color conversion?
    {
      goto process_overlay_buffer;
    }

    for(uint32 rowCount = 0; rowCount < maxRow; rowCount++)
    {
      for(uint32 colCount = 0; colCount < maxCol; colCount++)
      {
        switch(pixType)
        {
          case 2:
          case 5:
            //16 or 16+16Z
            //Alpha for main channel is opaque
            *ptrMainDisplayBuffer = (0xFFu << 24) | LUT16[ptrNuonFrameBuffer[0]][ptrNuonFrameBuffer[1]];
            break;
          case 4:
          case 6:
          default:
            //32 or 32+32Z
            const uint8 Y  = ptrNuonFrameBuffer[0];
            const uint8 CR = ptrNuonFrameBuffer[1];
            const uint8 CB = ptrNuonFrameBuffer[2];
            //Alpha for main channel is opaque
            *ptrMainDisplayBuffer = (0xFFu << 24) | ((uint32)CB << 16) | ((uint32)CR << 8) | (uint32)Y;
            break;
        }

        ptrNuonFrameBuffer += pixWidth;
        ptrMainDisplayBuffer++; //Always RGBA 32 bit
      }
    }
  }

process_overlay_buffer:
  if(bOverlayChannelActive)
  {
    const uint32 maxCol = structOverlayChannel.src_width;
          uint32 maxRow = structOverlayChannel.src_height;
    uint32 *ptrMainDisplayBuffer = overlayChannelBuffer;
    const uint32 pixType = (structOverlayChannel.dmaflags >> 4) & 0x0F;

    switch(pixType)
    {
      case 0:
        //mpeg
        break;
      case 1:
        //4 bit
        pixWidthShift = 1;
        pixWidth = 1;
        rowWidth = (structOverlayChannel.src_width >> 1);
        maxRow >>= 1;
        break;
      case 2:
        //16 bit
        pixWidthShift = 1;
        pixWidth = 2;
        rowWidth = (structOverlayChannel.src_width << pixWidthShift);
        break;
      case 3:
        //8 bit
        pixWidthShift = 0;
        pixWidth = 1;
        rowWidth = structOverlayChannel.src_width;
        break;
      case 4:
        //32 bit
        pixWidthShift = 2;
        pixWidth = 4;
        rowWidth = (structOverlayChannel.src_width << pixWidthShift);
        break;
      case 5:
        //16 bit+16z
        pixWidthShift = 1;
        pixWidth = 2;
        rowWidth = (structOverlayChannel.src_width << pixWidthShift);
        break;
      case 6:
        //32 bit+32z
        pixWidthShift = 3;
        pixWidth = 8;
        rowWidth = (structOverlayChannel.src_width << pixWidthShift);
        break;
      default:
        pixWidthShift = 2;
        pixWidth = 4;
        rowWidth = (structOverlayChannel.src_width << pixWidthShift);
        break;
    }

    if(pixType != 1)
    {
      ptrNuonFrameBuffer =
        (uint8 *)(nuonEnv.GetPointerToSystemMemory((uint32)structOverlayChannel.base + (((structOverlayChannel.src_yoff * structOverlayChannel.src_width) + structOverlayChannel.src_xoff) << pixWidthShift)));
    }
    else
    {
      ptrNuonFrameBuffer =
        (uint8 *)(nuonEnv.GetPointerToSystemMemory((uint32)structOverlayChannel.base + (((structOverlayChannel.src_yoff * structOverlayChannel.src_width) + structOverlayChannel.src_xoff) >> pixWidthShift)));
    } 
    pOverlayChannelBuffer = ptrNuonFrameBuffer;

    if(!nuonEnv.bOverlayBufferModified
        || pixType == 4 || pixType == 2) // pixel shader does the color conversion ?
    {
      goto render_main_buffer;
    }

    for(uint32 rowCount = 0; rowCount < maxRow; rowCount++)
    {
      for(uint32 colCount = 0; colCount < maxCol; colCount++)
      {
        uint8 Y, CR, CB, A, Y2, CR2, CB2, A2;
        switch(pixType)
        {
          case 1:
            //4-bit
            clutPtr = (uint8 *)&vdgCLUT[ptrNuonFrameBuffer[0] >> 4];
            Y  = clutPtr[0];
            CR = clutPtr[1];
            CB = clutPtr[2];
            A  = clutPtr[3];
            clutPtr = (uint8 *)&vdgCLUT[ptrNuonFrameBuffer[0] & 0xF];
            Y2  = clutPtr[0];
            CR2 = clutPtr[1];
            CB2 = clutPtr[2];
            A2  = clutPtr[3];
            break;
          case 2:
            //16-bit
            Y  = ptrNuonFrameBuffer[0] & 0xFC;
            CR = (ptrNuonFrameBuffer[0] << 6) | ((ptrNuonFrameBuffer[1] >> 2) & 0x38);
            CB = ptrNuonFrameBuffer[1] << 3;
            A  = structOverlayChannel.alpha;
            break;
          case 3:
            //8-bit
            clutPtr = (uint8 *)&vdgCLUT[ptrNuonFrameBuffer[0]];
            Y  = clutPtr[0];
            CR = clutPtr[1];
            CB = clutPtr[2];
            A  = clutPtr[3];
            break;
          case 4:
            //32-bit
            Y  = ptrNuonFrameBuffer[0];
            CR = ptrNuonFrameBuffer[1];
            CB = ptrNuonFrameBuffer[2];
            A  = ptrNuonFrameBuffer[3];
            break;
          case 5:
            //16+16Z
            Y  = ptrNuonFrameBuffer[0] & 0xFC;
            CR = (ptrNuonFrameBuffer[0] << 6) | ((ptrNuonFrameBuffer[1] >> 2) & 0x38);
            CB = ptrNuonFrameBuffer[1] << 3;
            A  = 0; //Z buffered pixels are always opaque
            break;
          case 6:
            //32+32Z
            Y  = ptrNuonFrameBuffer[0];
            CR = ptrNuonFrameBuffer[1];
            CB = ptrNuonFrameBuffer[2];
            A  = 0; //Z buffered pixels are always opaque
            break;
          default:
            break;
        }

        uint32 pixel = ((uint32)CB << 16) | ((uint32)CR << 8) | (uint32)Y;
        //Color (0,0,0) is always transparent per Nuon architecture document
        if(pixel)
        {
          pixel |= (0xFFu - (uint32)A) << 24;
        }

        *ptrMainDisplayBuffer = pixel;
        ptrMainDisplayBuffer++; //Always RGBA 32 bit

        if(pixType == 1)
        {
          pixel = ((uint32)CB2 << 16) | ((uint32)CR2 << 8) | (uint32)Y2;
          //Color (0,0,0) is always transparent per Nuon architecture document
          if(pixel)
          {
            pixel |= (0xFFu - (uint32)A2) << 24;
          }

          *ptrMainDisplayBuffer = pixel;
          ptrMainDisplayBuffer++; //Always RGBA 32 bit
        }

        ptrNuonFrameBuffer += pixWidth;
      }
    }
  }

render_main_buffer:
  if(bMainChannelActive && nuonEnv.bMainBufferModified)
  {
    glActiveTexture(mainTextureUnit);
    glBindTexture(TEXTURE_TARGET,videoTexInfo.mainTexName);

    const uint32 pixType = (structMainChannel.dmaflags >> 4) & 0x0F;
    const GLint mainInternalTextureFormat = (pixType == 2) ? mainInternalTextureFormat16 : mainInternalTextureFormat32;
    const GLint mainExternalTextureFormat = (pixType == 2) ? mainExternalTextureFormat16 : mainExternalTextureFormat32;
    const GLint mainPixelType = (pixType == 2) ? mainPixelType16 : mainPixelType32;
    const void* const mainPixels = (pixType == 4 || pixType == 2) // pixel shader does color conversion?
        ? (void*)pMainChannelBuffer : (void*)mainChannelBuffer;
    if(bMainTexturePixType != pixType) // format change or never created?
    {
      glTexImage2D(TEXTURE_TARGET,0,mainInternalTextureFormat, ALLOCATED_TEXTURE_WIDTH, ALLOCATED_TEXTURE_HEIGHT, 0,mainExternalTextureFormat,mainPixelType, mainPixels);
      bMainTexturePixType = pixType;
    }
    else
      glTexSubImage2D(TEXTURE_TARGET,0,0,0,structMainChannel.src_width,structMainChannel.src_height,mainExternalTextureFormat,mainPixelType, mainPixels);
  }

  if(bOverlayChannelActive && nuonEnv.bOverlayBufferModified)
  {
    glActiveTexture(osdTextureUnit);
    glBindTexture(TEXTURE_TARGET,videoTexInfo.osdTexName);

    const uint32 pixType = (structOverlayChannel.dmaflags >> 4) & 0x0F;
    const GLint osdInternalTextureFormat = (pixType == 2) ? osdInternalTextureFormat16 : osdInternalTextureFormat32;
    const GLint osdExternalTextureFormat = (pixType == 2) ? osdExternalTextureFormat16 : osdExternalTextureFormat32;
    const GLint osdPixelType = (pixType == 2) ? osdPixelType16 : osdPixelType32;
    const void* const osdPixels = (pixType == 4 || pixType == 2) // pixel shader does color conversion?
        ? (void*)pOverlayChannelBuffer : (void*)overlayChannelBuffer;
    if(bOverlayTexturePixType != pixType) // format change or never created?
    {
      glTexImage2D(TEXTURE_TARGET,0,osdInternalTextureFormat, ALLOCATED_TEXTURE_WIDTH, ALLOCATED_TEXTURE_HEIGHT, 0,osdExternalTextureFormat, osdPixelType, osdPixels);
      bOverlayTexturePixType = pixType;
    }
    else
      glTexSubImage2D(TEXTURE_TARGET,0,0,0,structOverlayChannel.src_width,structOverlayChannel.src_height,osdExternalTextureFormat, osdPixelType, osdPixels);
  }

  if(videoTexInfo.bUpdateDisplayList)
  {
    UpdateDisplayList();
    videoTexInfo.bUpdateDisplayList = false;
  }

  activeChannels = (bOverlayChannelActive ? CHANNELSTATE_OVERLAY_ACTIVE: 0);
  activeChannels |= (bMainChannelActive ? CHANNELSTATE_MAIN_ACTIVE : 0);
 
  glCallList(videoTexInfo.displayListName[activeChannels]);
  glFlush();

  nuonEnv.bMainBufferModified = false;
  nuonEnv.bOverlayBufferModified = false;
}

void UpdateBufferLengths(void)
{
  if(bMainChannelActive)
  {
    nuonEnv.mainChannelLowerLimit = (uint32)structMainChannel.base;
    nuonEnv.mainChannelUpperLimit = nuonEnv.mainChannelLowerLimit + (structMainChannel.src_width * structMainChannel.src_height * pixTypeToPixWidth[(structMainChannel.dmaflags >> 4) & 0x7]) - 1;
  }
  else
  {
    nuonEnv.mainChannelLowerLimit = 0;
    nuonEnv.mainChannelUpperLimit = 0;
  }

  if(bOverlayChannelActive)
  {
    nuonEnv.overlayChannelLowerLimit = (uint32)structOverlayChannel.base;
    nuonEnv.overlayChannelUpperLimit = nuonEnv.overlayChannelLowerLimit + (structOverlayChannel.src_width * structOverlayChannel.src_height * pixTypeToPixWidth[(structOverlayChannel.dmaflags >> 4) & 0x7]) - 1;
  }
  else
  {
    nuonEnv.overlayChannelLowerLimit = 0;
    nuonEnv.overlayChannelUpperLimit = 0;
  }
}

void VidQueryConfig(MPE &mpe)
{
  VidDisplay * const displayStruct = (VidDisplay *)nuonEnv.GetPointerToMemory(mpe,mpe.regs[0]);

  displayStruct->dispwidth = structMainDisplay.dispwidth;
  displayStruct->dispheight = structMainDisplay.dispheight;
  displayStruct->bordcolor = structMainDisplay.bordcolor;
  displayStruct->progressive = structMainDisplay.progressive;
  displayStruct->fps = structMainDisplay.fps;
  displayStruct->pixel_aspect_x = structMainDisplay.pixel_aspect_x;
  displayStruct->pixel_aspect_y = structMainDisplay.pixel_aspect_y;
  displayStruct->screen_aspect_x = structMainDisplay.screen_aspect_x;
  displayStruct->screen_aspect_y = structMainDisplay.screen_aspect_y;

  displayStruct->reserved[0] = structMainDisplay.reserved[0]; // not swapped/not needed
  displayStruct->reserved[1] = structMainDisplay.reserved[1];
  displayStruct->reserved[2] = structMainDisplay.reserved[2];

  SwapScalarBytes((uint32 *)&displayStruct->dispwidth);
  SwapScalarBytes((uint32 *)&displayStruct->dispheight);
  SwapScalarBytes((uint32 *)&displayStruct->bordcolor);
  SwapScalarBytes((uint32 *)&displayStruct->progressive);
  SwapScalarBytes((uint32 *)&displayStruct->fps);
  SwapWordBytes((uint16 *)&displayStruct->pixel_aspect_x);
  SwapWordBytes((uint16 *)&displayStruct->pixel_aspect_y);
  SwapWordBytes((uint16 *)&displayStruct->screen_aspect_x);
  SwapWordBytes((uint16 *)&displayStruct->screen_aspect_y);

  //Return 1 to specify NTSC
  mpe.regs[0] = 1;
}

void VidConfig(MPE &mpe)
{
  const uint32 display = mpe.regs[0];
  const uint32 main = mpe.regs[1];
  const uint32 osd = mpe.regs[2];
  const uint32 reserved = mpe.regs[3];
  VidDisplay maindisplay;
  VidChannel mainchannel, osdchannel;

  bool bUpdateOpenGLData = false;

  channelStatePrev = channelState;
  channelState = 0;

  if(display)
  {
    VidDisplay *const pMainDisplay = (VidDisplay *)nuonEnv.GetPointerToMemory(mpe, display);
    memcpy(&maindisplay,pMainDisplay,sizeof(VidDisplay));
    SwapScalarBytes((uint32 *)&maindisplay.bordcolor);

    maindisplay.dispwidth = VIDEO_WIDTH;
    maindisplay.dispheight = 480;
    maindisplay.fps = 0;
    maindisplay.progressive = -1;
  }
  else
    return; // leave unchanged from last call

  if(main)
  {
    VidChannel * const pMainChannel = (VidChannel *)nuonEnv.GetPointerToMemory(mpe, main);
    memcpy(&mainchannel,pMainChannel,sizeof(VidChannel));
    SwapVectorBytes((uint32 *)&mainchannel.dmaflags);
    SwapVectorBytes((uint32 *)&mainchannel.dest_width);
    SwapScalarBytes((uint32 *)&mainchannel.src_width);
    SwapScalarBytes((uint32 *)&mainchannel.src_height);
    if(mainchannel.base == 0)
    {
      //mpe.regs[0] = 0;
      //return;
      mainchannel.base = structMainChannel.base;
    }
  }

  if(osd)
  {
    VidChannel * const pOSDChannel = (VidChannel *)nuonEnv.GetPointerToMemory(mpe, osd);
    memcpy(&osdchannel,pOSDChannel,sizeof(VidChannel));
    SwapVectorBytes((uint32 *)&osdchannel.dmaflags);
    SwapVectorBytes((uint32 *)&osdchannel.dest_width);
    SwapScalarBytes((uint32 *)&osdchannel.src_width);
    SwapScalarBytes((uint32 *)&osdchannel.src_height);
    if(osdchannel.base == 0)
    {
      //mpe.regs[0] = 0;
      //return;
      osdchannel.base = structOverlayChannel.base;
    }
  }

  if(display)
  {
    memcpy(&structMainDisplayPrev,&structMainDisplay,sizeof(VidDisplay));
    memcpy(&structMainDisplay,&maindisplay,sizeof(VidDisplay));

    borderTexture[0] = ((uint32)structMainDisplay.bordcolor >> 24) & 0xFF;
    borderTexture[1] = (structMainDisplay.bordcolor >> 16) & 0xFF;
    borderTexture[2] = (structMainDisplay.bordcolor >> 8) & 0xFF;
    borderTexture[3] = 0;
    videoTexInfo.borderColor[0] = borderTexture[0];
    videoTexInfo.borderColor[1] = borderTexture[1];
    videoTexInfo.borderColor[2] = borderTexture[2];
    videoTexInfo.borderColor[3] = 0.0;

    bUpdateOpenGLData |= (structMainDisplay.bordcolor != structMainDisplayPrev.bordcolor);
  }

  if(!mainDisplayBuffer)
  {
    mainDisplayBuffer = new uint32[structMainDisplay.dispwidth * structMainDisplay.dispheight];
  }
  bMainChannelActive = false;
  bOverlayChannelActive = false;

  uint32 map;
  if(main)
  {
    memcpy(&structMainChannelPrev,&structMainChannel,sizeof(VidChannel));
    memcpy(&structMainChannel,&mainchannel,sizeof(VidChannel));

    map = (structMainChannel.dmaflags >> 4) & 0x0FUL;

    //Handle 16_16Z double and triple buffer frame buffers
    if((map >= 9) || (map == 5))
    {
      if(map == 5)
      {
        //16+16Z single buffer: map 0 contains 16-bit color map 1 contains 16 bit Z data
        map = 0;
      }
      else if(map < 13)
      {
        //16+16Z triple buffer: map 0,1 and 2 contain 16-bit color data, map 3 contains 16 bit Z data
        map = map - 9;
      }
      else
      {
        //16+16Z double buffer: map 0,1 contain 16-bit color data, map 2 contains 16 bit Z data
        map = map - 13;
      }
      //The specified base address always points to the start of the first color buffer/map.  The pixel type
      //determines which map is to be displayed.  The proper byte offset from the start of the first map can be
      //computed using (map * src_width * src_height * bytes_per_pixel) where bytes_per_pixel is always two.
      structMainChannel.base = ((uint8 *)structMainChannel.base) + (map * structMainChannel.src_width * structMainChannel.src_height * 2);
      //change the pixel type to 16-bit, no-Z
      structMainChannel.dmaflags = (structMainChannel.dmaflags & 0xFFFFFF0FUL) | (0x02 << 4);

      //It is safe to specify a pixel type of 12 or 15 but this will display Z-data and look really funky.
    }
    
    if(structMainChannel.dest_height == 0)
    {
      //Should the dest height default to the display height, or source height?
      //Is it even valid to leave this at zero?  The SDK docs say nothing about
      //this case, but Decaying Orbit sets dest height and dest width to zero!
      structMainChannel.dest_height = structMainDisplay.dispheight;
    }

    if(structMainChannel.dest_width == 0)
    {
      //Should the dest height default to the display width, or source width?
      //Is it even valid to leave this at zero?  The SDK docs say nothing about
      //this case, but Decaying Orbit sets dest height and dest width to zero!
      structMainChannel.dest_width = structMainDisplay.dispwidth;
    }

    //check for -1 in dest_xoff or dest_yoff: if either is -1, compute proper
    //offset needed to center along x and/or y
    if(structMainChannel.dest_xoff == -1)
    {
      structMainChannel.dest_xoff = (structMainDisplay.dispwidth - structMainChannel.dest_width) >> 1;
    }

    if(structMainChannel.dest_yoff == -1)
    {
      structMainChannel.dest_yoff = (structMainDisplay.dispheight - structMainChannel.dest_height) >> 1;
    }

    //mainChannelBuffer = AllocateTextureMemory32(((structMainChannel.src_width & 0xFFFF) * (structMainChannel.src_height & 0xFFFF)),false);
    mainChannelScaleX = (float)((double)structMainChannel.dest_width/(double)structMainChannel.src_width);
    mainChannelScaleY = (float)((double)structMainChannel.dest_height/(double)structMainChannel.src_height);
    bMainChannelActive = true;
    channelState |= CHANNELSTATE_MAIN_ACTIVE;

    bUpdateOpenGLData |= (structMainChannel.alpha != structMainChannelPrev.alpha);
    bUpdateOpenGLData |= (structMainChannel.clut_select != structMainChannelPrev.clut_select);
    bUpdateOpenGLData |= (structMainChannel.dest_height != structMainChannelPrev.dest_height);
    bUpdateOpenGLData |= (structMainChannel.dest_width != structMainChannelPrev.dest_width);
    bUpdateOpenGLData |= (structMainChannel.dest_xoff != structMainChannelPrev.dest_xoff);
    bUpdateOpenGLData |= (structMainChannel.dest_yoff != structMainChannelPrev.dest_yoff);
    bUpdateOpenGLData |= (structMainChannel.dmaflags != structMainChannelPrev.dmaflags);
    bUpdateOpenGLData |= (structMainChannel.hfilter != structMainChannelPrev.hfilter);
    bUpdateOpenGLData |= (structMainChannel.src_width != structMainChannelPrev.src_width);
    bUpdateOpenGLData |= (structMainChannel.src_height != structMainChannelPrev.src_height);
    bUpdateOpenGLData |= (structMainChannel.src_xoff != structMainChannelPrev.src_xoff);
    bUpdateOpenGLData |= (structMainChannel.src_yoff != structMainChannelPrev.src_yoff);
    bUpdateOpenGLData |= (structMainChannel.vfilter != structMainChannelPrev.vfilter);
  }

  if(osd)
  {
    memcpy(&structOverlayChannelPrev,&structOverlayChannel,sizeof(VidChannel));
    memcpy(&structOverlayChannel,&osdchannel,sizeof(VidChannel));

    map = (structOverlayChannel.dmaflags >> 4) & 0x0FUL;

    //Handle 16_16Z double and triple buffer frame buffers
    if((map >= 9) || (map == 5))
    {
      if(map == 5)
      {
        //16+16Z single buffer: map 0 contains 16-bit color map 1 contains 16 bit Z data
        map = 0;
      }
      else if(map < 13)
      {
        //16+16Z triple buffer: map 0,1 and 2 contain 16-bit color data, map 3 contains 16 bit Z data
        map = map - 9;
      }
      else
      {
        //16+16Z double buffer: map 0,1 contain 16-bit color data, map 2 contains 16 bit Z data
        map = map - 13;
      }
      //The specified base address always points to the start of the first color buffer/map.  The pixel type
      //determines which map is to be displayed.  The proper byte offset from the start of the first map can be
      //computed using (map * src_width * src_height * bytes_per_pixel) where bytes_per_pixel is always two.
      structOverlayChannel.base = ((uint8 *)structOverlayChannel.base) + (map * structOverlayChannel.src_width * structOverlayChannel.src_height * 2);
      //change the pixel type to 16-bit, no-Z
      structOverlayChannel.dmaflags = (structOverlayChannel.dmaflags & 0xFFFFFF0FUL) | (0x02 << 4);

      //It is safe to specify a pixel type of 12 or 15 but this will display Z-data and look really funky.
    }

    if(structOverlayChannel.dest_height == 0)
    {
      //Should the dest height default to the display height, or source height?
      //Is it even valid to leave this at zero?  The SDK docs say nothing about
      //this case, but Decaying Orbit sets dest height and dest width to zero!
      structOverlayChannel.dest_height = structMainDisplay.dispheight;
    }

    if(structOverlayChannel.dest_width == 0)
    {
      //Should the dest height default to the display width, or source width?
      //Is it even valid to leave this at zero?  The SDK docs say nothing about
      //this case, but Decaying Orbit sets dest height and dest width to zero!
      structOverlayChannel.dest_width = structMainDisplay.dispwidth;
    }

    //check for -1 in dest_xoff or dest_yoff: if either is -1, compute proper
    //offset needed to center along x and/or y
    if(structOverlayChannel.dest_xoff == -1)
    {
      structOverlayChannel.dest_xoff = (structMainDisplay.dispwidth - structOverlayChannel.dest_width) >> 1;
    }

    if(structOverlayChannel.dest_yoff == -1)
    {
      structOverlayChannel.dest_yoff = (structMainDisplay.dispheight - structOverlayChannel.dest_height) >> 1;
    }

    //overlayChannelBuffer = AllocateTextureMemory32(((structOverlayChannel.src_width & 0xFFFF) * (structOverlayChannel.src_height & 0xFFFF)),true);
    overlayChannelScaleX = (float)((double)structOverlayChannel.dest_width/(double)structOverlayChannel.src_width);
    overlayChannelScaleY = (float)((double)structOverlayChannel.dest_height/(double)structOverlayChannel.src_height);
    bOverlayChannelActive = true;
    channelState |= CHANNELSTATE_OVERLAY_ACTIVE;

    bUpdateOpenGLData |= (structOverlayChannel.alpha != structOverlayChannelPrev.alpha);
    bUpdateOpenGLData |= (structOverlayChannel.clut_select != structOverlayChannelPrev.clut_select);
    bUpdateOpenGLData |= (structOverlayChannel.dest_height != structOverlayChannelPrev.dest_height);
    bUpdateOpenGLData |= (structOverlayChannel.dest_width != structOverlayChannelPrev.dest_width);
    bUpdateOpenGLData |= (structOverlayChannel.dest_xoff != structOverlayChannelPrev.dest_xoff);
    bUpdateOpenGLData |= (structOverlayChannel.dest_yoff != structOverlayChannelPrev.dest_yoff);
    bUpdateOpenGLData |= (structOverlayChannel.dmaflags != structOverlayChannelPrev.dmaflags);
    bUpdateOpenGLData |= (structOverlayChannel.hfilter != structOverlayChannelPrev.hfilter);
    bUpdateOpenGLData |= (structOverlayChannel.src_width != structOverlayChannelPrev.src_width);
    bUpdateOpenGLData |= (structOverlayChannel.src_height != structOverlayChannelPrev.src_height);
    bUpdateOpenGLData |= (structOverlayChannel.src_xoff != structOverlayChannelPrev.src_xoff);
    bUpdateOpenGLData |= (structOverlayChannel.src_yoff != structOverlayChannelPrev.src_yoff);
    bUpdateOpenGLData |= (structOverlayChannel.vfilter != structOverlayChannelPrev.vfilter);
  }

  if(nuonEnv.systemBusDRAM)
  {
    //Set the video config field counter to the current video field counter value

    *((uint32 *)&nuonEnv.systemBusDRAM[LAST_VIDEO_CONFIG_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK])
      = *((uint32 *)&nuonEnv.systemBusDRAM[VIDEO_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK]);
  }

  bCanDisplayVideo = true;
  nuonEnv.bMainBufferModified = true;
  nuonEnv.bOverlayBufferModified = true;
  UpdateBufferLengths();

  if(bUpdateOpenGLData || (channelState != channelStatePrev))
  {
    videoTexInfo.bUpdateDisplayList = true;
    UpdateTextureStates();
  }

  mpe.regs[0] = 1;
}

void VidSetup(MPE &mpe)
{
  uint32 mainChannelBase = mpe.regs[0];
  uint32 dmaFlags = mpe.regs[1];
  uint32 sourceWidth = mpe.regs[2];
  uint32 sourceHeight = mpe.regs[3];
  uint32 filterType = mpe.regs[4];
  uint32 map;
  bool bUpdateOpenGLData = false;

  structMainDisplay.bordcolor = 0x10808000; //Black
  structMainDisplay.dispwidth = VIDEO_WIDTH;
  structMainDisplay.dispheight = 480;
  structMainDisplay.fps = 0;
  structMainDisplay.progressive = 0;

  channelStatePrev = channelState;
  channelState = 0;

  if(mainChannelBase)
  {
    structMainChannelPrev.base = structMainChannel.base;
    structMainChannel.base = (void *)mainChannelBase;
    channelState |= CHANNELSTATE_MAIN_ACTIVE;

    structMainChannelPrev.dmaflags = structMainChannel.dmaflags;
    structMainChannelPrev.src_width = structMainChannel.src_width;
    structMainChannelPrev.src_height = structMainChannel.src_height;
    structMainChannelPrev.clut_select = structMainChannel.clut_select;
    structMainChannelPrev.vfilter = structMainChannel.vfilter;
  }

  if(mainChannelBuffer)
  {
    //FreeTextureMemory(mainChannelBuffer,false);
  }

  //No overlay channel
  if(overlayChannelBuffer)
  {
    //FreeTextureMemory(overlayChannelBuffer,true);
  }

  if(!mainDisplayBuffer)
  {
    mainDisplayBuffer = new uint32[structMainDisplay.dispwidth * structMainDisplay.dispheight];
  }

  structMainChannel.dmaflags = dmaFlags;
  structMainChannel.dest_width = VIDEO_WIDTH;
  structMainChannel.dest_height = 480;
  structMainChannel.src_width = sourceWidth;
  structMainChannel.src_height = sourceHeight;
  structMainChannel.clut_select = 0;
  structMainChannel.alpha = 0;
  structMainChannel.vfilter = filterType;

  //DMA Mode bits for Pixel transfers: 
  //[11:11]: Cluster addressing
  //[9:9]: B backwards flag
  //[8:8]: V Transfer Direction (0 = Horizontal, 1 = Vertical)
  //[7:4]: Pixel type
  //[3:1]: Z comparison type
  //[0:0]: A backwards flag
  //B,V,A and Z are ignored for video DMA transfers

  //Get pixel type
  map = (dmaFlags >> 4) & 0x0FUL;

  //For pixel types 0 through 8, map 0 contains color data and map 1 contains Z data.  Because Z data is ignored
  //by the video DMA controller, transfers will always occur from map 0 so the main channel base address does not
  //need to be adjusted

  //Handle 16_16Z double and triple buffer frame buffers
  if((map >= 9) || (map == 5))
  {
    if(map == 5)
    {
      //16+16Z single buffer: map 0 contains 16-bit color map 1 contains 16 bit Z data
      map = 0;
    }
    else if(map < 13)
    {
      //16+16Z triple buffer: map 0,1 and 2 contain 16-bit color data, map 3 contains 16 bit Z data
      map = map - 9;
    }
    else
    {
      //16+16Z double buffer: map 0,1 contain 16-bit color data, map 2 contains 16 bit Z data
      map = map - 13;
    }
    //The specified base address always points to the start of the first color buffer/map.  The pixel type
    //determines which map is to be displayed.  The proper byte offset from the start of the first map can be
    //computed using (map * src_width * src_height * bytes_per_pixel) where bytes_per_pixel is always two.
    structMainChannel.base = ((uint8 *)structMainChannel.base) + (map * structMainChannel.src_width * structMainChannel.src_height * 2);
    //change the pixel type to 16-bit, no-Z
    structMainChannel.dmaflags = (structMainChannel.dmaflags & 0xFFFFFF0FUL) | (0x02UL << 4);
     //It is safe to specify a pixel type of 12 or 15 but this will display Z-data and look really funky.
  }

  //Center along X
  structMainChannel.dest_xoff = (structMainDisplay.dispwidth - structMainChannel.dest_width) >> 1;
  //Center along Y
  structMainChannel.dest_yoff = (structMainDisplay.dispheight - structMainChannel.dest_height) >> 1;

  //mainChannelBuffer = AllocateTextureMemory32(((structMainChannel.src_width & 0xFFFF) * (structMainChannel.src_height & 0xFFFF)),false);
  mainChannelScaleX = (float)structMainChannel.dest_width/(float)structMainChannel.src_width;
  mainChannelScaleY = (float)structMainChannel.dest_height/(float)structMainChannel.src_height;
  bMainChannelActive = true;
  bOverlayChannelActive = false;

  mpe.regs[0] = 1;
  bCanDisplayVideo = true;

  nuonEnv.bMainBufferModified = true;
  nuonEnv.bOverlayBufferModified = true;
  
  UpdateBufferLengths();

    bUpdateOpenGLData |= (channelState != channelStatePrev);
    bUpdateOpenGLData |= (structMainChannel.alpha != structMainChannelPrev.alpha);
    bUpdateOpenGLData |= (structMainChannel.clut_select != structMainChannelPrev.clut_select);
    bUpdateOpenGLData |= (structMainChannel.dest_height != structMainChannelPrev.dest_height);
    bUpdateOpenGLData |= (structMainChannel.dest_width != structMainChannelPrev.dest_width);
    bUpdateOpenGLData |= (structMainChannel.dest_xoff != structMainChannelPrev.dest_xoff);
    bUpdateOpenGLData |= (structMainChannel.dest_yoff != structMainChannelPrev.dest_yoff);
    bUpdateOpenGLData |= (structMainChannel.dmaflags != structMainChannelPrev.dmaflags);
    bUpdateOpenGLData |= (structMainChannel.hfilter != structMainChannelPrev.hfilter);
    bUpdateOpenGLData |= (structMainChannel.src_width != structMainChannelPrev.src_width);
    bUpdateOpenGLData |= (structMainChannel.src_height != structMainChannelPrev.src_height);
    bUpdateOpenGLData |= (structMainChannel.src_xoff != structMainChannelPrev.src_xoff);
    bUpdateOpenGLData |= (structMainChannel.src_yoff != structMainChannelPrev.src_yoff);
    bUpdateOpenGLData |= (structMainChannel.vfilter != structMainChannelPrev.vfilter);

  if(bUpdateOpenGLData || (channelState != channelStatePrev))
  {
    videoTexInfo.bUpdateDisplayList = true;
    UpdateTextureStates();
  }

  if(nuonEnv.systemBusDRAM)
  {
    //Set the video config field counter to the current video field counter value

    *((uint32 *)&nuonEnv.systemBusDRAM[LAST_VIDEO_CONFIG_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK])
      = *((uint32 *)&nuonEnv.systemBusDRAM[VIDEO_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK]);
  }

  bCanDisplayVideo = true;
}

void SetVideoMode(void)
{
  //always output in RGBA format
  if(!mainDisplayBuffer)
  {
    mainDisplayBuffer = new uint32[VIDEO_WIDTH * 480]; //NUON framebuffer size
  }
}

void VidChangeBase(MPE &mpe)
{
  const int32 which = mpe.regs[0];
  const int32 dmaflags = mpe.regs[1] | (1UL << 13); // always set read bit
  const int32 base = mpe.regs[2];

  mpe.regs[0] = 1;

  if(!base)
  {
    MessageBox(NULL,"VidChangeBase was called with a base parameter of 0","Invalid Video Pointer",MB_OK);
  }

  if(((which == VID_CHANNEL_MAIN) && !mainChannelBuffer) ||
    ((which == VID_CHANNEL_OSD) && !overlayChannelBuffer))
  {
    //Channel is not active, return 0
    mpe.regs[0] = 0;
  }
  else
  {
    uint32 map = (dmaflags >> 4) & 0x0FUL;

    switch(which)
    {
      case VID_CHANNEL_MAIN:
        //valid channel, set dmaflags and base then return 1
        structMainChannel.dmaflags = dmaflags;
        structMainChannel.base = (void *)base;
        //Handle 16_16Z double and triple buffer frame buffers
        if((map >= 9) || map == 5)
        {
          if(map == 5)
          {
            //16+16Z single buffer: map 0 contains 16-bit color map 1 contains 16 bit Z data
            map = 0;
          }
          else if(map < 13)
          {
            //16+16Z triple buffer: map 0,1 and 2 contain 16-bit color data, map 3 contains 16 bit Z data
            map = map - 9;
          }
          else
          {
            //16+16Z double buffer: map 0,1 contain 16-bit color data, map 2 contains 16 bit Z data
            map = map - 13;
          }
          //The specified base address always points to the start of the first color buffer/map.  The pixel type
          //determines which map is to be displayed.  The proper byte offset from the start of the first map can be
          //computed using (map * src_width * src_height * bytes_per_pixel) where bytes_per_pixel is always two.
          structMainChannel.base = ((uint8 *)structMainChannel.base) + (map * structMainChannel.src_width * structMainChannel.src_height * 2);
          //change the pixel type to 16-bit, no-Z
          structMainChannel.dmaflags = (structMainChannel.dmaflags & 0xFFFFFF0FUL) | (0x02UL << 4);

          //It is safe to specify a pixel type of 12 or 15 but this will display Z-data and look really funky.
        }
        UpdateTextureStates();
        //UpdateBufferLengths();
        nuonEnv.bMainBufferModified = true;
        break;
      case VID_CHANNEL_OSD:
        //valid channel, set dmaflags and base then return 1
        structOverlayChannel.dmaflags = dmaflags;
        structOverlayChannel.base = (void *)base;
        //Handle 16_16Z double and triple buffer frame buffers
        if((map >= 9) || (map == 5))
        {
          if(map == 5)
          {
            //16+16Z single buffer: map 0 contains 16-bit color map 1 contains 16 bit Z data
            map = 0;
          }
          else if(map < 13)
          {
            //16+16Z triple buffer: map 0,1 and 2 contain 16-bit color data, map 3 contains 16 bit Z data
            map = map - 9;
          }
          else
          {
            //16+16Z double buffer: map 0,1 contain 16-bit color data, map 2 contains 16 bit Z data
            map = map - 13;
          }
          //The specified base address always points to the start of the first color buffer/map.  The pixel type
          //determines which map is to be displayed.  The proper byte offset from the start of the first map can be
          //computed using (map * src_width * src_height * bytes_per_pixel) where bytes_per_pixel is always two.
          structOverlayChannel.base = ((uint8 *)structOverlayChannel.base) + (map * structOverlayChannel.src_width * structOverlayChannel.src_height * 2);
          //change the pixel type to 16-bit, no-Z
          structOverlayChannel.dmaflags = (structOverlayChannel.dmaflags & 0xFFFFFF0FUL) | (0x02UL << 4);

          //It is safe to specify a pixel type of 12 or 15 but this will display Z-data and look really funky.
        }
        UpdateTextureStates();
        //UpdateBufferLengths();
        nuonEnv.bOverlayBufferModified = true;
        break;
      default:
        //Invalid channel, return 0
        mpe.regs[0] = 0;
        break;
    }
  }

  if(nuonEnv.systemBusDRAM)
  {
    //Set the video config field counter to the current video field counter value
    *((uint32 *)&nuonEnv.systemBusDRAM[LAST_VIDEO_CONFIG_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK])
      = *((uint32 *)&nuonEnv.systemBusDRAM[VIDEO_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK]);
  }
}

void VidChangeScroll(MPE &mpe)
{
  const int32 which = mpe.regs[0];
  const int32 xoff = mpe.regs[1];
  const int32 yoff = mpe.regs[2];

  mpe.regs[0] = 1;

  if(((which == VID_CHANNEL_MAIN) && !mainChannelBuffer) ||
    ((which == VID_CHANNEL_OSD) && !overlayChannelBuffer))
  {
    //Channel is not active, return 0
    mpe.regs[0] = 0;
  }
  else
  {
    switch(which)
    {
      case VID_CHANNEL_MAIN:
        //valid channel, set offsets and return 1
        structMainChannel.src_xoff = xoff;
        structMainChannel.src_yoff = yoff;
        nuonEnv.bMainBufferModified = true;
        videoTexInfo.bUpdateDisplayList = true;
        break;
      case VID_CHANNEL_OSD:
        //valid channel, set offsets and return 1
        structOverlayChannel.src_xoff = xoff;
        structOverlayChannel.src_yoff = yoff;
        nuonEnv.bOverlayBufferModified = true;
        videoTexInfo.bUpdateDisplayList = true;
        break;
      default:
        //Invalid channel, return 0
        mpe.regs[0] = 0;
        break;
    }

    UpdateTextureStates();
  }

  if(nuonEnv.systemBusDRAM)
  {
    //Set the video config field counter to the current video field counter value

    *((uint32 *)&nuonEnv.systemBusDRAM[LAST_VIDEO_CONFIG_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK])
      = *((uint32 *)&nuonEnv.systemBusDRAM[VIDEO_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK]);
  }
}

void SetDefaultColor(MPE &mpe)
{
  structMainDisplay.bordcolor = mpe.regs[0];
  videoTexInfo.bUpdateDisplayList = true;
  UpdateTextureStates();
  nuonEnv.bMainBufferModified = true;
  nuonEnv.bOverlayBufferModified = true;
}

void VidSetCLUTRange(MPE &mpe)
{
  uint32 count = 0;

  const uint32 numColors = mpe.regs[1];
  const uint32 colors = mpe.regs[2];

  if(colors)
  {
    uint32 index = mpe.regs[0];
    const uint32 * const pColors = (uint32 *)nuonEnv.GetPointerToMemory(mpe, colors);

    for(; (index < 256) && (count < numColors); index++, count++)
    {
      vdgCLUT[index] = pColors[count];
      //SwapScalarBytes(&vdgCLUT[index]);
    }
  }

  //Despite the fact that the BIOS documentation shows a function prototype returning void, a disassembly of the BIOS
  //shows that VidSetCLUTRange returns 1 if all entries were copied and 0 otherwise (e.g. the starting index was out
  //of bounds or there were not enough entries in the video clut to accept all of the colors)
  mpe.regs[0] = (count == numColors) ? 1 : 0;

  nuonEnv.bOverlayBufferModified = true;
}

void VideoCleanup(void)
{
  for(uint32 i = 0; i < 4; i++)
  {
    if(glIsList(videoTexInfo.displayListName[i]))
    {
      glDeleteLists(videoTexInfo.displayListName[i],1);
    }
  }
}
