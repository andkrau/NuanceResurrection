// UI for NuanceResurrection (Linux)
// Uses kdialog/zenity for dialogs, keyboard shortcuts for controls
#ifndef _WIN32

#include "basetypes.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "mpe.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "video.h"
#include "NuanceUI.h"

extern NuonEnvironment nuonEnv;
extern bool bQuit;
extern bool Load(const char* file);
extern bool bRun;

#include "comm.h"

static bool g_UIInitialized = false;
static bool g_ShowUI = false;
static Display* g_Display = nullptr;
static Window g_Window = 0;

void NuanceUI_Init(void* display, unsigned long window)
{
    g_Display = (Display*)display;
    g_Window = (Window)window;
    g_UIInitialized = true;
    g_ShowUI = false;
}

void NuanceUI_Shutdown()
{
    g_UIInitialized = false;
}

// Helper: run a command and return its stdout
static std::string popen_read(const char* cmd)
{
    FILE* fp = popen(cmd, "r");
    if (!fp) return "";
    char buf[4096] = {};
    std::string result;
    while (fgets(buf, sizeof(buf), fp))
        result += buf;
    pclose(fp);
    // trim trailing newline
    while (!result.empty() && result.back() == '\n') result.pop_back();
    return result;
}

// Helper: show dialog (non-blocking via fork)
static void show_dialog_async(const std::string& text, const std::string& title)
{
    std::string cmd = "kdialog --msgbox '" + text + "' --title '" + title + "' 2>/dev/null &";
    system(cmd.c_str());
}

static void OpenFileDialog()
{
    std::string path = popen_read(
        "kdialog --getopenfilename ~ 'NUON files (*.run *.cof *.nuon *.cd *.zip *.7z *.iso *.img *.chd)' 2>/dev/null"
        " || zenity --file-selection --title='Load NUON Game' 2>/dev/null");

    if (!path.empty()) {
        fprintf(stderr, "Loading: %s\n", path.c_str()); fflush(stderr);
        Load(path.c_str());
    }
}

static bool GetMPERunStatus(uint32 which)
{
    return (nuonEnv.mpe[which & 0x03].mpectl & MPECTRL_MPEGO) != 0;
}

static void ShowStatusDialog()
{
    char buf[4096];
    snprintf(buf, sizeof(buf),
        "=== MPE Status ===\\n"
        "MPE0: $%08X %s\\n"
        "MPE1: $%08X %s\\n"
        "MPE2: $%08X %s\\n"
        "MPE3: $%08X %s\\n\\n"
        "=== Comm Bus ===\\n"
        "Pending requests: %u\\n\\n"
        "=== Compiler ===\\n"
        "Cache flushes: %u %u %u %u\\n"
        "Non-compilable: %u %u %u %u\\n"
        "Overlays: %u %u %u %u",
        GetMPERunStatus(0) ? nuonEnv.mpe[0].pcexec : 0, GetMPERunStatus(0) ? "RUN" : "---",
        GetMPERunStatus(1) ? nuonEnv.mpe[1].pcexec : 0, GetMPERunStatus(1) ? "RUN" : "---",
        GetMPERunStatus(2) ? nuonEnv.mpe[2].pcexec : 0, GetMPERunStatus(2) ? "RUN" : "---",
        GetMPERunStatus(3) ? nuonEnv.mpe[3].pcexec : 0, GetMPERunStatus(3) ? "RUN" : "---",
        nuonEnv.pendingCommRequests,
        nuonEnv.mpe[0].numNativeCodeCacheFlushes, nuonEnv.mpe[1].numNativeCodeCacheFlushes,
        nuonEnv.mpe[2].numNativeCodeCacheFlushes, nuonEnv.mpe[3].numNativeCodeCacheFlushes,
        nuonEnv.mpe[0].numNonCompilablePackets, nuonEnv.mpe[1].numNonCompilablePackets,
        nuonEnv.mpe[2].numNonCompilablePackets, nuonEnv.mpe[3].numNonCompilablePackets,
        nuonEnv.mpe[0].overlayManager.GetOverlaysInUse(), nuonEnv.mpe[1].overlayManager.GetOverlaysInUse(),
        nuonEnv.mpe[2].overlayManager.GetOverlaysInUse(), nuonEnv.mpe[3].overlayManager.GetOverlaysInUse());

    show_dialog_async(buf, "Nuance Status");
}

static void ShowRegistersDialog()
{
    // Show MPE3 registers (main processor) - could add selection later
    static int mpeIdx = 3;
    const MPE& mpe = nuonEnv.mpe[mpeIdx];

    char buf[4096];
    snprintf(buf, sizeof(buf),
        "=== MPE%d Registers ===\\n"
        "pcexec: $%08X  cc: $%08X\\n"
        "rc0: $%08X  rc1: $%08X  sp: $%08X\\n\\n"
        "v0: %08X %08X %08X %08X\\n"
        "v1: %08X %08X %08X %08X\\n"
        "v2: %08X %08X %08X %08X\\n"
        "v3: %08X %08X %08X %08X\\n"
        "v4: %08X %08X %08X %08X\\n"
        "v5: %08X %08X %08X %08X\\n"
        "v6: %08X %08X %08X %08X\\n"
        "v7: %08X %08X %08X %08X\\n\\n"
        "rx/ry/ru/rv: %08X %08X %08X %08X\\n"
        "rz/rzi1/rzi2: %08X %08X %08X\\n"
        "intctl: $%08X  intsrc: $%08X\\n"
        "inten1: $%08X  inten2sel: $%08X",
        mpeIdx, mpe.pcexec, mpe.cc,
        mpe.rc0, mpe.rc1, mpe.sp,
        mpe.regs[0], mpe.regs[1], mpe.regs[2], mpe.regs[3],
        mpe.regs[4], mpe.regs[5], mpe.regs[6], mpe.regs[7],
        mpe.regs[8], mpe.regs[9], mpe.regs[10], mpe.regs[11],
        mpe.regs[12], mpe.regs[13], mpe.regs[14], mpe.regs[15],
        mpe.regs[16], mpe.regs[17], mpe.regs[18], mpe.regs[19],
        mpe.regs[20], mpe.regs[21], mpe.regs[22], mpe.regs[23],
        mpe.regs[24], mpe.regs[25], mpe.regs[26], mpe.regs[27],
        mpe.regs[28], mpe.regs[29], mpe.regs[30], mpe.regs[31],
        mpe.rx, mpe.ry, mpe.ru, mpe.rv,
        mpe.rz, mpe.rzi1, mpe.rzi2,
        mpe.intctl, mpe.intsrc,
        mpe.inten1, mpe.inten2sel);

    show_dialog_async(buf, "Nuance Registers");
}

static void ShowKprintfLog()
{
    std::string log;
    if (nuonEnv.kprintRingBuffer) {
        size_t idx = nuonEnv.kprintCurrentLine;
        for (size_t n = 0; n < NuonEnvironment::KPRINT_RING_SIZE; n++) {
            idx = (idx + 1) % NuonEnvironment::KPRINT_RING_SIZE;
            if (nuonEnv.kprintRingBuffer[idx][0])
                log += std::string(nuonEnv.kprintRingBuffer[idx]) + "\\n";
        }
    }
    if (log.empty()) log = "(empty)";
    show_dialog_async(log, "kprintf Log");
}

static void DumpMPEs()
{
    for (int i = 0; i < 4; i++) {
        char fname[32];
        snprintf(fname, sizeof(fname), "mpe%d.bin", i);
        FILE* f = fopen(fname, "wb");
        if (f) {
            fwrite(nuonEnv.mpe[i].dtrom, 1, MPE_LOCAL_MEMORY_SIZE, f);
            fclose(f);
        }
    }
    show_dialog_async("MPE memory dumped to mpe0.bin - mpe3.bin", "Dump Complete");
}

static std::string VKName(int vk)
{
    if (vk >= 'A' && vk <= 'Z') return std::string(1, (char)vk);
    if (vk >= '0' && vk <= '9') return std::string(1, (char)vk);
    switch (vk) {
        case 0x25: return "Left";  case 0x26: return "Up";
        case 0x27: return "Right"; case 0x28: return "Down";
        case 0x0D: return "Enter"; case 0x20: return "Space";
        case 0x1B: return "Esc";
        default: char b[16]; snprintf(b, sizeof(b), "0x%02X", vk); return b;
    }
}

static int XKeyToVK(KeySym key)
{
    if (key >= XK_a && key <= XK_z) return 'A' + (key - XK_a);
    if (key >= XK_A && key <= XK_Z) return 'A' + (key - XK_A);
    if (key >= XK_0 && key <= XK_9) return '0' + (key - XK_0);
    switch (key) {
        case XK_Up: return 0x26; case XK_Down: return 0x28;
        case XK_Left: return 0x25; case XK_Right: return 0x27;
        case XK_Return: return 0x0D; case XK_space: return 0x20;
        case XK_Escape: return 0x1B;
        default: return key & 0xFF;
    }
}

// Button indices to configure, in order (skip unused 6,7)
static const int g_ConfigOrder[] = { 9, 11, 10, 8, 14, 3, 12, 13, 4, 5, 1, 15, 2, 0 };
static const char* g_ConfigNames[] = {
    [0]="C-Right", [1]="C-Up", [2]="C-Left", [3]="B", [4]="R", [5]="L",
    [6]="", [7]="",
    [8]="D-Right", [9]="D-Up", [10]="D-Left", [11]="D-Down",
    [12]="NUON", [13]="Start", [14]="A", [15]="C-Down"
};

static void ShowInputConfig()
{
    if (!g_Display) return;

    int numBtns = sizeof(g_ConfigOrder) / sizeof(g_ConfigOrder[0]);

    // Create config window
    int winW = 400, winH = 50 + numBtns * 22 + 30;
    int screen = DefaultScreen(g_Display);
    Window configWin = XCreateSimpleWindow(g_Display, RootWindow(g_Display, screen),
        200, 200, winW, winH, 1,
        BlackPixel(g_Display, screen), 0x1a1a2e);
    XStoreName(g_Display, configWin, "Configure Input - Press key for each button (ESC to cancel)");
    XSelectInput(g_Display, configWin, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(g_Display, configWin);
    XFlush(g_Display);

    // Load font
    XFontStruct* font = XLoadQueryFont(g_Display, "-*-fixed-medium-r-*-*-14-*-*-*-*-*-*-*");
    if (!font) font = XLoadQueryFont(g_Display, "fixed");
    GC gc = XCreateGC(g_Display, configWin, 0, nullptr);
    XSetFont(g_Display, gc, font->fid);

    // Store new mappings (start with current)
    int newVK[16];
    for (int i = 0; i < 16; i++) {
        ControllerButtonMapping m = nuonEnv.GetMappingForCTRLRBitnum(i);
        newVK[i] = (m.type == InputManager::KEY) ? m.idx : 0;
    }

    int currentBtn = 0;
    bool cancelled = false;
    bool done = false;

    auto redraw = [&]() {
        XClearWindow(g_Display, configWin);
        XSetForeground(g_Display, gc, 0xFFFF80); // yellow
        XDrawString(g_Display, configWin, gc, 10, 20, "Configure Input - Press key for each button:", 45);

        for (int i = 0; i < numBtns; i++) {
            int idx = g_ConfigOrder[i];
            int y = 45 + i * 22;

            // Highlight current
            if (i == currentBtn && !done) {
                XSetForeground(g_Display, gc, 0x304080);
                XFillRectangle(g_Display, configWin, gc, 5, y - 12, winW - 10, 20);
            }

            // Button name
            if (i == currentBtn && !done)
                XSetForeground(g_Display, gc, 0x80FF80); // green = active
            else if (i < currentBtn || done)
                XSetForeground(g_Display, gc, 0xC0C0C0); // gray = done
            else
                XSetForeground(g_Display, gc, 0x808080); // dim = pending

            char line[128];
            if (i < currentBtn || done)
                snprintf(line, sizeof(line), "%-10s = %s", g_ConfigNames[idx], VKName(newVK[idx]).c_str());
            else if (i == currentBtn)
                snprintf(line, sizeof(line), "%-10s = ??? (press key)", g_ConfigNames[idx]);
            else
                snprintf(line, sizeof(line), "%-10s = %s", g_ConfigNames[idx], VKName(newVK[idx]).c_str());

            XDrawString(g_Display, configWin, gc, 15, y + 2, line, strlen(line));
        }

        if (done) {
            XSetForeground(g_Display, gc, 0x80FF80);
            int y = 45 + numBtns * 22 + 10;
            const char* msg = "Done! Press Enter to save, Escape to cancel.";
            XDrawString(g_Display, configWin, gc, 15, y, msg, strlen(msg));
        }

        XFlush(g_Display);
    };

    // Wait for MapNotify before drawing
    while (true) {
        XEvent ev;
        XNextEvent(g_Display, &ev);
        if (ev.type == MapNotify) break;
    }

    redraw();

    // Event loop
    while (!cancelled && !(done && false)) {
        XEvent ev;
        XNextEvent(g_Display, &ev);

        if (ev.type == Expose) {
            redraw();
        }
        else if (ev.type == KeyPress) {
            KeySym key = XLookupKeysym(&ev.xkey, 0);

            if (key == XK_Escape) {
                if (done) { cancelled = true; break; }
                else { cancelled = true; break; }
            }

            if (done) {
                if (key == XK_Return) break; // save
                if (key == XK_Escape) { cancelled = true; break; }
                continue;
            }

            // Assign key to current button
            int vk = XKeyToVK(key);
            int idx = g_ConfigOrder[currentBtn];
            newVK[idx] = vk;
            currentBtn++;

            if (currentBtn >= numBtns) done = true;
            redraw();

            if (done) {
                // Wait for Enter or Escape
                while (true) {
                    XNextEvent(g_Display, &ev);
                    if (ev.type == Expose) redraw();
                    if (ev.type == KeyPress) {
                        key = XLookupKeysym(&ev.xkey, 0);
                        if (key == XK_Return) break;
                        if (key == XK_Escape) { cancelled = true; break; }
                    }
                }
                break;
            }
        }
    }

    XFreeFont(g_Display, font);
    XFreeGC(g_Display, gc);
    XDestroyWindow(g_Display, configWin);
    XFlush(g_Display);

    if (!cancelled) {
        // Apply new mappings
        for (int i = 0; i < 16; i++) {
            if (i == 6 || i == 7) continue;
            nuonEnv.SetControllerButtonMapping(i, ControllerButtonMapping(InputManager::KEY, newVK[i], 0));
        }
        // Save to config file
        nuonEnv.SaveConfigFile(nullptr);
        show_dialog_async("Input configuration saved to nuance.cfg", "Configuration Saved");
    }
}

static void ResetEmulator()
{
    for (int i = 0; i < 4; i++)
        nuonEnv.mpe[i].Reset();

    std::string msg = "All 4 MPEs have been reset.\\nUse F2 to load a new game.";
    show_dialog_async(msg, "Reset Complete");
}

static void TogglePause()
{
    bRun = !bRun;
    if (bRun)
        fprintf(stderr, "Resumed\n");
    else
        fprintf(stderr, "Paused\n");
    fflush(stderr);
}

static void SingleStep()
{
    bRun = false;
    nuonEnv.mpe[3].ExecuteSingleStep();
    nuonEnv.mpe[2].ExecuteSingleStep();
    nuonEnv.mpe[1].ExecuteSingleStep();
    nuonEnv.mpe[0].ExecuteSingleStep();
    if (nuonEnv.pendingCommRequests)
        DoCommBusController();

    // Show disassembly of what just executed
    char buf[4096];
    int off = 0;
    for (int i = 0; i < 4; i++) {
        if (!GetMPERunStatus(i)) continue;
        off += snprintf(buf + off, sizeof(buf) - off, "=== MPE%d pcexec=$%08X ===\\n", i, nuonEnv.mpe[i].pcexec);
        char pkt[2048];
        nuonEnv.mpe[i].PrintInstructionCachePacket(pkt, sizeof(pkt), nuonEnv.mpe[i].pcexec);
        // Escape single quotes for kdialog
        for (char* p = pkt; *p; p++) if (*p == '\'') *p = '`';
        for (char* p = pkt; *p; p++) if (*p == '\n') { off += snprintf(buf + off, sizeof(buf) - off, "\\n"); } else { buf[off++] = *p; }
        off += snprintf(buf + off, sizeof(buf) - off, "\\n\\n");
    }
    buf[off] = '\0';
    show_dialog_async(buf, "Single Step");
}

static void ShowDisassembly()
{
    static int mpeIdx = 3;
    char buf[4096];
    int off = 0;

    off += snprintf(buf + off, sizeof(buf) - off,
        "=== MPE%d  pcexec=$%08X ===\\n\\n", mpeIdx, nuonEnv.mpe[mpeIdx].pcexec);

    if (GetMPERunStatus(mpeIdx)) {
        char pkt[2048];
        nuonEnv.mpe[mpeIdx].PrintInstructionCachePacket(pkt, sizeof(pkt), nuonEnv.mpe[mpeIdx].pcexec);
        for (char* p = pkt; *p; p++) if (*p == '\'') *p = '`';
        for (char* p = pkt; *p; p++) if (*p == '\n') { off += snprintf(buf + off, sizeof(buf) - off, "\\n"); } else { buf[off++] = *p; }
    } else {
        off += snprintf(buf + off, sizeof(buf) - off, "(MPE%d is halted)", mpeIdx);
    }
    buf[off] = '\0';

    show_dialog_async(buf, "Disassembly");
}

bool NuanceUI_ProcessEvent(void* event)
{
    if (!g_UIInitialized) return false;

    XEvent* xev = (XEvent*)event;
    if (xev->type == KeyPress)
    {
        KeySym key = XLookupKeysym(&xev->xkey, 0);

        switch (key) {
            case XK_F2:
                OpenFileDialog();
                return true;
            case XK_F3:
                ShowStatusDialog();
                return true;
            case XK_F4:
                ShowRegistersDialog();
                return true;
            case XK_F5:
                ShowKprintfLog();
                return true;
            case XK_F6:
                DumpMPEs();
                return true;
            case XK_F7:
                ShowInputConfig();
                return true;
            case XK_F8:
                ResetEmulator();
                return true;
            case XK_F9:
                TogglePause();
                return true;
            case XK_F10:
                SingleStep();
                return true;
            case XK_F11:
                ShowDisassembly();
                return true;
        }
    }
    return false;
}

void NuanceUI_Render() {}
bool NuanceUI_IsVisible() { return false; }
void NuanceUI_SetVisible(bool) {}
void NuanceUI_TogglePause() {}

// Update window title with FPS and status
void NuanceUI_UpdateTitle(int kcs, int fps)
{
    if (!g_Display || !g_Window) return;
    char title[256];
    snprintf(title, sizeof(title),
        "Nuance - %d Kc/s - %dfps | F2:Load F3:Status F4:Regs F7:Input F8:Reset F9:Pause F10:Step F11:Disasm",
        kcs, fps);
    XStoreName(g_Display, g_Window, title);
    XFlush(g_Display);
}

#endif
