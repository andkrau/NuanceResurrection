// UI for NuanceResurrection (Linux)
// Dear ImGui overlay for live MPE status; kdialog/zenity for file picker and
// one-shot popups (kprintf log, disassembly, etc).
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
#include "joystick.h"
#include "video.h"
#include "NuanceUI.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui_impl_x11.h"

extern NuonEnvironment nuonEnv;
extern bool bQuit;
extern bool Load(const char* file);
extern bool bRun;

#include "comm.h"

static bool g_UIInitialized = false;
static bool g_ImGuiReady = false;
static bool g_ShowStatusPanel = false;
static bool g_ShowInputPanel = false;
static bool g_ShowSettingsPanel = false;
static bool g_ShowConsolePanel  = false;
static bool g_ConsoleAutoScroll = true;
static int  g_RegsPanelMpe = -1;       // -1 = closed, 0..3 = which MPE to show
static int  g_RebindingBitnum = -1;    // -1 = not capturing, otherwise CTRLR_BITNUM_* to bind
static Display* g_Display = nullptr;
static Window g_Window = 0;

// Short-lived in-window toast notification. Used by Reset / Step / Dump /
// Pause / Resume / etc instead of shelling out to kdialog popups.
static std::string g_ToastMsg;
static double      g_ToastUntil = -1.0;
static void ShowToast(const char* msg, double seconds = 3.0)
{
    g_ToastMsg = msg ? msg : "";
    g_ToastUntil = ImGui::GetTime() + seconds;
}

// Multi-line modal dialog for Disassembly / kprintf snapshot. The current
// text and a "want to show" flag survive between frames.
static std::string g_ModalTitle;
static std::string g_ModalBody;
static bool        g_ModalOpenRequest = false;
static void ShowModal(const char* title, const std::string& body)
{
    g_ModalTitle = title ? title : "Info";
    g_ModalBody  = body;
    g_ModalOpenRequest = true;
}

void NuanceUI_Init(void* display, unsigned long window)
{
    g_Display = (Display*)display;
    g_Window = (Window)window;
    g_UIInitialized = true;

    // ImGui sets up against the *current* GL context; the caller owns making
    // GLWindow_x11's GLX context current before this point (done at Create()).
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // don't pollute cwd with imgui.ini
    ImGui::StyleColorsDark();

    if (ImGui_ImplX11_Init(g_Display, g_Window) && ImGui_ImplOpenGL3_Init("#version 130")) {
        g_ImGuiReady = true;
        fprintf(stderr, "[NuanceUI] ImGui initialized (OpenGL3 + X11 backends)\n");
    } else {
        fprintf(stderr, "[NuanceUI] ImGui init FAILED\n");
    }
    // No panels open by default — menu bar is enough discoverability;
    // user opens what they need via the View menu or F-key accelerators.
}

void NuanceUI_Shutdown()
{
    if (g_ImGuiReady) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplX11_Shutdown();
        ImGui::DestroyContext();
        g_ImGuiReady = false;
    }
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

// Forward declarations - menu bar dispatches to handlers defined further down
static void ShowKprintfLog();
static void DumpMPEs();
static void ResetEmulator();
static void TogglePause();
static void SingleStep();
static void ShowDisassembly();

static void DrawMenuBar()
{
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Load Game...", "F2")) OpenFileDialog();
        ImGui::Separator();
        if (ImGui::MenuItem("Quit"))               bQuit = true;
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Emulation")) {
        if (ImGui::MenuItem(bRun ? "Pause" : "Resume", "F9")) TogglePause();
        if (ImGui::MenuItem("Single step",                "F10", false, !bRun)) SingleStep();
        ImGui::Separator();
        if (ImGui::MenuItem("Reset",                      "F8"))  ResetEmulator();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("MPE Status",     "F3",  &g_ShowStatusPanel);
        bool regsOpen = (g_RegsPanelMpe >= 0);
        if (ImGui::MenuItem("MPE Registers", "F4", regsOpen))
            g_RegsPanelMpe = regsOpen ? -1 : 3;
        ImGui::MenuItem("Input Mapping",  "F7",  &g_ShowInputPanel);
        ImGui::MenuItem("Settings",       "F12", &g_ShowSettingsPanel);
        ImGui::MenuItem("Console (kprintf)", "F5", &g_ShowConsolePanel);
        ImGui::Separator();
        if (ImGui::MenuItem("Disassembly",  "F11")) ShowDisassembly();
        if (ImGui::MenuItem("Dump MPE memory", "F6")) DumpMPEs();
        ImGui::EndMenu();
    }

    // OpenPopup has to be called outside BeginMenu/EndMenu so the popup is
    // anchored in the main viewport rather than as a child of the menu (which
    // would close instantly when the menu collapses).
    static bool s_openAbout = false;
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("About Nuance...")) s_openAbout = true;
        ImGui::EndMenu();
    }

    // FPS / Kc/s on the right side of the bar.
    char rhs[64];
    snprintf(rhs, sizeof(rhs), "%.1f FPS  ", ImGui::GetIO().Framerate);
    const float rhsW = ImGui::CalcTextSize(rhs).x;
    ImGui::SameLine(ImGui::GetWindowWidth() - rhsW - 8.0f);
    ImGui::TextDisabled("%s", rhs);

    ImGui::EndMainMenuBar();

    if (s_openAbout) { ImGui::OpenPopup("About Nuance"); s_openAbout = false; }

    // Center the About modal on the viewport.
    const ImVec2 viewportCenter = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(viewportCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("About Nuance", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::TextUnformatted("Nuance 0.6.7");
        ImGui::TextDisabled("NUON (VM Labs) emulator");
        ImGui::Separator();

        ImGui::TextWrapped("Original code: Mike Perry, 2002-2007.");
        ImGui::TextWrapped("Continued: Carsten Waechter (toxie at ainc.de), 2020-.");
        ImGui::TextWrapped("Linux / libretro port + ImGui UI: WizzardSK, 2026.");

        ImGui::Separator();
        ImGui::Text("Build:    %s %s", __DATE__, __TIME__);
        ImGui::Text("Arch:     %u-bit", (unsigned)(sizeof(void*) * 8));
#ifdef USE_ASMJIT
        ImGui::Text("JIT:      asmjit (compiled in, %s at runtime)",
                    nuonEnv.compilerOptions.bAllowCompile ? "enabled" : "disabled");
#else
        ImGui::Text("JIT:      interpreter only (USE_ASMJIT undefined)");
#endif
        ImGui::Separator();
        ImGui::TextDisabled("NUON is a trademark of Genesis Microchip, Inc.");
        ImGui::TextDisabled("See license.txt for the full license terms.");
        ImGui::Separator();

        ImGui::Text("Project: ");
        ImGui::SameLine();
        ImGui::TextDisabled("github.com/andkrau/NuanceResurrection");

        ImGui::Separator();
        // Center the OK button.
        const float btnW = 80.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - btnW) * 0.5f);
        if (ImGui::Button("OK", ImVec2(btnW, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

static void DrawStatusPanel()
{
    if (!g_ShowStatusPanel) return;
    ImGui::SetNextWindowPos(ImVec2(10, 35), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("MPE Status", &g_ShowStatusPanel,
                      ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("mpe_status", 4,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("MPE");
        ImGui::TableSetupColumn("State");
        ImGui::TableSetupColumn("pcexec");
        ImGui::TableSetupColumn("Regs");
        ImGui::TableHeadersRow();
        for (int i = 0; i < 4; i++) {
            const bool run = GetMPERunStatus(i);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("MPE%d", i);
            ImGui::TableNextColumn();
            ImGui::TextColored(run ? ImVec4(0.2f, 0.9f, 0.3f, 1.0f)
                                   : ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                               "%s", run ? "RUN" : "---");
            ImGui::TableNextColumn(); ImGui::Text("$%08X", nuonEnv.mpe[i].pcexec);
            ImGui::TableNextColumn();
            ImGui::PushID(i);
            if (ImGui::SmallButton("Open"))
                g_RegsPanelMpe = i;
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::Text("Comm pending: %u", nuonEnv.pendingCommRequests);

    if (ImGui::CollapsingHeader("JIT cache")) {
        if (ImGui::BeginTable("jit_stats", 4, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("MPE");
            ImGui::TableSetupColumn("Flushes");
            ImGui::TableSetupColumn("NonCompilable");
            ImGui::TableSetupColumn("Overlays");
            ImGui::TableHeadersRow();
            for (int i = 0; i < 4; i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("MPE%d", i);
                ImGui::TableNextColumn(); ImGui::Text("%u", nuonEnv.mpe[i].numNativeCodeCacheFlushes);
                ImGui::TableNextColumn(); ImGui::Text("%u", nuonEnv.mpe[i].numNonCompilablePackets);
                ImGui::TableNextColumn(); ImGui::Text("%u", nuonEnv.mpe[i].overlayManager.GetOverlaysInUse());
            }
            ImGui::EndTable();
        }
    }

    ImGui::TextDisabled("F3 to toggle this panel");
    ImGui::End();
}

static void DrawRegistersPanel()
{
    if (g_RegsPanelMpe < 0 || g_RegsPanelMpe > 3) return;
    char title[32];
    snprintf(title, sizeof(title), "MPE%d Registers", g_RegsPanelMpe);

    bool open = true;
    ImGui::SetNextWindowPos(ImVec2(10, 305), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title, &open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        if (!open) g_RegsPanelMpe = -1;
        return;
    }

    const MPE& mpe = nuonEnv.mpe[g_RegsPanelMpe];
    ImGui::Text("pcexec $%08X   cc $%08X   sp $%08X",
                mpe.pcexec, mpe.cc, mpe.sp);
    ImGui::Text("rc0 $%08X   rc1 $%08X", mpe.rc0, mpe.rc1);
    ImGui::Text("rx/ry/ru/rv  $%08X $%08X $%08X $%08X",
                mpe.rx, mpe.ry, mpe.ru, mpe.rv);
    ImGui::Text("rz/rzi1/rzi2 $%08X $%08X $%08X",
                mpe.rz, mpe.rzi1, mpe.rzi2);
    ImGui::Text("intctl $%08X  intsrc $%08X  inten1 $%08X  inten2sel $%08X",
                mpe.intctl, mpe.intsrc, mpe.inten1, mpe.inten2sel);

    ImGui::Separator();
    if (ImGui::BeginTable("vregs", 4, ImGuiTableFlags_Borders)) {
        for (int v = 0; v < 8; v++) {
            ImGui::TableNextRow();
            for (int e = 0; e < 4; e++) {
                ImGui::TableNextColumn();
                ImGui::Text("v%d[%d] %08X", v, e, mpe.regs[v * 4 + e]);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
    if (!open) g_RegsPanelMpe = -1;
}

// Human-readable name for a VK_ code (matches XKeyToVK in this file and the
// X11 -> VK_ mapping in GLWindow_x11.cpp).
static const char* VKDisplayName(int vk, char* buf, size_t bufsz)
{
    switch (vk) {
        case 0x0D: return "Enter";
        case 0x1B: return "Escape";
        case 0x20: return "Space";
        case 0x25: return "Left";
        case 0x26: return "Up";
        case 0x27: return "Right";
        case 0x28: return "Down";
        case 0x70: return "F1";  case 0x71: return "F2";
        case 0x72: return "F3";  case 0x73: return "F4";
        case 0x74: return "F5";  case 0x75: return "F6";
        case 0x76: return "F7";  case 0x77: return "F8";
        case 0x78: return "F9";  case 0x79: return "F10";
        case 0x7A: return "F11"; case 0x7B: return "F12";
    }
    if (vk >= 'A' && vk <= 'Z') { snprintf(buf, bufsz, "%c", (char)vk); return buf; }
    if (vk >= '0' && vk <= '9') { snprintf(buf, bufsz, "%c", (char)vk); return buf; }
    snprintf(buf, bufsz, "VK 0x%02X", vk & 0xFF);
    return buf;
}

// Display order mirrors the existing ShowInputConfig (DPad-first, then face / shoulder
// / Start+NUON / C-pad). Skips unused bitnums 6 and 7.
static const int kInputDisplayOrder[] = {
    CTRLR_BITNUM_DPAD_UP, CTRLR_BITNUM_DPAD_DOWN, CTRLR_BITNUM_DPAD_LEFT, CTRLR_BITNUM_DPAD_RIGHT,
    CTRLR_BITNUM_BUTTON_A, CTRLR_BITNUM_BUTTON_B,
    CTRLR_BITNUM_BUTTON_NUON, CTRLR_BITNUM_BUTTON_START,
    CTRLR_BITNUM_BUTTON_L, CTRLR_BITNUM_BUTTON_R,
    CTRLR_BITNUM_BUTTON_C_UP, CTRLR_BITNUM_BUTTON_C_DOWN,
    CTRLR_BITNUM_BUTTON_C_LEFT, CTRLR_BITNUM_BUTTON_C_RIGHT,
};
static const char* const kInputBitnumName[] = {
    /*0*/"C-Right", /*1*/"C-Up",    /*2*/"C-Left", /*3*/"B",
    /*4*/"R",       /*5*/"L",       /*6*/"-",      /*7*/"-",
    /*8*/"D-Right", /*9*/"D-Up",    /*10*/"D-Left",/*11*/"D-Down",
    /*12*/"NUON",   /*13*/"Start",  /*14*/"A",     /*15*/"C-Down"
};

static void DrawInputPanel()
{
    if (!g_ShowInputPanel) return;
    // Configuration windows open centered on the viewport (first time only;
    // user can drag them anywhere afterwards and the new position sticks).
    const ImVec2 vc = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(vc, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    if (!ImGui::Begin("Input Mapping", &g_ShowInputPanel,
                      ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    ImGui::TextDisabled("Keyboard only on Linux. Click \"Bind\" then press the new key.");
    ImGui::Separator();

    if (ImGui::BeginTable("input_mapping", 3,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Button");
        ImGui::TableSetupColumn("Current key");
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();

        for (int bitnum : kInputDisplayOrder) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextUnformatted(kInputBitnumName[bitnum]);

            ImGui::TableNextColumn();
            const ControllerButtonMapping& m = nuonEnv.GetMappingForCTRLRBitnum(bitnum);
            char buf[32];
            if (m.type == InputManager::KEY) {
                ImGui::TextUnformatted(VKDisplayName(m.idx, buf, sizeof(buf)));
            } else {
                ImGui::TextDisabled("(non-key: %s)", InputManager::InputTypeToStr(m.type));
            }

            ImGui::TableNextColumn();
            ImGui::PushID(bitnum);
            if (g_RebindingBitnum == bitnum) {
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "Press any key...");
            } else if (ImGui::SmallButton("Bind")) {
                g_RebindingBitnum = bitnum;
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::Separator();
    static bool s_inSaveOk = false;
    static double s_inSaveAt = -1.0;
    if (ImGui::Button("Save to nuance.cfg")) {
        s_inSaveOk = nuonEnv.SaveConfigFile("nuance.cfg");
        s_inSaveAt = ImGui::GetTime();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset to defaults")) {
        // The default mapping table lives in nuonEnv.controllerDefaultMapping[].
        // Restore each bit-num from there.
        for (int i = 0; i < 16; i++) {
            const ControllerButtonMapping& def = nuonEnv.GetDefaultMappingForCTRLRBitnum(i);
            nuonEnv.SetControllerButtonMapping(i, def);
        }
    }
    if (s_inSaveAt > 0.0 && (ImGui::GetTime() - s_inSaveAt) < 3.0) {
        ImGui::SameLine();
        if (s_inSaveOk)
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Saved.");
        else
            ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Save FAILED");
    }
    if (g_RebindingBitnum >= 0) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f),
                           "Press a key to bind %s (Esc to cancel)",
                           kInputBitnumName[g_RebindingBitnum]);
    }

    ImGui::TextDisabled("F7 to toggle this panel");
    ImGui::End();
}

static void DrawConsolePanel()
{
    if (!g_ShowConsolePanel) return;
    ImGui::SetNextWindowPos(ImVec2(10, 250), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(700, 200), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Console (kprintf)", &g_ShowConsolePanel)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear")) {
        if (nuonEnv.kprintRingBuffer) {
            for (size_t i = 0; i < NuonEnvironment::KPRINT_RING_SIZE; i++)
                nuonEnv.kprintRingBuffer[i][0] = 0;
            nuonEnv.kprintCurrentLine = 0;
            nuonEnv.kprintCurrentChar = 0;
        }
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &g_ConsoleAutoScroll);
    ImGui::SameLine();
    ImGui::TextDisabled("(ring buffer, last %u lines)",
                        (unsigned)NuonEnvironment::KPRINT_RING_SIZE);
    ImGui::Separator();

    // BeginChild/EndChild must be paired regardless of return value, so the
    // EndChild lives outside the if.
    ImGui::BeginChild("##console_scroll", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    if (nuonEnv.kprintRingBuffer) {
        size_t idx = nuonEnv.kprintCurrentLine;
        // Walk the ring in chronological order: oldest = (current+1) % SIZE.
        for (size_t n = 0; n < NuonEnvironment::KPRINT_RING_SIZE; n++) {
            idx = (idx + 1) % NuonEnvironment::KPRINT_RING_SIZE;
            const char* line = nuonEnv.kprintRingBuffer[idx];
            if (line && line[0])
                ImGui::TextUnformatted(line);
        }
    }

    // Auto-scroll if the NUON side wrote new content since the last frame.
    if (g_ConsoleAutoScroll && nuonEnv.kprintUpdated) {
        ImGui::SetScrollHereY(1.0f);
        nuonEnv.kprintUpdated = false;
    }

    ImGui::EndChild();

    ImGui::End();
}

static void DrawSettingsPanel()
{
    if (!g_ShowSettingsPanel) return;
    const ImVec2 vc = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(vc, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    if (!ImGui::Begin("Settings (nuance.cfg)", &g_ShowSettingsPanel,
                      ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    ImGui::TextDisabled("Changes apply live. Click Save to persist to nuance.cfg.");
    ImGui::Separator();

    // Audio
    bool audioInt = nuonEnv.GetAudioInterruptsEnabled();
    if (ImGui::Checkbox("AudioInterrupts", &audioInt))
        nuonEnv.SetAudioInterruptsEnabled(audioInt);
    ImGui::SameLine();
    ImGui::TextDisabled("(disable may speed up but break some games)");

    ImGui::Separator();
    ImGui::TextUnformatted("Compiler");

    ImGui::Checkbox("DynamicCompiler (JIT)", &nuonEnv.compilerOptions.bAllowCompile);
    ImGui::Checkbox("CompilerConstantPropagation", &nuonEnv.compilerOptions.bConstantPropagation);
    ImGui::Checkbox("CompilerDeadCodeElimination", &nuonEnv.compilerOptions.bDeadCodeElimination);
    ImGui::Checkbox("DumpCompiledBlocks", &nuonEnv.compilerOptions.bDumpBlocks);
    ImGui::SameLine();
    ImGui::TextDisabled("(verbose; for debugging)");
    ImGui::Checkbox("MPE3PacketHack", &nuonEnv.compilerOptions.bMPE3PacketHack);
    ImGui::SameLine();
    ImGui::TextDisabled("(most likely only relevant for Tempest 3000)");

    ImGui::Separator();
    ImGui::TextUnformatted("UI / Frontend");
    ImGui::Checkbox("AutomaticLoadPopup at startup", &nuonEnv.bAutomaticLoadPopup);
    ImGui::Checkbox("UseCRTshader",                  &nuonEnv.bUseCRTshader);

    ImGui::Separator();
    static bool s_saveOk = false;
    static double s_saveAt = -1.0;
    if (ImGui::Button("Save to nuance.cfg")) {
        s_saveOk = nuonEnv.SaveConfigFile("nuance.cfg");
        s_saveAt = ImGui::GetTime();
    }
    if (s_saveAt > 0.0 && (ImGui::GetTime() - s_saveAt) < 3.0) {
        ImGui::SameLine();
        if (s_saveOk)
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Saved.");
        else
            ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Save FAILED");
    }

    ImGui::TextDisabled("F12 to toggle this panel");
    ImGui::End();
}

static void ShowStatusDialog()
{
    // Toggle the ImGui overlay (preferred); fall back to kdialog popup if
    // ImGui isn't ready (e.g. early startup before the GL context exists).
    if (g_ImGuiReady) {
        g_ShowStatusPanel = !g_ShowStatusPanel;
        return;
    }

    char buf[1024];
    snprintf(buf, sizeof(buf),
        "MPE0: $%08X %s\\nMPE1: $%08X %s\\nMPE2: $%08X %s\\nMPE3: $%08X %s",
        GetMPERunStatus(0) ? nuonEnv.mpe[0].pcexec : 0, GetMPERunStatus(0) ? "RUN" : "---",
        GetMPERunStatus(1) ? nuonEnv.mpe[1].pcexec : 0, GetMPERunStatus(1) ? "RUN" : "---",
        GetMPERunStatus(2) ? nuonEnv.mpe[2].pcexec : 0, GetMPERunStatus(2) ? "RUN" : "---",
        GetMPERunStatus(3) ? nuonEnv.mpe[3].pcexec : 0, GetMPERunStatus(3) ? "RUN" : "---");
    show_dialog_async(buf, "Nuance Status");
}

static void ShowRegistersDialog()
{
    // Show MPE3 registers (main processor) - could add selection later
    static int mpeIdx = 3;

    // ImGui overlay when available; fall back to kdialog otherwise.
    if (g_ImGuiReady) {
        g_RegsPanelMpe = (g_RegsPanelMpe == mpeIdx) ? -1 : mpeIdx;
        return;
    }

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
    if (g_ImGuiReady) ShowModal("kprintf log", log);
    else              show_dialog_async(log, "kprintf Log");
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
    if (g_ImGuiReady) ShowToast("MPE memory dumped to mpe0.bin - mpe3.bin");
    else              show_dialog_async("MPE memory dumped to mpe0.bin - mpe3.bin", "Dump Complete");
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
        if (g_ImGuiReady) ShowToast("Input configuration saved to nuance.cfg");
        else              show_dialog_async("Input configuration saved to nuance.cfg",
                                            "Configuration Saved");
    }
}

static void ResetEmulator()
{
    extern std::string g_LastLoadedPath;
    for (int i = 0; i < 4; i++)
        nuonEnv.mpe[i].Reset();

    // Re-Load the currently running game so emulation actually restarts;
    // a bare MPE reset leaves us with no NUON code loaded, which looks like
    // a hang. If nothing was loaded, just toast the reset state.
    if (!g_LastLoadedPath.empty()) {
        const std::string p = g_LastLoadedPath;
        if (Load(p.c_str())) {
            if (g_ImGuiReady) ShowToast("Game reset");
            else              show_dialog_async("Game reset.", "Reset Complete");
        } else {
            if (g_ImGuiReady) ShowToast("Reset OK but re-Load failed");
            else              show_dialog_async("Reset OK but re-Load failed", "Reset");
        }
    } else {
        if (g_ImGuiReady) ShowToast("MPEs reset. Use F2 to load a game.");
        else              show_dialog_async("All 4 MPEs have been reset.\\nUse F2 to load a game.",
                                            "Reset Complete");
    }
}

static void TogglePause()
{
    bRun = !bRun;
    fprintf(stderr, "%s\n", bRun ? "Resumed" : "Paused");
    fflush(stderr);
    if (g_ImGuiReady) ShowToast(bRun ? "Resumed" : "Paused");
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

    // Format disassembly of what just executed
    std::string body;
    char pkt[2048];
    for (int i = 0; i < 4; i++) {
        if (!GetMPERunStatus(i)) continue;
        char hdr[64];
        snprintf(hdr, sizeof(hdr), "=== MPE%d pcexec=$%08X ===\n", i, nuonEnv.mpe[i].pcexec);
        body += hdr;
        nuonEnv.mpe[i].PrintInstructionCachePacket(pkt, sizeof(pkt), nuonEnv.mpe[i].pcexec);
        body += pkt;
        body += "\n\n";
    }
    if (body.empty()) body = "(all MPEs halted)";

    if (g_ImGuiReady) ShowModal("Single Step", body);
    else {
        // For kdialog: escape quotes + convert newlines to literal \n
        for (auto& c : body) if (c == '\'') c = '`';
        std::string esc;
        for (char c : body) { if (c == '\n') esc += "\\n"; else esc += c; }
        show_dialog_async(esc, "Single Step");
    }
}

static void ShowDisassembly()
{
    static int mpeIdx = 3;

    std::string body;
    char hdr[64];
    snprintf(hdr, sizeof(hdr), "=== MPE%d  pcexec=$%08X ===\n\n", mpeIdx, nuonEnv.mpe[mpeIdx].pcexec);
    body += hdr;
    if (GetMPERunStatus(mpeIdx)) {
        char pkt[2048];
        nuonEnv.mpe[mpeIdx].PrintInstructionCachePacket(pkt, sizeof(pkt), nuonEnv.mpe[mpeIdx].pcexec);
        body += pkt;
    } else {
        body += "(MPE";
        body += (char)('0' + mpeIdx);
        body += " is halted)";
    }

    if (g_ImGuiReady) ShowModal("Disassembly", body);
    else {
        for (auto& c : body) if (c == '\'') c = '`';
        std::string esc;
        for (char c : body) { if (c == '\n') esc += "\\n"; else esc += c; }
        show_dialog_async(esc, "Disassembly");
    }
}

bool NuanceUI_ProcessEvent(void* event)
{
    if (!g_UIInitialized) return false;

    XEvent* xev = (XEvent*)event;

    // Feed ImGui first so its panels can capture mouse/text/keys while open.
    // The X11 backend returns true when ImGuiIO wants this event (e.g. mouse
    // hovering over a panel, or typing into an InputText).
    bool imguiCaptured = false;
    if (g_ImGuiReady)
        imguiCaptured = ImGui_ImplX11_ProcessEvent(xev);

    if (xev->type == KeyPress)
    {
        KeySym key = XLookupKeysym(&xev->xkey, 0);

        // Input-mapping rebind capture: next key press becomes the new binding
        // for the bitnum the user clicked "Bind" on. Escape cancels.
        if (g_RebindingBitnum >= 0) {
            if (key == XK_Escape) {
                g_RebindingBitnum = -1;
            } else {
                int vk = XKeyToVK(key);
                if (vk > 0) {
                    nuonEnv.SetControllerButtonMapping(
                        g_RebindingBitnum, ControllerButtonMapping(InputManager::KEY, vk, 0));
                    g_RebindingBitnum = -1;
                }
            }
            return true; // consume the key — don't pass to the game
        }

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
                // ImGui console panel (preferred); fall back to kdialog
                // snapshot popup if ImGui isn't initialized yet.
                if (g_ImGuiReady) g_ShowConsolePanel = !g_ShowConsolePanel;
                else              ShowKprintfLog();
                return true;
            case XK_F6:
                DumpMPEs();
                return true;
            case XK_F7:
                // ImGui input panel (preferred); fall back to the legacy
                // raw-Xlib config window if ImGui isn't initialized yet.
                if (g_ImGuiReady) {
                    g_ShowInputPanel = !g_ShowInputPanel;
                    fprintf(stderr, "[NuanceUI] Input panel %s\n",
                            g_ShowInputPanel ? "ON" : "OFF");
                } else {
                    ShowInputConfig();
                }
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
            case XK_F12:
                if (g_ImGuiReady) g_ShowSettingsPanel = !g_ShowSettingsPanel;
                return true;
        }
    }
    return imguiCaptured;
}

void NuanceUI_Render()
{
    if (!g_ImGuiReady) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplX11_NewFrame();
    ImGui::NewFrame();

    DrawMenuBar();
    DrawStatusPanel();
    DrawRegistersPanel();
    DrawInputPanel();
    DrawSettingsPanel();
    DrawConsolePanel();

    // Toast notification (short transient message above the main menu bar's
    // bottom edge — overlays the game scene at the bottom-center of the
    // viewport for `g_ToastUntil` seconds).
    if (ImGui::GetTime() < g_ToastUntil && !g_ToastMsg.empty()) {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f,
                                       io.DisplaySize.y - 20.0f),
                                ImGuiCond_Always, ImVec2(0.5f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.85f);
        if (ImGui::Begin("##toast", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoInputs)) {
            ImGui::TextUnformatted(g_ToastMsg.c_str());
        }
        ImGui::End();
    }

    // Generic modal dialog (Disassembly / kprintf snapshot / etc.). Long
    // multi-line text inside a scrollable child + a Close button. Centered.
    if (g_ModalOpenRequest) {
        ImGui::OpenPopup(g_ModalTitle.c_str());
        g_ModalOpenRequest = false;
    }
    const ImVec2 vc = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(vc, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700, 360), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal(g_ModalTitle.c_str(), nullptr,
                               ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::BeginChild("##modal_body", ImVec2(0, -28), false,
                          ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(g_ModalBody.c_str());
        ImGui::EndChild();
        const float btnW = 80.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - btnW) * 0.5f);
        if (ImGui::Button("Close", ImVec2(btnW, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool NuanceUI_IsVisible() { return g_ShowStatusPanel || g_RegsPanelMpe >= 0; }
void NuanceUI_SetVisible(bool v) { g_ShowStatusPanel = v; }
void NuanceUI_TogglePause() {}

// Update window title with FPS and status
void NuanceUI_UpdateTitle(int kcs, int fps)
{
    if (!g_Display || !g_Window) return;
    char title[256];
    snprintf(title, sizeof(title), "Nuance - %d Kc/s - %dfps", kcs, fps);
    XStoreName(g_Display, g_Window, title);
    XFlush(g_Display);
}

#endif
