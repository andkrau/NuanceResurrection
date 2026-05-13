// Standalone Linux frontend for the NUON emulator. The emulator loop,
// globals and load/init helpers live in EmulatorCore (shared with the
// libretro core in libretro.cpp); this file only handles the X11/SDL2
// window and the title-bar FPS readout.
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
#include "EmulatorCore.h"
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

extern ControllerData *controller;
extern std::mutex gfx_lock;
extern VidChannel structMainChannel, structOverlayChannel;
extern bool bOverlayChannelActive, bMainChannelActive;
extern vidTexInfo videoTexInfo;

extern void SDL2_SwapWindow();

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
  EmulatorCore::ApplyController(controllerIdx, buttons);
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

  if (!EmulatorCore::LoadGame(actualFile.c_str())) {
    fprintf(stderr, "Cannot open file or Invalid COFF/NUONROM-DISK/Bles file: %s\n", actualFile.c_str());
    return false;
  }
  fprintf(stderr, "Loaded successfully: %s\n", actualFile.c_str());

  Run();
  return true;
}

int main(int argc, char* argv[])
{
#ifdef USE_ASMJIT
  extern bool asmjit_selftest();
  asmjit_selftest();
#endif

  EmulatorCore::Init();

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
    printf("  Supported formats: .run, .cd, .nuon, .cof, .iso, .img, .zip, .7z\n");
  }

  nuonEnv.videoDisplayCycleCount = 0;
  nuonEnv.MPE3wait_fieldCounter = 0;

  while (!bQuit)
  {
    display.MessagePump();

    // Run the shared emulator loop until the next video field.
    // Standalone: no time budget (budget_us=0), push audio periods so
    // miniaudio's worker thread can consume them.
    const uint64 cycles = EmulatorCore::RunUntilVideoFrame(/*budget_us=*/0,
                                                           /*push_audio=*/true);

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

  display.CleanUp();
  EmulatorCore::Shutdown();

  return 0;
}

#endif // !_WIN32
