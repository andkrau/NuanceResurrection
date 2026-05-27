// Shared NUON emulator core used by both the standalone Linux frontend
// (NuanceMain_linux.cpp) and the libretro core (libretro.cpp). Each
// frontend handles its own windowing / input / audio sink and just
// drives the emulator through this API.
#ifndef EMULATOR_CORE_H
#define EMULATOR_CORE_H

#include "basetypes.h"
#include "NuanceMain.h"   // extern bool bQuit; extern void StopEmulation(int);
#include "NuonEnvironment.h"
#include <string>

extern NuonEnvironment nuonEnv;
extern bool bRun;
extern std::string g_ISOPath;
extern std::string g_ISOPrefix;

namespace EmulatorCore {

// CPU extension probe, ALU lookup tables, NuonEnvironment::Init.
void Init();

// LoadNuonRomFile -> LoadCoffFile fallback, SetDVDBaseFromFileName, MPE3 Go.
// Returns false if neither loader recognised the file.
bool LoadGame(const char* path);

// Byte-swap and write to controller[controllerIdx].buttons. Safe pre-Init.
void ApplyController(unsigned controllerIdx, uint16 buttons);

// Drive all 4 MPEs until trigger_render_video is set or, if budget_us > 0,
// until that many microseconds of wall time have elapsed since entry.
// push_audio = also push one Nuon audio period to the host ring per video
// field (the standalone build's miniaudio sink consumes those periods;
// libretro uses DrainAudioRing from retro_run instead and passes false).
// Returns the number of inner-loop iterations executed in this call,
// useful for the standalone frontend's FPS / kHz title-bar readout.
uint32 RunUntilVideoFrame(uint64 budget_us, bool push_audio);

// VideoCleanup + CleanupArchives. Idempotent on second call.
void Shutdown();

} // namespace EmulatorCore

#endif // EMULATOR_CORE_H
