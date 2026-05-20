// NuanceMain implementation for Linux
#ifndef _WIN32

#include "basetypes.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include <mutex>
#include <unistd.h>
#include <sys/stat.h>

#include "byteswap.h"
#include "Utility.h"
#include "comm.h"
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
#include "NuanceUI.h"
#include "archive.h"

NuonEnvironment nuonEnv;

extern ControllerData *controller;
extern std::mutex gfx_lock;
extern VidChannel structMainChannel, structOverlayChannel;
extern bool bOverlayChannelActive, bMainChannelActive;
extern vidTexInfo videoTexInfo;

extern void SDL2_SwapWindow();

bool bQuit = false;
bool bRun = false;
std::string g_ISOPath;   // path to mounted ISO (for reading data files)
std::string g_ISOPrefix; // NUON directory name inside ISO (e.g. "NUON")
static bool load4firsttime = true;

GLWindow display;

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

static void ExecuteSingleStep()
{
  nuonEnv.mpe[3].ExecuteSingleStep();
  nuonEnv.mpe[2].ExecuteSingleStep();
  nuonEnv.mpe[1].ExecuteSingleStep();
  nuonEnv.mpe[0].ExecuteSingleStep();
  if(nuonEnv.pendingCommRequests)
    DoCommBusController();
}

void StopEmulation(int mpeIndex)
{
  bRun = false;
}

bool OnDisplayPaint(WPARAM, LPARAM)
{
  if(bRun)
    RenderVideo(display.clientWidth, display.clientHeight);
  else
    glClear(GL_COLOR_BUFFER_BIT);

  SDL2_SwapWindow();
  return true;
}

// NuanceUI_TogglePause defined in NuanceUI.cpp

bool OnDisplayResize(uint16 width, uint16 height)
{
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_FOG);
  glClear(GL_COLOR_BUFFER_BIT);
  OnDisplayPaint(0, 0);
  return false;
}

static void ApplyControllerState(const unsigned int controllerIdx, const uint16 buttons)
{
  if (controller)
    controller[controllerIdx].buttons = SwapBytes(buttons);
}

static void Run()
{
  uint32 bpAddr = 0;
  FILE* inFile;
  if (fopen_s(&inFile, "breakpoint.txt", "r") == 0) {
    fscanf(inFile, "%x", &bpAddr);
    fclose(inFile);
  }
  for (int i = 0; i < 4; i++)
    nuonEnv.mpe[i].breakpointAddress = bpAddr;
  bRun = true;
}

bool Load(const char* file)
{
  if (!file) return false;

  const std::string actualFile = ResolveGameFile(file);
  if (actualFile.empty()) {
    fprintf(stderr, "Cannot resolve / extract: %s\n", file);
    return false;
  }

  // Try loading as NUONROM-DISK/Bles first, then as raw COFF
  bool bSuccess = nuonEnv.mpe[3].LoadNuonRomFile(actualFile.c_str());
  if (!bSuccess) {
    bSuccess = nuonEnv.mpe[3].LoadCoffFile(actualFile.c_str());
    if (!bSuccess) {
      fprintf(stderr, "Cannot open file or Invalid COFF/NUONROM-DISK/Bles file: %s\n", actualFile.c_str());
      return false;
    }
  }
  fprintf(stderr, "Loaded successfully: %s\n", actualFile.c_str());

  if (bSuccess) {
    nuonEnv.SetDVDBaseFromFileName(actualFile.c_str());
    nuonEnv.mpe[3].Go();
    Run();
    return true;
  }
  return false;
}

int main(int argc, char* argv[])
{
#ifdef USE_ASMJIT
  extern bool asmjit_selftest();
  asmjit_selftest();
#endif

  init_supported_CPU_extensions();

  GenerateMirrorLookupTable();
  GenerateSaturateColorTables();

  nuonEnv.Init();

  display.applyControllerState = ApplyControllerState;
  display.resizeHandler = OnDisplayResize;
  display.paintHandler = OnDisplayPaint;

  display.Create();

  // Initialize OpenGL viewport
  OnDisplayResize(display.clientWidth, display.clientHeight);

  if (argc > 1) {
    if (Load(argv[1]))
      load4firsttime = false;
  } else {
    printf("Usage: nuance <rom_file>\n");
    printf("  Supported formats: .run, .cd, .nuon, .cof, .iso, .img, .zip, .7z, .chd\n");
  }

  nuonEnv.videoDisplayCycleCount = 0;
  nuonEnv.MPE3wait_fieldCounter = 0;

  while (!bQuit)
  {
    display.MessagePump();

    uint64 cycles = 0;
    while (bRun && !nuonEnv.trigger_render_video)
    {
      cycles++;

      for (int i = 3; i >= 0; --i)
        if (i != 3 || nuonEnv.MPE3wait_fieldCounter == 0)
          nuonEnv.mpe[i].FetchDecodeExecute();

      if (nuonEnv.pendingCommRequests)
        DoCommBusController();

      if ((cycles % 500) == 0)
      {
        static uint64 last_time0 = useconds_since_start();
        static uint64 last_time1 = useconds_since_start();
        static uint64 last_time2 = useconds_since_start();
        static uint64 last_time3 = useconds_since_start();
        const uint64 new_time = useconds_since_start();

        if (nuonEnv.timer_rate[0] > 0) {
          if (new_time >= last_time0 + (uint64)nuonEnv.timer_rate[0]) {
            nuonEnv.ScheduleInterrupt(INT_SYSTIMER0);
            last_time0 = new_time;
          }
        } else last_time0 = new_time;

        if (nuonEnv.timer_rate[1] > 0) {
          if (new_time >= last_time1 + (uint64)nuonEnv.timer_rate[1]) {
            nuonEnv.ScheduleInterrupt(INT_SYSTIMER1);
            last_time1 = new_time;
          }
        } else last_time1 = new_time;

        if (nuonEnv.timer_rate[2] > 0) {
          if (new_time >= last_time2 + (uint64)nuonEnv.timer_rate[2]) {
            IncrementVideoFieldCounter();
            nuonEnv.TriggerVideoInterrupt();
            nuonEnv.trigger_render_video = true;
            const uint32 fieldCounter = SwapBytes(*((uint32*)&nuonEnv.systemBusDRAM[VIDEO_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK]));
            if (fieldCounter >= nuonEnv.MPE3wait_fieldCounter)
              nuonEnv.MPE3wait_fieldCounter = 0;
            last_time2 = new_time;
          }
        } else last_time2 = new_time;

        // audTimer - push one Nuon audio period into the host audio ring (byte-swapped), advance the
        // DMA half pointer, fire INT_AUDIO. Ring full -> skip this iteration
        if (nuonEnv.timer_rate[2] > 0) {
          if (nuonEnv.pNuonAudioBuffer &&
              (new_time >= last_time3 + (uint64)nuonEnv.timer_rate[2]) &&
              ((nuonEnv.nuonAudioChannelMode & (ENABLE_WRAP_INT | ENABLE_HALF_INT)) != (nuonEnv.oldNuonAudioChannelMode & (ENABLE_WRAP_INT | ENABLE_HALF_INT))) &&
              ((((nuonEnv.mpe[0].intsrc & nuonEnv.mpe[0].inten1) | (nuonEnv.mpe[1].intsrc & nuonEnv.mpe[1].inten1) | (nuonEnv.mpe[2].intsrc & nuonEnv.mpe[2].inten1) | (nuonEnv.mpe[3].intsrc & nuonEnv.mpe[3].inten1)) & INT_AUDIO) == 0)) {
            if (nuonEnv.TryPushAudioPeriod())
              last_time3 = new_time;
          }
        } else last_time3 = new_time;
      }

      nuonEnv.TriggerScheduledInterrupts();
    }

    if (nuonEnv.trigger_render_video)
    {
      static uint64 old_time = 0;
      static uint64 old_acc_time = 0;
      static int acc_kcs = 0;
      static uint64 acc_cycles = 0;
      acc_cycles += cycles;

      const uint64 new_time = useconds_since_start();
      if (new_time - old_acc_time > 2000000) {
        acc_kcs = (int)(acc_cycles * 1000 / (double)(new_time - old_acc_time));
        acc_cycles = 0;
        old_acc_time = new_time;
      }
      int fps = (old_time != 0) ? (int)(1000000.0 / (double)(new_time - old_time)) : 0;
      old_time = new_time;

      NuanceUI_UpdateTitle(acc_kcs, fps);
      OnDisplayPaint(0, 0);
      nuonEnv.trigger_render_video = false;
    }
    else if (!bRun)
    {
      OnDisplayPaint(0, 0);
      usleep(16000);
    }
  }

  VideoCleanup();
  display.CleanUp();
  CleanupArchives();

  return 0;
}

#endif // !_WIN32
