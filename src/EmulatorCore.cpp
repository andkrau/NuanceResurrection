// Shared NUON emulator core. See EmulatorCore.h for the API contract.
#include "EmulatorCore.h"

#include "Utility.h"
#include "ExecuteMEM.h"
#include "comm.h"
#include "byteswap.h"
#include "mpe.h"
#include "video.h"
#include "timer.h"
#include "Bios.h"
#include "NuonMemoryMap.h"
#include "joystick.h"
#include "archive.h"

#include <cstdio>
#include <cstdlib>

// Definitions of the single-instance globals declared in EmulatorCore.h.
// These used to live (duplicated) in each frontend translation unit.
NuonEnvironment nuonEnv;
bool bQuit = false;
bool bRun  = false;
std::string g_ISOPath;
std::string g_ISOPrefix;

extern ControllerData* controller;

// Satisfies the `extern void StopEmulation(int)` declaration in NuanceMain.h.
void StopEmulation(int /*mpeIndex*/)
{
    bRun = false;
}

namespace EmulatorCore {

void Init()
{
    init_supported_CPU_extensions();
    GenerateMirrorLookupTable();
    GenerateSaturateColorTables();
    nuonEnv.Init();
}

bool LoadGame(const char* path)
{
    if (!path)
        return false;
    bool ok = nuonEnv.mpe[3].LoadNuonRomFile(path);
    if (!ok)
        ok = nuonEnv.mpe[3].LoadCoffFile(path);
    if (!ok)
        return false;
    nuonEnv.SetDVDBaseFromFileName(path);
    nuonEnv.mpe[3].Go();
    return true;
}

void ApplyController(unsigned controllerIdx, uint16 buttons)
{
    if (controller)
        controller[controllerIdx].buttons = SwapBytes(buttons);
}

uint32 RunUntilVideoFrame(uint64 budget_us, bool push_audio)
{
    // Wall-time anchors for the three Nuon system timers and the audio push
    // cadence. Static so the schedule survives across calls (each call is
    // one video field of work).
    static uint64 last_time0 = useconds_since_start();
    static uint64 last_time1 = useconds_since_start();
    static uint64 last_time2 = useconds_since_start();
    static uint64 last_time3 = useconds_since_start();

    const uint64 entry_us = useconds_since_start();
    uint32 cycles = 0;
    while (bRun && !nuonEnv.trigger_render_video) {
        cycles++;

        for (int i = 3; i >= 0; --i)
            if (i != 3 || nuonEnv.MPE3wait_fieldCounter == 0)
                nuonEnv.mpe[i].FetchDecodeExecute();

        if (nuonEnv.pendingCommRequests)
            DoCommBusController();

        if ((cycles % 500) == 0) {
            const uint64 now = useconds_since_start();

            // Libretro frame budget: return control to retroarch even if
            // no video field fired yet, so the host frontend can drive
            // its own 60Hz pacing.
            if (budget_us > 0 && (now - entry_us) >= budget_us)
                break;

            if (nuonEnv.timer_rate[0] > 0) {
                if (now >= last_time0 + (uint64)nuonEnv.timer_rate[0]) {
                    nuonEnv.ScheduleInterrupt(INT_SYSTIMER0);
                    last_time0 = now;
                }
            } else {
                last_time0 = now;
            }

            if (nuonEnv.timer_rate[1] > 0) {
                if (now >= last_time1 + (uint64)nuonEnv.timer_rate[1]) {
                    nuonEnv.ScheduleInterrupt(INT_SYSTIMER1);
                    last_time1 = now;
                }
            } else {
                last_time1 = now;
            }

            if (nuonEnv.timer_rate[2] > 0) {
                if (now >= last_time2 + (uint64)nuonEnv.timer_rate[2]) {
                    IncrementVideoFieldCounter();
                    nuonEnv.TriggerVideoInterrupt();
                    nuonEnv.trigger_render_video = true;
                    const uint32 fieldCounter = SwapBytes(*((uint32*)&nuonEnv.systemBusDRAM[VIDEO_FIELD_COUNTER_ADDRESS & SYSTEM_BUS_VALID_MEMORY_MASK]));
                    if (fieldCounter >= nuonEnv.MPE3wait_fieldCounter)
                        nuonEnv.MPE3wait_fieldCounter = 0;
                    last_time2 = now;
                }
            } else {
                last_time2 = now;
            }

            // Audio period push (standalone only — libretro consumes via
            // DrainAudioRing). One period per video field, gated on a
            // channel-mode toggle so we only push when NISE is actively
            // ping-ponging buffers; INT_AUDIO mask check avoids pushing
            // while a previous period is still being acknowledged.
            if (push_audio) {
                if (nuonEnv.timer_rate[2] > 0) {
                    if (nuonEnv.pNuonAudioBuffer &&
                        (now >= last_time3 + (uint64)nuonEnv.timer_rate[2]) &&
                        ((nuonEnv.nuonAudioChannelMode & (ENABLE_WRAP_INT | ENABLE_HALF_INT)) != (nuonEnv.oldNuonAudioChannelMode & (ENABLE_WRAP_INT | ENABLE_HALF_INT))) &&
                        ((((nuonEnv.mpe[0].intsrc & nuonEnv.mpe[0].inten1) |
                           (nuonEnv.mpe[1].intsrc & nuonEnv.mpe[1].inten1) |
                           (nuonEnv.mpe[2].intsrc & nuonEnv.mpe[2].inten1) |
                           (nuonEnv.mpe[3].intsrc & nuonEnv.mpe[3].inten1)) & INT_AUDIO) == 0)) {
                        if (nuonEnv.TryPushAudioPeriod())
                            last_time3 = now;
                    }
                } else {
                    last_time3 = now;
                }
            }
        }

        nuonEnv.TriggerScheduledInterrupts();
    }
    return cycles;
}

void Shutdown()
{
    VideoCleanup();
    CleanupArchives();
}

} // namespace EmulatorCore
