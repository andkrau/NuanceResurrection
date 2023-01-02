#include "basetypes.h"

#include <cstdio>
#include <string>
#include <windows.h>
#include <Commctrl.h>
#include <tchar.h>
#include <commdlg.h>
#include "external\glew-2.2.0\include\GL\glew.h"
#include <GL/gl.h>
#include <mutex>

#include "byteswap.h"
#include "Utility.h"
#include "comm.h"
//#include "CriticalSection.h"
#include "GLWindow.h"
#include "audio.h"
#include "mpe.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "NuanceRes.h"
#include "joystick.h"
#include "video.h"
#include "ExecuteMEM.h"
#include "timer.h"
#include "Bios.h"

NuonEnvironment nuonEnv;
char **pArgs = 0;

extern ControllerData *controller;

extern std::mutex gfx_lock;

extern VidChannel structMainChannel, structOverlayChannel;
extern bool bOverlayChannelActive, bMainChannelActive;
extern vidTexInfo videoTexInfo;

//

static bool bQuit = false;
static bool bRun = false;

static bool load4firsttime = true;

GLWindow display;

static HICON iconApp;
static HBITMAP bmpLEDOn;
static HBITMAP bmpLEDOff;
static HWND picMPE0LED;
static HWND picMPE1LED;
static HWND picMPE2LED;
static HWND picMPE3LED;
static HWND textMPE0;
static HWND textMPE1;
static HWND textMPE2;
static HWND textMPE3;
static HWND textMPE0Pcexec;
static HWND textMPE1Pcexec;
static HWND textMPE2Pcexec;
static HWND textMPE3Pcexec;
static HWND cbLoadFile;
static HWND cbCfgInput;
static HWND cbSingleStep;
static HWND cbRun;
static HWND cbStop;
static HWND cbReset;
static HWND reTermDisplay;

static HWND cbCommStatus;
static HWND cbDisplayStatus;
static HWND cbMPEStatus;
static HWND cbDumpMPEs;
static HWND reStatus;

static OPENFILENAME ofn;
static char openFileName[512];

static unsigned long disassemblyMPE = 3;
static char whichStatus = -1;


static bool GetMPERunStatus(const uint32 which)
{
  return (nuonEnv.mpe[which & 0x03].mpectl & MPECTRL_MPEGO) != 0;
}

static void SetMPERunStatus(const uint32 which, const bool run)
{
  if(run)
    nuonEnv.mpe[which & 0x03].mpectl |= MPECTRL_MPEGO;
  else
    nuonEnv.mpe[which & 0x03].mpectl &= ~MPECTRL_MPEGO;
}

static void UpdateStatusWindowDisplay()
{
  char buf[1024];
  if(whichStatus == 0)
  {
    sprintf_s(buf, sizeof(buf),"Pending Comm Requests = %lu\n",nuonEnv.pendingCommRequests);
    SendMessage(reStatus,WM_SETTEXT,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE0 commctl = $%8.8lx, commxmit0 = $%8.8lx, commrecv0 = $%8.8lx, comminfo = $%8.8lx\n",nuonEnv.mpe[0].commctl,nuonEnv.mpe[0].commxmit[0],nuonEnv.mpe[0].commrecv[0],nuonEnv.mpe[0].comminfo);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE1 commctl = $%8.8lx, commxmit0 = $%8.8lx, commrecv0 = $%8.8lx, comminfo = $%8.8lx\n",nuonEnv.mpe[1].commctl,nuonEnv.mpe[1].commxmit[0],nuonEnv.mpe[1].commrecv[0],nuonEnv.mpe[1].comminfo);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE2 commctl = $%8.8lx, commxmit0 = $%8.8lx, commrecv0 = $%8.8lx, comminfo = $%8.8lx\n",nuonEnv.mpe[2].commctl,nuonEnv.mpe[2].commxmit[0],nuonEnv.mpe[2].commrecv[0],nuonEnv.mpe[2].comminfo);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE3 commctl = $%8.8lx, commxmit0 = $%8.8lx, commrecv0 = $%8.8lx, comminfo = $%8.8lx\n",nuonEnv.mpe[3].commctl,nuonEnv.mpe[3].commxmit[0],nuonEnv.mpe[3].commrecv[0],nuonEnv.mpe[3].comminfo);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
  }
  else if(whichStatus == 1)
  {
    sprintf_s(buf, sizeof(buf),"Main Channel = %s : Overlay Channel = %s\n",bMainChannelActive ? "ON" : "OFF",bOverlayChannelActive ? "ON" : "OFF");
    SendMessage(reStatus,WM_SETTEXT,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Main Channel texture coordinates = (%.3f %.3f), (%.3f %.3f), (%.3f %.3f), (%.3f %.3f)\n",
      videoTexInfo.mainTexCoords[0],videoTexInfo.mainTexCoords[1],
      videoTexInfo.mainTexCoords[2],videoTexInfo.mainTexCoords[3],
      videoTexInfo.mainTexCoords[4],videoTexInfo.mainTexCoords[5],
      videoTexInfo.mainTexCoords[6],videoTexInfo.mainTexCoords[7]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Overlay Channel texture coordinates = (%.3f %.3f), (%.3f %.3f), (%.3f %.3f), (%.3f %.3f)\n",
      videoTexInfo.osdTexCoords[0],videoTexInfo.osdTexCoords[1],
      videoTexInfo.osdTexCoords[2],videoTexInfo.osdTexCoords[3],
      videoTexInfo.osdTexCoords[4],videoTexInfo.osdTexCoords[5],
      videoTexInfo.osdTexCoords[6],videoTexInfo.osdTexCoords[7]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Main Channel pixel type = %lu, Overlay Channel pixel type = %lu\n",(structMainChannel.dmaflags >> 4) & 0xF,(structOverlayChannel.dmaflags >> 4) & 0xF);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Main Channel base = $%8.8lx, Overlay Channel base = $%8.8lx\n",(structMainChannel.base),(structOverlayChannel.base));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Main Channel src_width = %lu, Main Channel src_height = %lu\n",(structMainChannel.src_width),(structMainChannel.src_height));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Main Channel dest_width = %lu, Main Channel dest_height = %lu\n",(structMainChannel.dest_width),(structMainChannel.dest_height));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Main Channel src_xoff = %lu, Main Channel src_yoff = %lu\n",(structMainChannel.src_xoff),(structMainChannel.src_yoff));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Main Channel dest_xoff = %lu, Main Channel dest_yoff = %lu\n",(structMainChannel.dest_xoff),(structMainChannel.dest_yoff));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Overlay Channel src_width = %lu, Overlay Channel src_height = %lu\n",(structOverlayChannel.src_width),(structOverlayChannel.src_height));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Overlay Channel dest_width = %lu, Overlay Channel dest_height = %lu\n",(structOverlayChannel.dest_width),(structOverlayChannel.dest_height));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Overlay Channel src_xoff = %lu, Overlay Channel src_yoff = %lu\n",(structOverlayChannel.src_xoff),(structOverlayChannel.src_yoff));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Overlay Channel dest_xoff = %lu, Overlay Channel dest_yoff = %lu\n",(structOverlayChannel.dest_xoff),(structOverlayChannel.dest_yoff));
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
  }
  else if (whichStatus == 2)
  {
    sprintf_s(buf, sizeof(buf),"Interpreter cache flushes: (%u, %u, %u, %u)\n",
      nuonEnv.mpe[0].numInterpreterCacheFlushes,
      nuonEnv.mpe[1].numInterpreterCacheFlushes,
      nuonEnv.mpe[2].numInterpreterCacheFlushes,
      nuonEnv.mpe[3].numInterpreterCacheFlushes);
    SendMessage(reStatus,WM_SETTEXT,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Native code cache flushes: (%u, %u, %u, %u)\n",
      nuonEnv.mpe[0].numNativeCodeCacheFlushes,
      nuonEnv.mpe[1].numNativeCodeCacheFlushes,
      nuonEnv.mpe[2].numNativeCodeCacheFlushes,
      nuonEnv.mpe[3].numNativeCodeCacheFlushes);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Non-compilable packets: (%u, %u, %u, %u)\n",
      nuonEnv.mpe[0].numNonCompilablePackets,
      nuonEnv.mpe[1].numNonCompilablePackets,
      nuonEnv.mpe[2].numNonCompilablePackets,
      nuonEnv.mpe[3].numNonCompilablePackets);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Overlays in use: (%u, %u, %u, %u)\n",
      nuonEnv.mpe[0].overlayManager.GetOverlaysInUse(),
      nuonEnv.mpe[1].overlayManager.GetOverlaysInUse(),
      nuonEnv.mpe[2].overlayManager.GetOverlaysInUse(),
      nuonEnv.mpe[3].overlayManager.GetOverlaysInUse());
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE3 intvec1 = $%8.8lx, intvec2 = $%8.8lx\n",nuonEnv.mpe[3].intvec1,nuonEnv.mpe[3].intvec2);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE3 intctl = $%8.8lx, inten1 = $%8.8lx, inten2sel = $%8.8lx\n",nuonEnv.mpe[3].intctl,nuonEnv.mpe[3].inten1,nuonEnv.mpe[3].inten2sel);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE3 intsrc = $%8.8lx, excepsrc = $%8.8lx\n",nuonEnv.mpe[3].intsrc,nuonEnv.mpe[3].excepsrc);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE2 intvec1 = $%8.8lx, intvec2 = $%8.8lx\n",nuonEnv.mpe[2].intvec1,nuonEnv.mpe[2].intvec2);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE2 intctl = $%8.8lx, inten1 = $%8.8lx, inten2sel = $%8.8lx\n",nuonEnv.mpe[2].intctl,nuonEnv.mpe[2].inten1,nuonEnv.mpe[2].inten2sel);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE2 intsrc = $%8.8lx, excepsrc = $%8.8lx\n",nuonEnv.mpe[2].intsrc,nuonEnv.mpe[2].excepsrc);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE1 intvec1 = $%8.8lx, intvec2 = $%8.8lx\n",nuonEnv.mpe[1].intvec1,nuonEnv.mpe[1].intvec2);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE1 intctl = $%8.8lx, inten1 = $%8.8lx, inten2sel = $%8.8lx\n",nuonEnv.mpe[1].intctl,nuonEnv.mpe[1].inten1,nuonEnv.mpe[1].inten2sel);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE1 intsrc = $%8.8lx, excepsrc = $%8.8lx\n",nuonEnv.mpe[1].intsrc,nuonEnv.mpe[1].excepsrc);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE0 intvec1 = $%8.8lx, intvec2 = $%8.8lx\n",nuonEnv.mpe[0].intvec1,nuonEnv.mpe[0].intvec2);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE0 intctl = $%8.8lx, inten1 = $%8.8lx, inten2sel = $%8.8lx\n",nuonEnv.mpe[0].intctl,nuonEnv.mpe[0].inten1,nuonEnv.mpe[0].inten2sel);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE0 intsrc = $%8.8lx, excepsrc = $%8.8lx\n",nuonEnv.mpe[0].intsrc,nuonEnv.mpe[0].excepsrc);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"Stack pointers = [0: $%8.8lx, 1: $%8.8lx, 2: $%8.8lx, 3: $%8.8lx]\n",nuonEnv.mpe[0].sp,nuonEnv.mpe[1].sp,nuonEnv.mpe[2].sp,nuonEnv.mpe[3].sp);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"MPE %u:\n", disassemblyMPE);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"v0 = (r00: $%8.8lx, r01: $%8.8lx, r02: $%8.8lx, r03: $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].regs[0],nuonEnv.mpe[disassemblyMPE].regs[1],nuonEnv.mpe[disassemblyMPE].regs[2],nuonEnv.mpe[disassemblyMPE].regs[3]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"v1 = (r04: $%8.8lx, r05: $%8.8lx, r06: $%8.8lx, r07: $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].regs[4],nuonEnv.mpe[disassemblyMPE].regs[5],nuonEnv.mpe[disassemblyMPE].regs[6],nuonEnv.mpe[disassemblyMPE].regs[7]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"v2 = (r08: $%8.8lx, r09: $%8.8lx, r10: $%8.8lx, r11: $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].regs[8],nuonEnv.mpe[disassemblyMPE].regs[9],nuonEnv.mpe[disassemblyMPE].regs[10],nuonEnv.mpe[disassemblyMPE].regs[11]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"v3 = (r12: $%8.8lx, r13: $%8.8lx, r14: $%8.8lx, r15: $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].regs[12],nuonEnv.mpe[disassemblyMPE].regs[13],nuonEnv.mpe[disassemblyMPE].regs[14],nuonEnv.mpe[disassemblyMPE].regs[15]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"v4 = (r16: $%8.8lx, r17: $%8.8lx, r18: $%8.8lx, r19: $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].regs[16],nuonEnv.mpe[disassemblyMPE].regs[17],nuonEnv.mpe[disassemblyMPE].regs[18],nuonEnv.mpe[disassemblyMPE].regs[19]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"v5 = (r20: $%8.8lx, r21: $%8.8lx, r22: $%8.8lx, r23: $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].regs[20],nuonEnv.mpe[disassemblyMPE].regs[21],nuonEnv.mpe[disassemblyMPE].regs[22],nuonEnv.mpe[disassemblyMPE].regs[23]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"v6 = (r24: $%8.8lx, r25: $%8.8lx, r26: $%8.8lx, r27: $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].regs[24],nuonEnv.mpe[disassemblyMPE].regs[25],nuonEnv.mpe[disassemblyMPE].regs[26],nuonEnv.mpe[disassemblyMPE].regs[27]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"v7 = (r28: $%8.8lx, r29: $%8.8lx, r30: $%8.8lx, r31: $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].regs[28],nuonEnv.mpe[disassemblyMPE].regs[29],nuonEnv.mpe[disassemblyMPE].regs[30],nuonEnv.mpe[disassemblyMPE].regs[31]);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"cc/rc0/rc1 = ($%8.8lx, $%8.8lx, $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].cc,nuonEnv.mpe[disassemblyMPE].rc0,nuonEnv.mpe[disassemblyMPE].rc1);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"rx/ry/ru/rv = ($%8.8lx, $%8.8lx, $%8.8lx, $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].rx,nuonEnv.mpe[disassemblyMPE].ry,nuonEnv.mpe[disassemblyMPE].ru,nuonEnv.mpe[disassemblyMPE].rv);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"rz/rzi1/rzi2 = ($%8.8lx, $%8.8lx, $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].rz,nuonEnv.mpe[disassemblyMPE].rzi1,nuonEnv.mpe[disassemblyMPE].rzi2);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"xyctl/uvctl/xyrange/uvrange = ($%8.8lx, $%8.8lx, $%8.8lx, $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].xyctl,nuonEnv.mpe[disassemblyMPE].uvctl,nuonEnv.mpe[disassemblyMPE].xyrange,nuonEnv.mpe[disassemblyMPE].uvrange);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
    sprintf_s(buf, sizeof(buf),"acshift/svshift = ($%8.8lx, $%8.8lx)\n",nuonEnv.mpe[disassemblyMPE].acshift,nuonEnv.mpe[disassemblyMPE].svshift);
    SendMessage(reStatus,EM_REPLACESEL,NULL,LPARAM(buf));
    SendMessage(reStatus,EM_SETSEL,WPARAM(-1),LPARAM(-1));
  }
}

static void UpdateControlPanelDisplay()
{
  const HWND ledHandles[] = {picMPE0LED,picMPE1LED,picMPE2LED,picMPE3LED};
  const HWND pcexecHandles[] = {textMPE0Pcexec,textMPE1Pcexec,textMPE2Pcexec,textMPE3Pcexec};

  for(uint32 i = 0; i < 4; i++)
  {
    SendMessage(ledHandles[i],STM_SETIMAGE,IMAGE_BITMAP, GetMPERunStatus(i) ? LPARAM(bmpLEDOn) : LPARAM(bmpLEDOff));
    char addressStr[16];
    sprintf_s(addressStr, sizeof(addressStr), "$%8.8lX", GetMPERunStatus(i) ? nuonEnv.mpe[i].pcexec : 0);
    SendMessage(pcexecHandles[i],WM_SETTEXT,0,LPARAM(addressStr));
  }

  char buf[1024];
  sprintf_s(buf, sizeof(buf),"mpe%lu: $%8.8X\n{\n", disassemblyMPE, GetMPERunStatus(disassemblyMPE) ? nuonEnv.mpe[disassemblyMPE].pcexec : 0);

  SendMessage(reTermDisplay,WM_SETTEXT,NULL,LPARAM(buf));
  SendMessage(reTermDisplay,EM_SETSEL,WPARAM(-1),LPARAM(-1));

  if(GetMPERunStatus(disassemblyMPE))
    nuonEnv.mpe[disassemblyMPE].PrintInstructionCachePacket(buf, sizeof(buf), nuonEnv.mpe[disassemblyMPE].pcexec);
  else
    buf[0] = '\0';
  SendMessage(reTermDisplay,EM_REPLACESEL,NULL,LPARAM(buf));

  sprintf_s(buf, sizeof(buf),"}\n");
  SendMessage(reTermDisplay,EM_REPLACESEL,NULL,LPARAM(buf));
}

static void OnMPELEDDoubleClick(uint32 which)
{
  const HWND handles[] = {picMPE0LED,picMPE1LED,picMPE2LED,picMPE3LED};

  which = which & 0x03;
  const bool rs = GetMPERunStatus(which);
  SetMPERunStatus(which,!rs);
  SendMessage(handles[which],STM_SETIMAGE,IMAGE_BITMAP,rs ? LPARAM(bmpLEDOff) : LPARAM(bmpLEDOn));
}

static void ExecuteSingleStep()
{
  nuonEnv.mpe[3].ExecuteSingleStep();
  nuonEnv.mpe[2].ExecuteSingleStep();
  nuonEnv.mpe[1].ExecuteSingleStep();
  nuonEnv.mpe[0].ExecuteSingleStep();
  if(nuonEnv.pendingCommRequests)
    DoCommBusController();
}

INT_PTR CALLBACK StatusWindowDialogProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
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
            FILE* outFile;
            if ( fopen_s(&outFile, "mpe0.bin","wb") == 0 )
            {
              fwrite(nuonEnv.mpe[0].dtrom,sizeof(uint8),MPE_LOCAL_MEMORY_SIZE,outFile);
              fclose(outFile);
            }
            if ( fopen_s(&outFile, "mpe1.bin","wb") == 0 )
            {
              fwrite(nuonEnv.mpe[1].dtrom,sizeof(uint8),MPE_LOCAL_MEMORY_SIZE,outFile);
              fclose(outFile);
            }
            if ( fopen_s(&outFile, "mpe2.bin","wb") == 0 )
            {
              fwrite(nuonEnv.mpe[2].dtrom,sizeof(uint8),MPE_LOCAL_MEMORY_SIZE,outFile);
              fclose(outFile);
            }
            if ( fopen_s(&outFile, "mpe3.bin","wb") == 0 )
            {
              fwrite(nuonEnv.mpe[3].dtrom,sizeof(uint8),MPE_LOCAL_MEMORY_SIZE,outFile);
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

void Run()
{
            uint32 bpAddr = 0;
            FILE* inFile;

            if ( fopen_s(&inFile, "breakpoint.txt", "r") == 0 )
            {
              fscanf_s(inFile,"%x",&bpAddr);
              fclose(inFile);
            }

            nuonEnv.mpe[0].breakpointAddress = bpAddr;
            nuonEnv.mpe[1].breakpointAddress = bpAddr;
            nuonEnv.mpe[2].breakpointAddress = bpAddr;
            nuonEnv.mpe[3].breakpointAddress = bpAddr;

            EnableWindow(cbLoadFile,FALSE);
            EnableWindow(cbRun,FALSE);
            EnableWindow(cbSingleStep,FALSE);
            EnableWindow(cbStop,TRUE);
            bRun = true;
}

bool Load()
{
  if(GetOpenFileName(&ofn))
  {
    bool bSuccess = nuonEnv.mpe[3].LoadNuonRomFile(ofn.lpstrFile);
    if(!bSuccess)
    {
      bSuccess = nuonEnv.mpe[3].LoadCoffFile(ofn.lpstrFile);
      if(!bSuccess)
        MessageBox(NULL,"Invalid COFF or NUONROM-DISK file",ERROR,MB_ICONWARNING);
    }
    
    if(bSuccess)
    {
      nuonEnv.SetDVDBaseFromFileName(ofn.lpstrFile);
      nuonEnv.mpe[3].Go();
      UpdateControlPanelDisplay();
      SetWindowPos(display.hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
      Run();
      return true;
    }
  }

  return false;
}

static int EditIDToCtrlrBitnum(int id)
{
  switch (id)
  {
  case IDC_SET_DUP:
    return CTRLR_BITNUM_DPAD_UP;

  case IDC_SET_DRIGHT:
    return CTRLR_BITNUM_DPAD_RIGHT;

  case IDC_SET_DDOWN:
    return CTRLR_BITNUM_DPAD_DOWN;

  case IDC_SET_DLEFT:
    return CTRLR_BITNUM_DPAD_LEFT;

  case IDC_SET_CUP:
    return CTRLR_BITNUM_BUTTON_C_UP;

  case IDC_SET_CRIGHT:
    return CTRLR_BITNUM_BUTTON_C_RIGHT;

  case IDC_SET_CDOWN:
    return CTRLR_BITNUM_BUTTON_C_DOWN;

  case IDC_SET_CLEFT:
    return CTRLR_BITNUM_BUTTON_C_LEFT;

  case IDC_SET_A:
    return CTRLR_BITNUM_BUTTON_A;
    
  case IDC_SET_B:
    return CTRLR_BITNUM_BUTTON_B;

  case IDC_SET_L:
    return CTRLR_BITNUM_BUTTON_L;

  case IDC_SET_R:
    return CTRLR_BITNUM_BUTTON_R;

  case IDC_SET_NUON:
    return CTRLR_BITNUM_BUTTON_NUON;

  case IDC_SET_START:
    return CTRLR_BITNUM_BUTTON_START;

  default:
    return -1;
  }
}

void SetNewMapping(HWND hWnd, InputManager::InputType type, int idx, int subIdx)
{
  char mappingStr[ControllerButtonMapping::MAPPING_STRING_SIZE];
  ControllerButtonMapping newMap(type, idx, subIdx);
  newMap.toString(mappingStr, _countof(mappingStr));
  SetWindowText(hWnd, mappingStr);
  SetFocus(GetNextDlgTabItem(GetParent(hWnd), hWnd, FALSE));
}

static WNDPROC pOrigEditProc;
LRESULT APIENTRY SetButtonControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // Call down to base control handler first.
  LRESULT res = CallWindowProc(pOrigEditProc, hWnd, uMsg, wParam, lParam);

  // Intercept most keydown messages.
  if ((uMsg == WM_GETDLGCODE) && lParam)
  {
    MSG* msg = (MSG*)lParam;

    if (msg->message == WM_KEYDOWN)
    {
      switch (msg->wParam)
      {
        // Use default processing for Escape
      case VK_ESCAPE:
        break;

      default:
        SetNewMapping(hWnd, InputManager::KEY, (int)wParam, 0);
        return DLGC_WANTMESSAGE;
      }
    }
  }

  return res;
}

struct JoyPressedCtx
{
  InputManager::InputType type;
  int idx;
  int subIdx;
  bool pressed;
};

static void AnyJoyPressed(void* ctx, InputManager::InputType type, int idx, int subIdx)
{
  JoyPressedCtx *joyCtx = (JoyPressedCtx *)ctx;

  if (!joyCtx->pressed)
  {
    joyCtx->type = type;
    joyCtx->idx = idx;
    joyCtx->subIdx = subIdx;
    joyCtx->pressed = true;
  }
}

#define FOR_ALL_JOY_EDIT_CTRLS(_op) \
    _op(IDC_SET_DUP); \
    _op(IDC_SET_DDOWN); \
    _op(IDC_SET_DLEFT); \
    _op(IDC_SET_DRIGHT); \
    _op(IDC_SET_START); \
    _op(IDC_SET_NUON); \
    _op(IDC_SET_CUP); \
    _op(IDC_SET_CDOWN); \
    _op(IDC_SET_CLEFT); \
    _op(IDC_SET_CRIGHT); \
    _op(IDC_SET_A); \
    _op(IDC_SET_B); \
    _op(IDC_SET_L); \
    _op(IDC_SET_R)

static constexpr unsigned int IDT_JOY_TIMER = 0x00000001;
static HWND hFocusedWnd = nullptr;
static int joyGrabbed = -1;
INT_PTR CALLBACK CfgInputDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  InputManager* im = display.GetInputManager();
  switch (msg)
  {
  case WM_INITDIALOG:
    if (im)
    {
      size_t numJoysticks;
      const Joystick* pJoysticks = im->EnumJoysticks(&numJoysticks);
      const GUID& controller1Joystick = nuonEnv.GetController1Joystick();
      int controller1JoystickIdx = -1;
      HWND hwndCtrl = GetDlgItem(hwndDlg, IDC_JOYSTICK_COMBO);

      for (size_t i = 0; i < numJoysticks; i++) {
        SendMessage(hwndCtrl, CB_ADDSTRING, 0, (LPARAM)pJoysticks[i].tszName);
        if (controller1Joystick == pJoysticks[i].guid)
        {
          controller1JoystickIdx = i;
        }
      }

      if (numJoysticks)
      {
        if (controller1JoystickIdx < 0) controller1JoystickIdx = 0;
        SendMessage(hwndCtrl, CB_SETCURSEL, controller1JoystickIdx, 0);
        if (im->GrabJoystick(hwndDlg, controller1JoystickIdx)) joyGrabbed = controller1JoystickIdx;
      }

      char mappingStr[ControllerButtonMapping::MAPPING_STRING_SIZE];
#define INIT_CTRLR_EDIT(id) \
do { \
  hwndCtrl = GetDlgItem(hwndDlg, (id)); \
  WNDPROC proc = (WNDPROC)SetWindowLongPtr(hwndCtrl, GWL_WNDPROC, (LONG_PTR)SetButtonControlProc); \
  if (pOrigEditProc) \
  { \
      assert(pOrigEditProc == proc); \
  } \
  else \
  { \
    pOrigEditProc = proc; \
  } \
  nuonEnv.GetMappingForCTRLRBitnum(EditIDToCtrlrBitnum(id)).toString(mappingStr, _countof(mappingStr)); \
  SetWindowText(hwndCtrl, mappingStr); \
} while (0)
      FOR_ALL_JOY_EDIT_CTRLS(INIT_CTRLR_EDIT);
#undef INIT_CTRLR_EDIT
    }
    break;

  case WM_DESTROY:
    if (im)
    {
      HWND hwndCtrl;
#define UNINIT_CTRLR_EDIT(id) \
do { \
  hwndCtrl = GetDlgItem(hwndDlg, (id)); \
  SetWindowLongPtr(hwndCtrl, GWL_WNDPROC, (LONG_PTR)pOrigEditProc); \
} while(0)
      FOR_ALL_JOY_EDIT_CTRLS(UNINIT_CTRLR_EDIT);
#undef UNINIT_CTRLR_EDIT
    }
    return TRUE;

  case WM_CTLCOLORSTATIC:
  {
    HWND hWndCtrl = (HWND)lParam;
    if (hWndCtrl == hFocusedWnd)
    {
      HDC hDC = (HDC)wParam;
      SetBkColor(hDC, RGB(182, 208, 226));
      return (INT_PTR)CreateSolidBrush(RGB(182, 208, 226));
    }
  }
  break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDB_OK:
      {
        EndDialog(hwndDlg, TRUE);
        if (joyGrabbed >= 0)
        {
          im->UngrabJoystick();
          joyGrabbed = -1;
        }

        HWND hJoyCombo = GetDlgItem(hwndDlg, IDC_JOYSTICK_COMBO);
        LRESULT res = SendMessage(hJoyCombo, CB_GETCURSEL, 0, 0);

        if (res != CB_ERR)
        {
          size_t numJoysticks;
          const Joystick* pJoysticks = im->EnumJoysticks(&numJoysticks);
          im->SetJoystick((size_t)res);
          nuonEnv.SetController1Joystick(pJoysticks[res].guid);
        }

#define APPLY_MAPPING(id) \
do { \
  HWND hEditWnd = GetDlgItem(hwndDlg, (id)); \
  char dlgText[ControllerButtonMapping::MAPPING_STRING_SIZE]; \
  GetWindowTextA(hEditWnd, dlgText, _countof(dlgText)); \
  ControllerButtonMapping newMap; \
  ControllerButtonMapping::fromString(dlgText, &newMap); \
  nuonEnv.SetControllerButtonMapping(EditIDToCtrlrBitnum(id), newMap); \
} while (0)
        FOR_ALL_JOY_EDIT_CTRLS(APPLY_MAPPING);
#undef APPLY_MAPPING
      }
      return TRUE;

    case IDB_CANCEL:
      EndDialog(hwndDlg, FALSE);
      if (joyGrabbed >= 0)
      {
        im->UngrabJoystick();
        joyGrabbed = -1;
      }
      return TRUE;

    case IDC_JOYSTICK_COMBO:
      if (HIWORD(wParam) != CBN_SELENDOK) break;

      if (joyGrabbed >= 0)
      {
        im->UngrabJoystick();
      }
      if (im)
      {
        HWND hJoyCombo = (HWND)lParam;
        LRESULT res = SendMessage(hJoyCombo, CB_GETCURSEL, 0, 0);

        if (res != CB_ERR)
        {
          if (im->GrabJoystick(hwndDlg, (size_t)res)) joyGrabbed = (int)res;
        }
        else
        {
          MessageBox(NULL, _T("Error sending GETCURSEL message"), _T("Error"), MB_ICONERROR);
        }
      }
      return TRUE;

    case IDC_SET_DUP:
    case IDC_SET_DDOWN:
    case IDC_SET_DLEFT:
    case IDC_SET_DRIGHT:
    case IDC_SET_START:
    case IDC_SET_NUON:
    case IDC_SET_CUP:
    case IDC_SET_CDOWN:
    case IDC_SET_CLEFT:
    case IDC_SET_CRIGHT:
    case IDC_SET_A:
    case IDC_SET_B:
    case IDC_SET_L:
    case IDC_SET_R:
      switch (HIWORD(wParam))
      {
      case EN_SETFOCUS:
      {
        HWND hEdit = (HWND)lParam;
        hFocusedWnd = hEdit;
        if (joyGrabbed >= 0)
        {
          im->UpdateState(NULL, NULL, NULL);
          SetTimer(hwndDlg, IDT_JOY_TIMER, 50, nullptr);
        }
        InvalidateRect(hEdit, NULL, FALSE);

        break;
      }
      case EN_KILLFOCUS:
      {
        HWND hEdit = (HWND)lParam;
        if (hFocusedWnd == hEdit)
        {
          if (joyGrabbed >= 0) KillTimer(hwndDlg, IDT_JOY_TIMER);
          hFocusedWnd = nullptr;
          InvalidateRect(hEdit, NULL, FALSE);
        }
        break;

      }
      default:
        break;
      }
      break;
    }

  case WM_TIMER:
    if ((wParam == IDT_JOY_TIMER) && im && hFocusedWnd)
    {
      JoyPressedCtx ctx;
      ctx.pressed = false;

      im->UpdateState(NULL, AnyJoyPressed, &ctx);

      if (ctx.pressed)
      {
        SetNewMapping(hFocusedWnd, ctx.type, ctx.idx, ctx.subIdx);
      }
      return 0;
    }
    break;

  default:
    break;
  }

  return FALSE;
}

INT_PTR CALLBACK ControlPanelDialogProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
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
            Run();
            return TRUE;
          }
          else if((HWND)lParam == cbSingleStep)
          {
            EnableWindow(cbRun,FALSE);
            EnableWindow(cbSingleStep,FALSE);
            EnableWindow(cbStop,TRUE);
            ExecuteSingleStep();
            UpdateStatusWindowDisplay();
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
            for(uint32 i = 0; i < 4; i++)
              nuonEnv.mpe[i].Reset();

            EnableWindow(cbLoadFile, TRUE);

            return TRUE;
          }
          else if((HWND)lParam == cbLoadFile)
          {
            if (!load4firsttime)
              nuonEnv.InitBios(); //!! hacky, but seems to work

            if (Load())
              load4firsttime = false;

            return TRUE;
          }
          else if ((HWND)lParam == cbCfgInput)
          {
              HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hwndDlg, GWL_HINSTANCE);
              if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_CFG_INPUT), hwndDlg, CfgInputDialogProc))
              {
                if (MessageBox(hwndDlg, _T("Save Joystick config to configuration file?"), _T("Save Config"), MB_YESNO | MB_ICONQUESTION) == IDYES)
                {
                  nuonEnv.SaveConfigFile(nullptr);
                }
              }
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

INT_PTR CALLBACK SplashScreenDialogProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
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
    RenderVideo(display.clientWidth,display.clientHeight);
  else
  {
    if(bUseSeparateThread) gfx_lock.lock();
    glClear(GL_COLOR_BUFFER_BIT);
    //glFlush();
    SwapBuffers(display.hDC);
    if(bUseSeparateThread) gfx_lock.unlock();
  }

  return true;
}

bool OnDisplayResize(uint16 width, uint16 height)
{
  if(bUseSeparateThread) gfx_lock.lock();

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

  if(bUseSeparateThread) gfx_lock.unlock();

  OnDisplayPaint(0,0);

  return false;
}

static void ApplyControllerState(unsigned int controllerIdx, uint16 buttons)
{
    if (controller)
    {
        SwapWordBytes(&buttons);
        controller[controllerIdx].buttons = buttons;
    }
}

bool CheckForInvalidCommStatus(const MPE &mpe)
{
  bool bInvalid = (mpe.intsrc & INT_COMMRECV) && 
   (!(mpe.commctl & (1UL << 31)));  //&& (!(mpe.intctl & (1UL << 5)) || (mpe.pcexec >= 0x807604D2))) ||
   //((mpe.commctl & (1UL << 31)) && (mpe.pcexec >= 0x807604C8) && (mpe.pcexec < 0x807604D2)));
  bool bInvalid2 = (!(mpe.intsrc & INT_COMMRECV) && (mpe.commctl & (1UL << 31)) &&
    ((mpe.pcexec <= 0x807604C0) || (mpe.pcexec >= 0x807604D2)));
  if(bInvalid || bInvalid2)
  {
    bInvalid = false;
    bInvalid2 = true;
  }
  return bInvalid || bInvalid2;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  init_supported_CPU_extensions();

  HWND hDlg = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_SPLASH_SCREEN),NULL,SplashScreenDialogProc);
  Sleep(1000);
  ShowWindow(hDlg,FALSE);
  EndDialog(hDlg,IDOK);

  GenerateMirrorLookupTable();
  GenerateSaturateColorTables();

  nuonEnv.Init();

  display.hInstance = hInstance;
  display.applyControllerState = ApplyControllerState;
  display.resizeHandler = OnDisplayResize;
  display.paintHandler = OnDisplayPaint;

  display.Create();
  while (!display.bVisible) {}

  if(bUseSeparateThread)
  {
    HGLRC hRC = wglCreateContext(display.hDC);
    wglShareLists(hRC, display.hRC);
    //wglMakeCurrent(display.hDC, hRC);
  }

  const HMODULE hRichEditLibrary = LoadLibrary("Riched20.dll"); // needs to be loaded, otherwise program hangs
  hDlg = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_CONTROL_PANEL),NULL,ControlPanelDialogProc);
  const HWND hStatusDlg = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_STATUS_DIALOG),NULL,StatusWindowDialogProc);

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
  cbCfgInput = GetDlgItem(hDlg, IDC_CB_CFG_INPUT);
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
  SendMessage(display.hWnd,WM_SETICON,ICON_SMALL,LPARAM(iconApp));

  UpdateControlPanelDisplay();

  if(Load())
    load4firsttime = false;

  nuonEnv.videoDisplayCycleCount = 0;
  nuonEnv.MPE3wait_fieldCounter = 0;

  while(!bQuit)
  {
    display.MessagePump();

    MSG msg;
    while(PeekMessage(&msg,hDlg,0,0,PM_REMOVE))
      IsDialogMessage(hDlg,&msg);

    while(PeekMessage(&msg,hStatusDlg,0,0,PM_REMOVE))
      IsDialogMessage(hStatusDlg,&msg);

    uint64 cycles = 0;
    while(bRun && !nuonEnv.trigger_render_video)
    {
      cycles++;

      for(int i = 3; i >= 0; --i)
        if(i != 3 || nuonEnv.MPE3wait_fieldCounter == 0) // is MPE3 'stuck' in the VidSync call?
          nuonEnv.mpe[i].FetchDecodeExecute(); // execute a single cycle

      if(nuonEnv.pendingCommRequests)
        DoCommBusController();

      // handle the sysTimer0, sysTimer1 and vidTimer (video refresh at 50/60Hz)
      // note that these all run at the real hostCPU time rate, NOT the actual emulated time/cycles!
      if ((cycles % 500) == 0) // only pull hostCPU timer from time to time, otherwise too costly
      {
        static uint64 last_time0 = useconds_since_start();
        static uint64 last_time1 = useconds_since_start();
        static uint64 last_time2 = useconds_since_start();
        static uint64 last_time3 = useconds_since_start();
        const uint64 new_time = useconds_since_start();

        // sysTimer0 (BIOS, should always be 200Hz)
        if (nuonEnv.timer_rate[0] > 0)
        {
        if (new_time >= last_time0 + nuonEnv.timer_rate[0])
        {
          nuonEnv.ScheduleInterrupt(INT_SYSTIMER0);
          last_time0 = new_time;
        }
        } else last_time0 = new_time;

        // sysTimer1 (User)
        if (nuonEnv.timer_rate[1] > 0)
        {
        if (new_time >= last_time1 + nuonEnv.timer_rate[1])
        {
          nuonEnv.ScheduleInterrupt(INT_SYSTIMER1);
          last_time1 = new_time;
        }
        } else last_time1 = new_time;

        // vidTimer (should always be 50Hz or 60Hz)
        if (nuonEnv.timer_rate[2] > 0)
        {
        if (new_time >= last_time2 + nuonEnv.timer_rate[2])
        {
          IncrementVideoFieldCounter();
          nuonEnv.TriggerVideoInterrupt();
          nuonEnv.trigger_render_video = true;

          // check if the vsync that has passed allows the MPE3 now to continue (if it was blocked by VidSync)
          uint32 fieldCounter = *((uint32*)&nuonEnv.systemBusDRAM[VIDEO_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK]);
          SwapScalarBytes(&fieldCounter);
          if (fieldCounter >= nuonEnv.MPE3wait_fieldCounter)
            nuonEnv.MPE3wait_fieldCounter = 0;

          last_time2 = new_time;
        }
        } else last_time2 = new_time;

        // audTimer
        if (nuonEnv.timer_rate[2] > 0)
        {
        if (nuonEnv.pNuonAudioBuffer && // was nuon audio setup already?
        (new_time >= last_time3 + nuonEnv.timer_rate[2]) && // did enough time pass for new audio data to be ready?
        ((nuonEnv.nuonAudioChannelMode & (ENABLE_WRAP_INT | ENABLE_HALF_INT)) != (nuonEnv.oldNuonAudioChannelMode & (ENABLE_WRAP_INT | ENABLE_HALF_INT))) && // was a new audio interrupt requested?
        ((((nuonEnv.mpe[0].intsrc & nuonEnv.mpe[0].inten1) | (nuonEnv.mpe[1].intsrc & nuonEnv.mpe[1].inten1) | (nuonEnv.mpe[2].intsrc & nuonEnv.mpe[2].inten1) | (nuonEnv.mpe[3].intsrc & nuonEnv.mpe[3].inten1)) & INT_AUDIO) == 0) && // & (INT_AUDIO | INT_COMMXMIT | INT_VIDTIMER) // are any audio interrupts still pending?
        _InterlockedExchange(&nuonEnv.audio_buffer_played,0) == 1) // was an audio buffer already played since last cycle?
        {
          nuonEnv.audio_buffer_offset = (nuonEnv.nuonAudioChannelMode & ENABLE_HALF_INT) ? 0 : (nuonEnv.nuonAudioBufferSize >> 1); //!! ENABLE_HALF_INT leads to better sound in Tetris, although one would assume it should be ENABLE_WRAP_INT here
          nuonEnv.oldNuonAudioChannelMode = nuonEnv.nuonAudioChannelMode;
          nuonEnv.TriggerAudioInterrupt();
          last_time3 = new_time;
        }
        } else last_time3 = new_time;
      }

      nuonEnv.TriggerScheduledInterrupts();
    }

    if(nuonEnv.trigger_render_video) // set by the ~50 or 60Hz timer.cpp routine
    {
      static uint64 old_time = 0;

      static uint64 old_acc_time = 0;
      static string acc_kcs;
      static uint64 acc_cycles = 0;
      acc_cycles += cycles;

      const uint64 new_time = useconds_since_start();
      if (new_time-old_acc_time > 2000000) // update averaged cycles every 2 secs
      {
        acc_kcs = std::to_string((int)(acc_cycles*1000/(double)(new_time-old_acc_time)));
        acc_cycles = 0;
        old_acc_time = new_time;
      }
      const string title = "Nuance (F1 to toggle fullscreen) - "
          + acc_kcs + " " + std::to_string((int)(cycles*1000/(double)(new_time-old_time))) + " Kc/s - "
          + std::to_string((int)(1000000./(double)(new_time-old_time))) + "fps";
      if(old_time != 0 && !display.bFullScreen)
        SetWindowText(display.hWnd,title.c_str());
      old_time = new_time;

      //

      InvalidateRect(display.hWnd, NULL, FALSE);
      //UpdateWindow(display.hWnd);
      nuonEnv.trigger_render_video = false;
    }
  }

/********************************************************************
CLEANUP AND APPLICATION SHUTDOWN CODE
********************************************************************/

  ShowWindow(hDlg,FALSE);
  ShowWindow(hStatusDlg,FALSE);
  EndDialog(hDlg,IDOK);
  EndDialog(hStatusDlg,IDOK);

  VideoCleanup();

  return 0;
}
