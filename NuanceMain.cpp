#define STRICT

#include <windows.h>
#include "commdlg.h"
#include "external\glew-2.1.0\include\GL\glew.h"
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>
#include "Basetypes.h"
#include "ByteSwap.h"
#include "Comm.h"
#include "CriticalSection.h"
#include "GLWindow.h"
#include "Audio.h"
#include "MPE.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "NuanceRes.h"
#include "Joystick.h"
#include "Video.h"

NuonEnvironment *nuonEnv = NULL;
CriticalSection *csVideoDisplay = NULL;
CriticalSection *csDebugDisplay = NULL;
char **pArgs = 0;
GLWindow *display;
extern ControllerData *controller;
extern uint8 *mainChannelBuffer;
extern uint8 *overlayChannelBuffer;
extern char *dvdBase;
bool bClosingApplication;
bool bQuit = false;
bool bRun;

HICON iconApp;
HBITMAP bmpLEDOn;
HBITMAP bmpLEDOff;
HWND picMPE0LED;
HWND picMPE1LED;
HWND picMPE2LED;
HWND picMPE3LED;
HWND textMPE0;
HWND textMPE1;
HWND textMPE2;
HWND textMPE3;
HWND textMPE0Pcexec;
HWND textMPE1Pcexec;
HWND textMPE2Pcexec;
HWND textMPE3Pcexec;
HWND cbLoadFile;
HWND cbSingleStep;
HWND cbRun;
HWND cbStop;
HWND cbReset;
HWND reTermDisplay;

HWND cbCommStatus;
HWND cbDisplayStatus;
HWND cbMPEStatus;
HWND cbDumpMPEs;
HWND reStatus;
OPENFILENAME ofn;
char openFileName[512];
unsigned long whichController = 1;
unsigned long disassemblyMPE = 3;
unsigned long whichStatus = 0;

extern VidChannel structMainChannel, structOverlayChannel;
extern bool bOverlayChannelActive, bMainChannelActive;
extern vidTexInfo videoTexInfo;

bool GetMPERunStatus(uint32 which)
{
  return (nuonEnv->mpe[which & 0x03]->mpectl & MPECTRL_MPEGO) != 0;
}

void SetMPERunStatus(uint32 which, bool bRun)
{
  if(bRun)
  {
    nuonEnv->mpe[which & 0x03]->mpectl |= MPECTRL_MPEGO;
  }
  else
  {
    nuonEnv->mpe[which & 0x03]->mpectl &= ~MPECTRL_MPEGO;
  }
}

void UpdateStatusWindowDisplay()
{
  static char buf[1024];

  if(whichStatus == 0)
  {
    sprintf(buf,"Pending Comm Requests = %lu\n",nuonEnv->pendingCommRequests);
    SendMessage(reStatus,WM_SETTEXT,NULL,LPARAM(buf));
    sprintf(buf,"MPE0 commctl = $%8.8lx, commxmit0 = $%lx, commrecv0 = $%lx, comminfo = $%lx\n",nuonEnv->mpe[0]->commctl,nuonEnv->mpe[0]->commxmit0,nuonEnv->mpe[0]->commrecv0,nuonEnv->mpe[0]->comminfo);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"MPE1 commctl = $%8.8lx, commxmit0 = $%lx, commrecv0 = $%lx, comminfo = $%lx\n",nuonEnv->mpe[1]->commctl,nuonEnv->mpe[1]->commxmit0,nuonEnv->mpe[1]->commrecv0,nuonEnv->mpe[1]->comminfo);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"MPE2 commctl = $%8.8lx, commxmit0 = $%lx, commrecv0 = $%lx, comminfo = $%lx\n",nuonEnv->mpe[2]->commctl,nuonEnv->mpe[2]->commxmit0,nuonEnv->mpe[2]->commrecv0,nuonEnv->mpe[2]->comminfo);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"MPE3 commctl = $%8.8lx, commxmit0 = $%lx, commrecv0 = $%lx, comminfo = $%lx\n",nuonEnv->mpe[3]->commctl,nuonEnv->mpe[3]->commxmit0,nuonEnv->mpe[3]->commrecv0,nuonEnv->mpe[3]->comminfo);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
  }
  else if(whichStatus == 1)
  {
    sprintf(buf,"Main Channel = %s : Overlay Channel = %s\n",bMainChannelActive ? "ON" : "OFF",bOverlayChannelActive ? "ON" : "OFF");
    SendMessage(reStatus,WM_SETTEXT,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"Main Channel texture coordinates = (%.3f %.3f), (%.3f %.3f), (%.3f %.3f), (%.3f %.3f)\n",
      videoTexInfo.mainTexCoords[0],videoTexInfo.mainTexCoords[1],
      videoTexInfo.mainTexCoords[2],videoTexInfo.mainTexCoords[3],
      videoTexInfo.mainTexCoords[4],videoTexInfo.mainTexCoords[5],
      videoTexInfo.mainTexCoords[6],videoTexInfo.mainTexCoords[7]);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Overlay Channel texture coordinates = (%.3f %.3f), (%.3f %.3f), (%.3f %.3f), (%.3f %.3f)\n",
      videoTexInfo.osdTexCoords[0],videoTexInfo.osdTexCoords[1],
      videoTexInfo.osdTexCoords[2],videoTexInfo.osdTexCoords[3],
      videoTexInfo.osdTexCoords[4],videoTexInfo.osdTexCoords[5],
      videoTexInfo.osdTexCoords[6],videoTexInfo.osdTexCoords[7]);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Main Channel pixel type = %lu, Overlay Channel pixel type = %lu\n",(structMainChannel.dmaflags >> 4) & 0xF,(structOverlayChannel.dmaflags >> 4) & 0xF);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"Main Channel base = $%8.8lx, Overlay Channel base = $%8.8lx\n",(structMainChannel.base),(structOverlayChannel.base));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Main Channel src_width = %lu, Main Channel src_height = %lu\n",(structMainChannel.src_width),(structMainChannel.src_height));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Main Channel dest_width = %lu, Main Channel dest_height = %lu\n",(structMainChannel.dest_width),(structMainChannel.dest_height));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Main Channel src_xoff = %lu, Main Channel src_yoff = %lu\n",(structMainChannel.src_xoff),(structMainChannel.src_yoff));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Main Channel dest_xoff = %lu, Main Channel dest_yoff = %lu\n",(structMainChannel.dest_xoff),(structMainChannel.dest_yoff));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Overlay Channel src_width = %lu, Overlay Channel src_height = %lu\n",(structOverlayChannel.src_width),(structOverlayChannel.src_height));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Overlay Channel dest_width = %lu, Overlay Channel dest_height = %lu\n",(structOverlayChannel.dest_width),(structOverlayChannel.dest_height));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Overlay Channel src_xoff = %lu, Overlay Channel src_yoff = %lu\n",(structOverlayChannel.src_xoff),(structOverlayChannel.src_yoff));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Overlay Channel dest_xoff = %lu, Overlay Channel dest_yoff = %lu\n",(structOverlayChannel.dest_xoff),(structOverlayChannel.dest_yoff));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
  }
  else
  {
    sprintf(buf,"Interpreter cache flushes: (%d, %d, %d, %d)\n",
      nuonEnv->mpe[0]->numInterpreterCacheFlushes,
      nuonEnv->mpe[1]->numInterpreterCacheFlushes,
      nuonEnv->mpe[2]->numInterpreterCacheFlushes,
      nuonEnv->mpe[3]->numInterpreterCacheFlushes);
    SendMessage(reStatus,WM_SETTEXT,NULL,LPARAM(buf));
    sprintf(buf,"Native code cache flushes: (%d, %d, %d, %d)\n",
      nuonEnv->mpe[0]->numNativeCodeCacheFlushes,
      nuonEnv->mpe[1]->numNativeCodeCacheFlushes,
      nuonEnv->mpe[2]->numNativeCodeCacheFlushes,
      nuonEnv->mpe[3]->numNativeCodeCacheFlushes);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Non-compilable packets: (%d, %d, %d, %d)\n",
      nuonEnv->mpe[0]->numNonCompilablePackets,
      nuonEnv->mpe[1]->numNonCompilablePackets,
      nuonEnv->mpe[2]->numNonCompilablePackets,
      nuonEnv->mpe[3]->numNonCompilablePackets);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"Overlays in use: (%d, %d, %d, %d)\n",
      nuonEnv->mpe[0]->overlayManager->GetOverlaysInUse(),
      nuonEnv->mpe[1]->overlayManager->GetOverlaysInUse(),
      nuonEnv->mpe[2]->overlayManager->GetOverlaysInUse(),
      nuonEnv->mpe[3]->overlayManager->GetOverlaysInUse());     
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"MPE3 invec1 = $%8.8lx, intvec2 = $%8.8lx\n",nuonEnv->mpe[3]->intvec1,nuonEnv->mpe[3]->intvec2);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"MPE3 intctl = $%8.8lx, inten1 = $%8.8lx, inten2sel = $%8.8lx\n",nuonEnv->mpe[3]->intctl,nuonEnv->mpe[3]->inten1,nuonEnv->mpe[3]->inten2sel);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"MPE3 intsrc = $%8.8lx, excepsrc = $%8.8lx\n",nuonEnv->mpe[3]->intsrc,nuonEnv->mpe[3]->excepsrc);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"MPE2 invec1 = $%8.8lx, intvec2 = $%8.8lx\n",nuonEnv->mpe[2]->intvec1,nuonEnv->mpe[2]->intvec2);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"MPE2 intctl = $%8.8lx, inten1 = $%8.8lx, inten2sel = $%8.8lx\n",nuonEnv->mpe[2]->intctl,nuonEnv->mpe[2]->inten1,nuonEnv->mpe[2]->inten2sel);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"MPE2 intsrc = $%8.8lx, excepsrc = $%8.8lx\n",nuonEnv->mpe[2]->intsrc,nuonEnv->mpe[2]->excepsrc);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"MPE1 invec1 = $%8.8lx, intvec2 = $%8.8lx\n",nuonEnv->mpe[1]->intvec1,nuonEnv->mpe[1]->intvec2);
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"MPE1 intctl = $%8.8lx, inten1 = $%8.8lx, inten2sel = $%8.8lx\n",nuonEnv->mpe[1]->intctl,nuonEnv->mpe[1]->inten1,nuonEnv->mpe[1]->inten2sel);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"MPE1 intsrc = $%8.8lx, excepsrc = $%8.8lx\n",nuonEnv->mpe[1]->intsrc,nuonEnv->mpe[1]->excepsrc);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"MPE0 invec1 = $%8.8lx, intvec2 = $%8.8lx\n",nuonEnv->mpe[0]->intvec1,nuonEnv->mpe[0]->intvec2);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"MPE0 intctl = $%8.8lx, inten1 = $%8.8lx, inten2sel = $%8.8lx\n",nuonEnv->mpe[0]->intctl,nuonEnv->mpe[0]->inten1,nuonEnv->mpe[0]->inten2sel);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"MPE0 intsrc = $%8.8lx, excepsrc = $%8.8lx\n",nuonEnv->mpe[0]->intsrc,nuonEnv->mpe[0]->excepsrc);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"Stack pointers = [$%8.8lx, $%8.8lx, $%8.8lx, $%8.8lx]\n",nuonEnv->mpe[0]->sp,nuonEnv->mpe[1]->sp,nuonEnv->mpe[2]->sp,nuonEnv->mpe[3]->sp);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    sprintf(buf,"v0: = ($%8.8lx, $%8.8lx, $%8.8lx, $%8.8lx}\n",nuonEnv->mpe[disassemblyMPE]->regs[0],nuonEnv->mpe[disassemblyMPE]->regs[1],nuonEnv->mpe[disassemblyMPE]->regs[2],nuonEnv->mpe[disassemblyMPE]->regs[3]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"v1: = ($%8.8lx, $%8.8lx, $%8.8lx, $%8.8lx}\n",nuonEnv->mpe[disassemblyMPE]->regs[4],nuonEnv->mpe[disassemblyMPE]->regs[5],nuonEnv->mpe[disassemblyMPE]->regs[6],nuonEnv->mpe[disassemblyMPE]->regs[7]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"v2: = ($%8.8lx, $%8.8lx, $%8.8lx, $%8.8lx}\n",nuonEnv->mpe[disassemblyMPE]->regs[8],nuonEnv->mpe[disassemblyMPE]->regs[9],nuonEnv->mpe[disassemblyMPE]->regs[10],nuonEnv->mpe[disassemblyMPE]->regs[11]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"v7: = ($%8.8lx, $%8.8lx, $%8.8lx, $%8.8lx}\n",nuonEnv->mpe[disassemblyMPE]->regs[28],nuonEnv->mpe[disassemblyMPE]->regs[29],nuonEnv->mpe[disassemblyMPE]->regs[30],nuonEnv->mpe[disassemblyMPE]->regs[31]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"rx/ry/ru/rv: = ($%8.8lx, $%8.8lx, $%8.8lx, $%8.8lx}\n",nuonEnv->mpe[disassemblyMPE]->rx,nuonEnv->mpe[disassemblyMPE]->ry,nuonEnv->mpe[disassemblyMPE]->ru,nuonEnv->mpe[disassemblyMPE]->rv);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf(buf,"rz/rzi1/rzi2: = ($%8.8lx, $%8.8lx, $%8.8lx}\n",nuonEnv->mpe[disassemblyMPE]->rz,nuonEnv->mpe[disassemblyMPE]->rzi1,nuonEnv->mpe[disassemblyMPE]->rzi2);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));

  }
}

void UpdateControlPanelDisplay()
{
  HWND ledHandles[] = {picMPE0LED,picMPE1LED,picMPE2LED,picMPE3LED};
  HWND pcexecHandles[] = {textMPE0Pcexec,textMPE1Pcexec,textMPE2Pcexec,textMPE3Pcexec};

  uint32 i;
  char addressStr[10];
  static char buf[1024];
  
  for(i = 0; i < 4; i++)
  {
    if(GetMPERunStatus(i))
    {
      SendMessage(ledHandles[i],STM_SETIMAGE,IMAGE_BITMAP,LPARAM(bmpLEDOn));
    }
    else
    {
      SendMessage(ledHandles[i],STM_SETIMAGE,IMAGE_BITMAP,LPARAM(bmpLEDOff));
    }
    sprintf(addressStr,"$%8.8lX",nuonEnv->mpe[i]->pcexec);
    SendMessage(pcexecHandles[i],WM_SETTEXT,0,LPARAM(addressStr));
  }


  sprintf(buf,"mpe%lu: $%8.8X\n{\n", disassemblyMPE, nuonEnv->mpe[disassemblyMPE]->pcexec);
  SendMessage(reTermDisplay,WM_SETTEXT,NULL,LPARAM(buf));
  SendMessage(reTermDisplay,EM_SETSEL,WPARAM(-1),LPARAM(-1));
  nuonEnv->mpe[disassemblyMPE]->PrintInstructionCachePacket(buf,nuonEnv->mpe[disassemblyMPE]->pcexec);
  SendMessage(reTermDisplay,EM_REPLACESEL,NULL,LPARAM(buf));
  sprintf(buf,"}\n");
  SendMessage(reTermDisplay,EM_REPLACESEL,NULL,LPARAM(buf));
}

void NuanceStart(int argc,char *argv[]);

char displayWindowTitle[] = "Nuance Video Display";

void OnMPELEDDoubleClick(uint32 which)
{
  HWND handles[] = {picMPE0LED,picMPE1LED,picMPE2LED,picMPE3LED};

  which = which & 0x03;
  if(GetMPERunStatus(which))
  {
    SetMPERunStatus(which,false);
    SendMessage(handles[which],STM_SETIMAGE,IMAGE_BITMAP,LPARAM(bmpLEDOff));
  }
  else
  {
    SetMPERunStatus(which,true);
    SendMessage(handles[which],STM_SETIMAGE,IMAGE_BITMAP,LPARAM(bmpLEDOn));
  }
}

void ExecuteSingleStep()
{
  nuonEnv->mpe[3]->ExecuteSingleStep();
  nuonEnv->mpe[2]->ExecuteSingleStep();
  nuonEnv->mpe[1]->ExecuteSingleStep();
  nuonEnv->mpe[0]->ExecuteSingleStep();
  if(nuonEnv->pendingCommRequests)
  {
    DoCommBusController();
  }
}

BOOL CALLBACK StatusWindowDialogProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
  FILE *outFile;

  switch(msg)
  {
    case WM_CLOSE:
      bQuit = true;
      return FALSE;
    case WM_COMMAND:
      switch(HIWORD(wParam))
      {
        case BN_CLICKED:
          if((HWND)lParam == cbCommStatus)
          {
            whichStatus = 0;
            UpdateStatusWindowDisplay();
            return TRUE;
          }
          else if((HWND)lParam == cbDisplayStatus)
          {
            whichStatus = 1;
            UpdateStatusWindowDisplay();
            return TRUE;
          }
          else if((HWND)lParam == cbMPEStatus)
          {
            whichStatus = 2;
            UpdateStatusWindowDisplay();
            return TRUE;
          }
          else if((HWND)lParam == cbDumpMPEs)
          {
            outFile = fopen("mpe0.bin","wb");
            if(outFile)
            {
              fwrite(nuonEnv->mpe[0]->dtrom,sizeof(uint8),MPE_LOCAL_MEMORY_SIZE,outFile);
              fclose(outFile);
            }
            outFile = fopen("mpe1.bin","wb");
            if(outFile)
            {
              fwrite(nuonEnv->mpe[1]->dtrom,sizeof(uint8),MPE_LOCAL_MEMORY_SIZE,outFile);
              fclose(outFile);
            }
            outFile = fopen("mpe2.bin","wb");
            if(outFile)
            {
              fwrite(nuonEnv->mpe[2]->dtrom,sizeof(uint8),MPE_LOCAL_MEMORY_SIZE,outFile);
              fclose(outFile);
            }
            outFile = fopen("mpe3.bin","wb");
            if(outFile)
            {
              fwrite(nuonEnv->mpe[3]->dtrom,sizeof(uint8),MPE_LOCAL_MEMORY_SIZE,outFile);
              fclose(outFile);
            }
            return TRUE;
          }
        default:
          return FALSE;
      }
    default:
      return FALSE;
  }
}

BOOL CALLBACK ControlPanelDialogProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
  uint32 i;
  bool bSuccess;

  switch(msg)
  {
    case WM_CLOSE:
      bQuit = true;
      return FALSE;
    case WM_COMMAND:
      switch(HIWORD(wParam))
      {
        case BN_CLICKED:
          if((HWND)lParam == cbRun)
          {
Run:
            uint32 bpAddr = 0;

            FILE *inFile;
            inFile = fopen("breakpoint.txt","r");

            if(inFile)
            {
              fscanf(inFile,"%x",&bpAddr);
              fclose(inFile);
            }

            nuonEnv->mpe[0]->breakpointAddress = bpAddr;
            nuonEnv->mpe[1]->breakpointAddress = bpAddr;
            nuonEnv->mpe[2]->breakpointAddress = bpAddr;
            nuonEnv->mpe[3]->breakpointAddress = bpAddr;

            EnableWindow(cbLoadFile,FALSE);
            EnableWindow(cbRun,FALSE);
            EnableWindow(cbSingleStep,FALSE);
            EnableWindow(cbStop,TRUE);
            bRun = true;
            return TRUE;
          }
          else if((HWND)lParam == cbSingleStep)
          {
            EnableWindow(cbRun,FALSE);
            EnableWindow(cbSingleStep,FALSE);
            EnableWindow(cbStop,TRUE);
            ExecuteSingleStep();
            UpdateControlPanelDisplay();
            EnableWindow(cbRun,TRUE);
            EnableWindow(cbSingleStep,TRUE);
            EnableWindow(cbStop,FALSE);
            return TRUE;
          }
          else if((HWND)lParam == cbStop)
          {
            EnableWindow(cbStop,FALSE);
            EnableWindow(cbSingleStep,TRUE);
            EnableWindow(cbLoadFile,FALSE);
            EnableWindow(cbRun,TRUE);
            bRun = false;
            UpdateControlPanelDisplay();
            return TRUE;
          }
          else if((HWND)lParam == cbReset)
          {
            for(i = 0; i < 4; i++)
            {
              nuonEnv->mpe[i]->Reset();
            }
            return TRUE;
          }
          else if((HWND)lParam == cbLoadFile)
          {
            if(GetOpenFileName(&ofn))
            {
              bSuccess = nuonEnv->mpe[3]->LoadNuonRomFile(ofn.lpstrFile);
              if(!bSuccess)
              {
                bSuccess = nuonEnv->mpe[3]->LoadCoffFile(ofn.lpstrFile);
                if(!bSuccess)
                {
                  MessageBox(NULL,"Invalid COFF or NUONROM-DISK file",ERROR,MB_ICONWARNING);
                }
              }
              
              if(bSuccess)
              {
                nuonEnv->SetDVDBaseFromFileName(ofn.lpstrFile);
                nuonEnv->mpe[3]->Go();
                UpdateControlPanelDisplay();
                goto Run;
              }
            }
            return TRUE;
          }
          else if((HWND)lParam == textMPE0)
          {
            disassemblyMPE = 0;
            UpdateControlPanelDisplay();
          }
          else if((HWND)lParam == textMPE1)
          {
            disassemblyMPE = 1;
            UpdateControlPanelDisplay();
          }
          else if((HWND)lParam == textMPE2)
          {
            disassemblyMPE = 2;
            UpdateControlPanelDisplay();
          }
          else if((HWND)lParam == textMPE3)
          {
            disassemblyMPE = 3;
            UpdateControlPanelDisplay();
          }
          return TRUE;
        case STN_DBLCLK:
        {
          if((HWND)lParam == picMPE0LED)
          {
            OnMPELEDDoubleClick(0);
            return TRUE;
          }
          else if((HWND)lParam == picMPE1LED)
          {
            OnMPELEDDoubleClick(1);
            return TRUE;
          }
          else if((HWND)lParam == picMPE2LED)
          {
            OnMPELEDDoubleClick(2);
            return TRUE;
          }
          else if((HWND)lParam == picMPE3LED)
          {
            OnMPELEDDoubleClick(3);
            return TRUE;
          }
        }
      }
    default:
      return FALSE;
  }
}

BOOL CALLBACK SplashScreenDialogProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
  switch(msg)
  {
    case WM_CTLCOLORDLG:
      return INT_PTR(GetStockObject(BLACK_BRUSH));
    default:
      return FALSE;
  }
}

bool OnDisplayPaint(WPARAM wparam, LPARAM lparam)
{
  if(bRun)
  {
    nuonEnv->bMainBufferModified = true;
    nuonEnv->bOverlayBufferModified = true;
    RenderVideo(display->clientWidth,display->clientHeight);
  }
  else
  {
    glClear(GL_COLOR_BUFFER_BIT);
  }
  return true;
}

bool OnDisplayResize(uint16 width, uint16 height)
{
  glViewport(0,0,width,height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0,1.0,0.0,1.0,-1.0,1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_FOG);

  glClear(GL_COLOR_BUFFER_BIT);
  OnDisplayPaint(0,0);
  return false;
}

void OnDisplayTimer(uint32 idEvent)
{
  InvalidateRect(display->hWnd,NULL,FALSE);
  UpdateWindow(display->hWnd);
  nuonEnv->TriggerVideoInterrupt();
}

bool OnDisplayKeyDown(int16 vkey, uint32 keydata)
{
  uint16 buttons = 0;

  switch(vkey)
  {
    case 'A':
      //Start
      buttons = CTRLR_BUTTON_START;
      break;
    case 'S':
      //NUON (Z)
      buttons = CTRLR_BUTTON_NUON;
      break;
    case 'D':
      //A
      buttons = CTRLR_BUTTON_A;
      break;
    case 'F':
      //B
      buttons = CTRLR_BUTTON_B;
      break;
    case VK_UP:
      //DPad Up
      buttons = CTRLR_DPAD_UP;
      break;
    case VK_DOWN:
      //DPad Down
      buttons = CTRLR_DPAD_DOWN;
      break;
    case VK_LEFT:
      //DPad Left
      buttons = CTRLR_DPAD_LEFT;
      break;
    case VK_RIGHT:
      //DPad Right
      buttons = CTRLR_DPAD_RIGHT;
      break;
    case 'Q':
      //LShoulder
      buttons = CTRLR_BUTTON_L;
      break;
    case 'T':
      //RShoulder
      buttons = CTRLR_BUTTON_R;
      break;
    case 'W':
      //C left
      buttons = CTRLR_BUTTON_C_LEFT;
      break;
    case 'E':
      //C down
      buttons = CTRLR_BUTTON_C_DOWN;
      break;
    case 'R':
      //C right
      buttons = CTRLR_BUTTON_C_RIGHT;
      break;
    case '3':
      //C up
      buttons = CTRLR_BUTTON_C_UP;
      break;
  }
  SwapWordBytes(&buttons);
  if(controller)
  {
    controller[whichController].buttons |= buttons;
  }

  return false;
}

bool OnDisplayKeyUp(int16 vkey, uint32 keydata)
{
  uint16 buttons = 0;

  switch(vkey)
  {
    case 'A':
      //Start
      buttons = ~CTRLR_BUTTON_START;
      break;
    case 'S':
      //NUON (Z)
      buttons = ~CTRLR_BUTTON_NUON;
      break;
    case 'D':
      //A
      buttons = ~CTRLR_BUTTON_A;
      break;
    case 'F':
      //B
      buttons = ~CTRLR_BUTTON_B;
      break;
    case VK_UP:
      //DPad Up
      buttons = ~CTRLR_DPAD_UP;
      break;
    case VK_DOWN:
      //DPad Down
      buttons = ~CTRLR_DPAD_DOWN;
      break;
    case VK_LEFT:
      //DPad Left
      buttons = ~CTRLR_DPAD_LEFT;
      break;
    case VK_RIGHT:
      //DPad Right
      buttons = ~CTRLR_DPAD_RIGHT;
      break;
    case 'Q':
      //LShoulder
      buttons = ~CTRLR_BUTTON_L;
      break;
    case 'T':
      //RShoulder
      buttons = ~CTRLR_BUTTON_R;
      break;
    case 'W':
      //C left
      buttons = ~CTRLR_BUTTON_C_LEFT;
      break;
    case 'E':
      //C down
      buttons = ~CTRLR_BUTTON_C_DOWN;
      break;
    case 'R':
      //C right
      buttons = ~CTRLR_BUTTON_C_RIGHT;
      break;
    case '3':
      //C up
      buttons = ~CTRLR_BUTTON_C_UP;
      break;
    case 'Z':
      //Toggle between IR controller and Controller[1]
      whichController = 1 - whichController;
      break;
  }
  SwapWordBytes(&buttons);
  if(controller)
  {
    controller[whichController].buttons &= buttons;
  }
  return false;
}

inline void ProcessCycleBasedEvents(void)
{
  nuonEnv->audioInterruptCycleCount--;
  if(nuonEnv->audioInterruptCycleCount == 0)
  {
    if(nuonEnv->IsAudioSampleInterruptEnabled())
    {
      nuonEnv->TriggerAudioInterrupt();
    }
    else if(nuonEnv->whichAudioInterrupt == 0)
    {
      if(nuonEnv->IsAudioHalfInterruptEnabled())
      {
        nuonEnv->TriggerAudioInterrupt();
      }
    }
    else
    {
      if(nuonEnv->IsAudioWrapInterruptEnabled())
      {
        nuonEnv->TriggerAudioInterrupt();
      }
    }
    nuonEnv->audioInterruptCycleCount = nuonEnv->cyclesPerAudioInterrupt;
    nuonEnv->whichAudioInterrupt = 1 - nuonEnv->whichAudioInterrupt;
  }

  //nuonEnv->videoDisplayCycleCount--;
  //if(nuonEnv->videoDisplayCycleCount == 0 && nuonEnv->bUseCycleBasedTiming)
  //{
    //SendMessage(videoDisplayWindow.hWnd,WM_TIMER,16,NULL);
    //RenderVideo(display->clientWidth,display->clientHeight);
 //   InvalidateRect(display->hWnd,NULL,FALSE);
 //   UpdateWindow(display->hWnd);
 //   nuonEnv->videoDisplayCycleCount = nuonEnv->cyclesPerVideoDisplay;
 // }
}

bool CheckForInvalidCommStatus(MPE *mpe)
{
  static bool bInvalid;
  static bool bInvalid2;

  bInvalid = false;
  bInvalid2 = false;

  bInvalid = (mpe->intsrc & INT_COMMRECV) && 
   (!(mpe->commctl & (1UL << 31)));  //&& (!(mpe->intctl & (1UL << 5)) || (mpe->pcexec >= 0x807604D2))) ||
   //((mpe->commctl & (1UL << 31)) && (mpe->pcexec >= 0x807604C8) && (mpe->pcexec < 0x807604D2)));
  bInvalid2 = (!(mpe->intsrc & INT_COMMRECV) && (mpe->commctl & (1UL << 31)) &&
    ((mpe->pcexec <= 0x807604C0) || (mpe->pcexec >= 0x807604D2)));
  if(bInvalid || bInvalid2)
  {
    bInvalid = false;
    bInvalid2 = true;
  }
  return bInvalid || bInvalid2;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  HWND hDlg, hStatusDlg;
  MSG msg;
  LPARAM lParam;
  WPARAM wParam;
  uint32 nCycles = 500;
  uint32 displayCycles;
  static uint32 prevPcexec;
  static uint32 prevCommctl;
  static uint32 prevIntsrc;
  INT_PTR result;

  //Create the Nuon environment object
  nuonEnv = new NuonEnvironment;
  //Initialize the BIOS
  nuonEnv->InitBios();
  hDlg = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_SPLASH_SCREEN),NULL,SplashScreenDialogProc);
  Sleep(1000);
  ShowWindow(hDlg,FALSE);
  EndDialog(hDlg,result);

  display = new GLWindow();
  display->title = displayWindowTitle;
  display->clientWidth = 720;
  display->clientHeight = 480;
  display->x = 100;
  display->y = 100;
  display->bUseSeparateThread = false;
  display->resizeHandler = OnDisplayResize;
  display->keyDownHandler = OnDisplayKeyDown;
  display->keyUpHandler = OnDisplayKeyUp;
  display->paintHandler = OnDisplayPaint;
  display->timerHandler = OnDisplayTimer;

  display->Create();
  while(!display->bVisible);
  GLenum err = glewInit();
  if(err != GLEW_OK)
  {
    MessageBox(NULL,(char *)glewGetErrorString(err),"Error",MB_ICONWARNING);
  }

  mainChannelBuffer = AllocateTextureMemory(ALLOCATED_TEXTURE_WIDTH*ALLOCATED_TEXTURE_HEIGHT*4,false);
  overlayChannelBuffer = AllocateTextureMemory(ALLOCATED_TEXTURE_WIDTH*ALLOCATED_TEXTURE_HEIGHT*4,true);
  InitializeYCrCbColorSpace();
  
  HMODULE hRichEditLibrary = LoadLibrary("Riched20.dll");
  hDlg = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_CONTROL_PANEL),NULL,ControlPanelDialogProc);
  hStatusDlg = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_STATUS_DIALOG),NULL,StatusWindowDialogProc);

  iconApp = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
  bmpLEDOn = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_LED_ON));
  bmpLEDOff = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_LED_OFF));
  reTermDisplay = GetDlgItem(hDlg, RE_TERM_DISPLAY);
  reStatus = GetDlgItem(hStatusDlg, RE_STATUS_DISPLAY);
  picMPE0LED = GetDlgItem(hDlg, IDC_MPE0_LED);
  picMPE1LED = GetDlgItem(hDlg, IDC_MPE1_LED);
  picMPE2LED = GetDlgItem(hDlg, IDC_MPE2_LED);
  picMPE3LED = GetDlgItem(hDlg, IDC_MPE3_LED);
  textMPE0 = GetDlgItem(hDlg, IDC_LABEL_MPE0);
  textMPE1 = GetDlgItem(hDlg, IDC_LABEL_MPE1);
  textMPE2 = GetDlgItem(hDlg, IDC_LABEL_MPE2);
  textMPE3 = GetDlgItem(hDlg, IDC_LABEL_MPE3);
  textMPE0Pcexec = GetDlgItem(hDlg, IDC_MPE0_PCEXEC);
  textMPE1Pcexec = GetDlgItem(hDlg, IDC_MPE1_PCEXEC);
  textMPE2Pcexec = GetDlgItem(hDlg, IDC_MPE2_PCEXEC);
  textMPE3Pcexec = GetDlgItem(hDlg, IDC_MPE3_PCEXEC);
  cbLoadFile = GetDlgItem(hDlg, IDC_CB_LOAD_FILE);
  cbRun = GetDlgItem(hDlg, IDC_CB_RUN);
  cbSingleStep = GetDlgItem(hDlg, IDC_CB_SINGLE_STEP);
  cbStop = GetDlgItem(hDlg, IDC_CB_STOP);
  cbReset = GetDlgItem(hDlg, IDC_CB_RESET);
  cbCommStatus = GetDlgItem(hStatusDlg, IDC_COMM_STATUS);
  cbDisplayStatus = GetDlgItem(hStatusDlg, IDC_DISPLAY_STATUS);
  cbMPEStatus = GetDlgItem(hStatusDlg, IDC_MPE_STATUS);
  cbDumpMPEs = GetDlgItem(hStatusDlg, IDC_DUMP_MPES);

  memset(&ofn,0,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hDlg;
  ofn.hInstance = hInstance;
  ofn.Flags = OFN_LONGNAMES|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
  ofn.lpstrFile = openFileName;
  ofn.nMaxFile = sizeof(openFileName);

  SendMessage(hDlg,WM_SETICON,ICON_SMALL,LPARAM(iconApp));
  SendMessage(hStatusDlg,WM_SETICON,ICON_SMALL,LPARAM(iconApp));
  SendMessage(display->hWnd,WM_SETICON,ICON_SMALL,LPARAM(iconApp));

  UpdateControlPanelDisplay();

  display->timerInterval = 1000.0/nuonEnv->fps;
  display->SetTimer();

  nuonEnv->videoDisplayCycleCount = 0;

  while(!bQuit)
  {
    if(PeekMessage(&msg,hDlg,0,0,PM_REMOVE))
    {
      IsDialogMessage(hDlg,&msg);
    }

    if(PeekMessage(&msg,hStatusDlg,0,0,PM_REMOVE))
    {
      IsDialogMessage(hStatusDlg,&msg);
    }

    display->MessagePump();

    if(bRun)
    {
      while(nCycles--)
      {
        nuonEnv->mpe[3]->ExecuteSingleCycle();
        nuonEnv->mpe[2]->ExecuteSingleCycle();
        nuonEnv->mpe[1]->ExecuteSingleCycle();
        nuonEnv->mpe[0]->ExecuteSingleCycle();
        if(nuonEnv->pendingCommRequests)
        {
          DoCommBusController();
        }
        //nuonEnv->videoDisplayCycleCount += nuonEnv->mpe[3]->cycleCounter;
        //ProcessCycleBasedEvents();
      }      

      //if(nuonEnv->videoDisplayCycleCount >= (54000000/60))
      //{
      //  IncrementVideoFieldCounter();
      //  nuonEnv->videoDisplayCycleCount -= (54000000/60);
      //}
    }

    nCycles = 500;
  }

/********************************************************************
CLEANUP AND APPLICATION SHUTDOWN CODE
********************************************************************/

  ShowWindow(hDlg,FALSE);
  ShowWindow(hStatusDlg,FALSE);
  EndDialog(hDlg,result);
  EndDialog(hStatusDlg,result);

  FreeTextureMemory(mainChannelBuffer,false);
  FreeTextureMemory(overlayChannelBuffer,true);

  //Destroy the Nuon environment object
  delete nuonEnv;

  display->Close();

  delete display;

  return 0;
}

void NuanceStart(int argc, char *argv[])
{
  //emulatorShell->show();
  //registerWindow->show();

  //registerWindow->UpdateDebugDisplay(3);
  //nuanceApp.exec();
  
}