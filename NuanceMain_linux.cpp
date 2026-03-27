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
#include "iso9660.h"

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

static std::vector<std::string> tempMountPoints;

static std::vector<std::string> tempDirs; // non-mount temp dirs to clean up

static void CleanupMounts()
{
  for (auto it = tempMountPoints.rbegin(); it != tempMountPoints.rend(); ++it) {
    std::string cmd = "fusermount -uz \"" + *it + "\" 2>/dev/null";
    system(cmd.c_str());
    rmdir(it->c_str());
  }
  tempMountPoints.clear();
  for (auto& d : tempDirs) {
    std::string cmd = "rm -rf \"" + d + "\" 2>/dev/null";
    system(cmd.c_str());
  }
  tempDirs.clear();
}

static std::string MountPath(const char* archivePath)
{
  char tmpl[] = "/tmp/nuance_XXXXXX";
  char* dir = mkdtemp(tmpl);
  if (!dir) return "";
  std::string mountPoint = dir;

  // Try fuse-zip for .zip, archivemount for others, fuseiso for .iso
  std::string path(archivePath);
  std::string cmd;
  int ret = -1;

  // Detect type
  size_t len = path.size();
  bool isZip = (len > 4 && strcasecmp(path.c_str() + len - 4, ".zip") == 0);
  bool isIso = (len > 4 && (strcasecmp(path.c_str() + len - 4, ".iso") == 0 ||
                             strcasecmp(path.c_str() + len - 4, ".img") == 0));

  if (isIso) {
    // For ISO: try fuseiso first, then udisksctl, then extract NUON dir via 7z
    cmd = "fuseiso \"" + path + "\" \"" + mountPoint + "\" 2>/dev/null";
    ret = system(cmd.c_str());
    if (ret != 0) {
      // Extract only NUON directory from ISO (much faster than full mount)
      cmd = "7z x -y -o\"" + mountPoint + "\" \"" + path + "\" NUON/ nuon/ Nuon/ > /dev/null 2>&1";
      ret = system(cmd.c_str());
      if (ret == 0) {
        // 7z extracted, not mounted — don't add to mount list, just use the dir
        fprintf(stderr, "Extracted NUON dir from ISO: %s\n", path.c_str());
        return mountPoint;
      }
    }
  }
  if (ret != 0 && isZip) {
    // mount-zip supports random access (seekable), fuse-zip does not
    cmd = "mount-zip \"" + path + "\" \"" + mountPoint + "\" 2>/dev/null";
    ret = system(cmd.c_str());
  }
  if (ret != 0 && isZip) {
    cmd = "fuse-zip -r \"" + path + "\" \"" + mountPoint + "\" 2>/dev/null";
    ret = system(cmd.c_str());
  }
  if (ret != 0) {
    cmd = "archivemount -o readonly \"" + path + "\" \"" + mountPoint + "\" 2>/dev/null";
    ret = system(cmd.c_str());
  }

  if (ret != 0) {
    fprintf(stderr, "Failed to mount: %s\n", archivePath);
    rmdir(mountPoint.c_str());
    return "";
  }

  fprintf(stderr, "Mounted: %s -> %s\n", archivePath, mountPoint.c_str());
  tempMountPoints.push_back(mountPoint);
  return mountPoint;
}

static std::string popen_line(const std::string& cmd)
{
  FILE* fp = popen(cmd.c_str(), "r");
  if (!fp) return "";
  char buf[1024] = {};
  if (fgets(buf, sizeof(buf), fp)) {
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
  }
  pclose(fp);
  return buf;
}

static std::string MountAndFind(const char* archivePath)
{
  fprintf(stderr, "Mounting: %s\n", archivePath);

  std::string mp = MountPath(archivePath);
  if (mp.empty()) return "";

  // Look for nuon.run or NUON.CD
  std::string result = popen_line("find \"" + mp + "\" -maxdepth 5 \\( -iname 'nuon.run' -o -iname 'NUON.CD' \\) -print -quit 2>/dev/null");

  // If not found, look for an ISO inside and read it via ISO9660 parser
  if (result.empty()) {
    std::string iso = popen_line("find \"" + mp + "\" -maxdepth 2 \\( -iname '*.iso' -o -iname '*.img' \\) -print -quit 2>/dev/null");
    if (!iso.empty()) {
      fprintf(stderr, "Found ISO inside: %s\n", iso.c_str());
      ISO9660Reader reader;
      if (reader.open(iso.c_str())) {
        // Extract only the boot file (nuon.run / NUON.CD), read data files from ISO at runtime
        const char* tryPaths[] = {
          "NUON/nuon.run", "NUON/NUON.CD", "NUON/cd_app.cof",
          "nuon/nuon.run", "nuon/NUON.CD", nullptr
        };
        uint32_t lba, fsize;
        const char* foundPath = nullptr;
        for (const char** f = tryPaths; *f; f++) {
          if (reader.findFile(*f, lba, fsize)) { foundPath = *f; break; }
        }

        if (foundPath) {
          char tmpl2[] = "/tmp/nuance_iso_XXXXXX";
          char* isoDir = mkdtemp(tmpl2);
          if (isoDir) {
            std::string ff(foundPath);
            std::string subdir = ff.substr(0, ff.find('/'));
            std::string nuonDir = std::string(isoDir) + "/" + subdir;
            mkdir(nuonDir.c_str(), 0755);
            tempDirs.push_back(isoDir);

            std::string bootName = ff.substr(ff.find('/') + 1);
            std::string dstPath = nuonDir + "/" + bootName;
            reader.extractFile(foundPath, dstPath.c_str());
            fprintf(stderr, "  Extracted boot: %s\n", foundPath);

            // Store ISO path globally so MediaOpen can read data files from ISO
            extern std::string g_ISOPath;
            extern std::string g_ISOPrefix;
            g_ISOPath = iso;
            g_ISOPrefix = subdir;
            fprintf(stderr, "  ISO data path: %s/%s/\n", iso.c_str(), subdir.c_str());

            result = dstPath;
          }
        }
        reader.close();
      }
    }
  }

  // Fallback: cd_app.cof
  if (result.empty())
    result = popen_line("find \"" + mp + "\" -maxdepth 5 -iname 'cd_app.cof' -print -quit 2>/dev/null");

  if (result.empty()) {
    fprintf(stderr, "No NUON game found in: %s\n", archivePath);
    return "";
  }

  fprintf(stderr, "Found: %s\n", result.c_str());
  return result;
}

static bool IsISOFile(const char* path)
{
  size_t len = strlen(path);
  if (len < 4) return false;
  const char* ext = path + len - 4;
  return (strcasecmp(ext, ".iso") == 0 || strcasecmp(ext, ".img") == 0 ||
          (len >= 5 && strcasecmp(path + len - 5, ".chd") == 0));
}

static bool IsArchiveFile(const char* path)
{
  size_t len = strlen(path);
  if (len < 3) return false;
  const char* ext = path + len;
  // Check common archive extensions
  for (const char* e : {".zip", ".7z", ".gz", ".rar", ".bz2", ".xz"}) {
    size_t elen = strlen(e);
    if (len >= elen && strcasecmp(path + len - elen, e) == 0) return true;
  }
  return false;
}

bool Load(const char* file)
{
  if (!file) return false;

  std::string actualFile = file;

  // Handle ISO/IMG files — extract NUON.CD from them
  if (IsISOFile(file) || IsArchiveFile(file)) {
    fprintf(stderr, "Extracting: %s\n", file);
    actualFile = MountAndFind(file);
    if (actualFile.empty()) return false;
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
    printf("  Supported formats: .nuon, .cof\n");
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

        if (nuonEnv.timer_rate[2] > 0) {
          if (nuonEnv.pNuonAudioBuffer &&
              (new_time >= last_time3 + (uint64)nuonEnv.timer_rate[2]) &&
              ((nuonEnv.nuonAudioChannelMode & (ENABLE_WRAP_INT | ENABLE_HALF_INT)) != (nuonEnv.oldNuonAudioChannelMode & (ENABLE_WRAP_INT | ENABLE_HALF_INT))) &&
              ((((nuonEnv.mpe[0].intsrc & nuonEnv.mpe[0].inten1) | (nuonEnv.mpe[1].intsrc & nuonEnv.mpe[1].inten1) | (nuonEnv.mpe[2].intsrc & nuonEnv.mpe[2].inten1) | (nuonEnv.mpe[3].intsrc & nuonEnv.mpe[3].inten1)) & INT_AUDIO) == 0) &&
              _InterlockedExchange(&nuonEnv.audio_buffer_played, 0) == 1) {
            nuonEnv.audio_buffer_offset = (nuonEnv.nuonAudioChannelMode & ENABLE_HALF_INT) ? 0 : (nuonEnv.nuonAudioBufferSize >> 1);
            nuonEnv.oldNuonAudioChannelMode = nuonEnv.nuonAudioChannelMode;
            nuonEnv.TriggerAudioInterrupt();
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
  CleanupMounts();

  return 0;
}

#endif // !_WIN32
