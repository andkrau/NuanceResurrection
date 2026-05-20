// NuanceResurrection libretro core. The emulator loop, globals and
// load/init helpers live in EmulatorCore (shared with the standalone
// Linux frontend in NuanceMain_linux.cpp); this file only handles the
// libretro API surface, input mapping and game-file discovery.

#include "libretro.h"
#include "basetypes.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <mutex>
#include <cstdarg>

#include "byteswap.h"
#include "Utility.h"
#include "comm.h"
#include "audio.h"
#include "mpe.h"
#include "EmulatorCore.h"
#include "NuonEnvironment.h"
#include "NuonMemoryMap.h"
#include "joystick.h"
#include "video.h"
#include "ExecuteMEM.h"
#include "timer.h"
#include "Bios.h"
#include "iso9660.h"

// GLWindow stubs for libretro (no windowing needed)
#include "GLWindow.h"
GLWindow::GLWindow() : bFullScreen(false), bVisible(false), keyDownHandler(nullptr), keyUpHandler(nullptr),
    paintHandler(nullptr), resizeHandler(nullptr), applyControllerState(nullptr), inputManager(nullptr),
    clientWidth(720), clientHeight(480), x(0), y(0), width(720), height(480),
    fullScreenWidth(1920), fullScreenHeight(1080), hInstance(nullptr), hWnd(nullptr), hDC(nullptr), hRC(nullptr),
    windowStyle(0), windowExtendedStyle(0), fullScreenWindowStyle(0), fullScreenWindowExtendedStyle(0),
    threadHandle(0), threadID(0), restoreWidth(720), restoreHeight(480), restoreX(0), restoreY(0) {}
GLWindow::~GLWindow() {}
void GLWindow::UpdateRestoreValues() {}
void GLWindow::ToggleFullscreen() {}
bool GLWindow::ChangeScreenResolution(int,int) { return false; }
bool GLWindow::CreateWindowGL() { return false; }
void GLWindow::CleanUp() {}
bool GLWindow::RegisterWindowClass() { return false; }
bool GLWindow::Create() { return false; }
void GLWindow::MessagePump() {}
void GLWindow::OnResize(int,int) {}
unsigned GLWindow::GLWindowMain(void*) { return 0; }
GLWindow display;

extern ControllerData *controller;
extern VidChannel structMainChannel, structOverlayChannel;
extern bool bOverlayChannelActive, bMainChannelActive;
extern vidTexInfo videoTexInfo;

// Libretro callbacks
static retro_environment_t environ_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_hw_render_callback hw_render;

static bool gl_initialized = false;
static bool game_loaded = false;

// Audio buffer
#define AUDIO_BUFFER_SIZE 4096
static int16_t audio_buffer[AUDIO_BUFFER_SIZE];

// Software framebuffer fallback
#define FB_WIDTH 720
#define FB_HEIGHT 480
static uint32_t framebuffer[FB_WIDTH * FB_HEIGHT];

static void log_printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

// InputManager stubs for libretro (input handled by retro_input_state)
#include "InputManager.h"
InputManager::~InputManager() {}
InputManager* InputManager::Create() { return nullptr; }
bool InputManager::StrToInputType(const char* str, InputType* type) {
    if (!strcasecmp("KEY", str)) { *type = KEY; return true; }
    if (!strcasecmp("JOYBUT", str)) { *type = JOYBUT; return true; }
    if (!strcasecmp("JOYAXIS", str)) { *type = JOYAXIS; return true; }
    if (!strcasecmp("JOYPOV", str)) { *type = JOYPOV; return true; }
    return false;
}
const char* InputManager::InputTypeToStr(InputType type) {
    switch (type) { case KEY: return "KEY"; case JOYBUT: return "JOYBUT"; case JOYAXIS: return "JOYAXIS"; case JOYPOV: return "JOYPOV"; default: return ""; }
}

static uint16 GetInputButtons()
{
    uint16 buttons = 0;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))    buttons |= CTRLR_DPAD_UP;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))  buttons |= CTRLR_DPAD_DOWN;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))  buttons |= CTRLR_DPAD_LEFT;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT)) buttons |= CTRLR_DPAD_RIGHT;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A))     buttons |= CTRLR_BUTTON_A;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B))     buttons |= CTRLR_BUTTON_B;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START)) buttons |= CTRLR_BUTTON_START;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT))buttons |= CTRLR_BUTTON_NUON;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L))     buttons |= CTRLR_BUTTON_L;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R))     buttons |= CTRLR_BUTTON_R;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X))     buttons |= CTRLR_BUTTON_C_UP;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y))     buttons |= CTRLR_BUTTON_C_LEFT;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2))    buttons |= CTRLR_BUTTON_C_DOWN;
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2))    buttons |= CTRLR_BUTTON_C_RIGHT;
    return buttons;
}

static void context_reset(void)
{
    glewExperimental = GL_TRUE;
    GLenum glew_err = glewInit();
    // GLEW often reports GLEW_ERROR_NO_GL_VERSION on core/forward-compatible contexts
    // because glGetString(GL_VERSION) is unavailable; the function pointers still load.
    if (glew_err != GLEW_OK && glew_err != 4 /* GLEW_ERROR_NO_GL_VERSION */) {
        log_printf("libretro: glewInit failed: %s\n", glewGetErrorString(glew_err));
        gl_initialized = false;
        return;
    }
    log_printf("libretro: GL context reset OK (GL %s)\n", (const char*)glGetString(GL_VERSION));

    gl_initialized = true;

    // (CPU extensions, ALU lookup tables and nuonEnv.Init now happen
    // unconditionally in retro_load_game via EmulatorCore::Init.)

    // Setup GL state
    glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glClearColor(0, 0, 0, 1);
}

static void context_destroy(void)
{
    gl_initialized = false;
}

// --- Libretro API ---

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;
    bool no_game = false;
    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_game);

    static struct retro_input_descriptor desc[] = {
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "NUON" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L Shoulder" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R Shoulder" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "C-Up" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "C-Left" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2, "C-Down" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2, "C-Right" },
        { 0, 0, 0, 0, NULL },
    };
    cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t) {}
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

unsigned retro_api_version(void) { return RETRO_API_VERSION; }

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "Nuance";
    info->library_version = "0.6.7";
    info->valid_extensions = "run|cof|nuon|cd|iso|img";
    info->need_fullpath = true;
    info->block_extract = false; // let RetroArch extract ZIPs for us
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    info->geometry.base_width = FB_WIDTH;
    info->geometry.base_height = FB_HEIGHT;
    info->geometry.max_width = FB_WIDTH;
    info->geometry.max_height = FB_HEIGHT;
    info->geometry.aspect_ratio = 4.0f / 3.0f;
    info->timing.fps = VIDEO_HZ;
    info->timing.sample_rate = 32000.0;
}

void retro_init(void)
{
    log_printf("libretro: retro_init start\n");
    fflush(stderr);
    // Defer heavy init to retro_load_game
    log_printf("libretro: retro_init done\n");
    fflush(stderr);
}

void retro_deinit(void)
{
    log_printf("libretro: retro_deinit\n");
}

void retro_set_controller_port_device(unsigned, unsigned) {}

void retro_reset(void)
{
    for (int i = 0; i < 4; i++)
        nuonEnv.mpe[i].Reset();
}

static std::string popen_line(const std::string& cmd)
{
    FILE* fp = popen(cmd.c_str(), "r");
    if (!fp) return "";
    char buf[2048] = {};
    if (fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    }
    pclose(fp);
    return buf;
}

// Extract game files using 7z (no FUSE dependency - works in any context)
static std::string FindGameFile(const char* path)
{
    std::string spath(path);
    size_t len = spath.size();

    // Direct .run or .cof file
    if ((len > 4 && strcasecmp(path + len - 4, ".run") == 0) ||
        (len > 4 && strcasecmp(path + len - 4, ".cof") == 0) ||
        (len > 3 && strcasecmp(path + len - 3, ".cd") == 0) ||
        (len > 5 && strcasecmp(path + len - 5, ".nuon") == 0))
        return spath;

    bool isZip = (len > 4 && strcasecmp(path + len - 4, ".zip") == 0);
    bool isIso = (len > 4 && (strcasecmp(path + len - 4, ".iso") == 0 || strcasecmp(path + len - 4, ".img") == 0));
    if (!isZip && !isIso) return spath;

    // Create extraction dir next to the ISO/ZIP file (e.g. nuon-roms/Game.extract/)
    std::string baseName = spath.substr(spath.rfind('/') + 1);
    size_t dotPos = baseName.rfind('.');
    if (dotPos != std::string::npos) baseName = baseName.substr(0, dotPos);
    std::string parentDir = spath.substr(0, spath.rfind('/'));
    std::string td = parentDir + "/" + baseName + ".extract";
    mkdir(td.c_str(), 0755);
    log_printf("libretro: temp dir: %s\n", td.c_str()); fflush(stderr);

    std::string isoFile;
    if (isIso) {
        isoFile = spath;
    } else {
        // Extract ISO from ZIP (unzip is faster than 7z for simple ZIP)
        std::string cmd = "unzip -o -d \"" + td + "\" \"" + spath + "\" '*.iso' '*.img' > /dev/null 2>&1";
        log_printf("libretro: extracting ISO from ZIP...\n"); fflush(stderr);
        system(cmd.c_str());
        isoFile = popen_line("find \"" + td + "\" -maxdepth 2 \\( -iname '*.iso' -o -iname '*.img' \\) -print -quit 2>/dev/null");
    }
    if (isoFile.empty()) { log_printf("libretro: no ISO found\n"); return ""; }
    log_printf("libretro: ISO: %s\n", isoFile.c_str()); fflush(stderr);

    // Extract NUON directory from ISO (skip if already extracted)
    std::string gameDir = td + "/game";
    std::string result = popen_line("find \"" + gameDir + "\" -maxdepth 3 \\( -iname 'nuon.run' -o -iname 'NUON.CD' -o -iname 'cd_app.cof' \\) -print -quit 2>/dev/null");
    if (result.empty()) {
      std::string cmd = "mkdir -p \"" + gameDir + "\" && 7z x -y -o\"" + gameDir + "\" \"" + isoFile + "\" NUON/ nuon/ > /dev/null 2>&1";
      log_printf("libretro: extracting NUON dir from ISO...\n"); fflush(stderr);
      system(cmd.c_str());
      result = popen_line("find \"" + gameDir + "\" -maxdepth 3 \\( -iname 'nuon.run' -o -iname 'NUON.CD' -o -iname 'cd_app.cof' \\) -print -quit 2>/dev/null");
    } else {
      log_printf("libretro: using cached extraction\n"); fflush(stderr);
    }
    if (result.empty()) { log_printf("libretro: no NUON boot file found\n"); return ""; }

    log_printf("libretro: boot file: %s\n", result.c_str()); fflush(stderr);
    return result;
}

bool retro_load_game(const struct retro_game_info *game)
{
    if (!game || !game->path) return false;

    log_printf("libretro: loading %s\n", game->path); fflush(stderr);

    // Defer CPU init to context_reset to avoid potential issues

    // Request OpenGL context
    hw_render.context_type = RETRO_HW_CONTEXT_OPENGL;
    hw_render.context_reset = context_reset;
    hw_render.context_destroy = context_destroy;
    hw_render.depth = false;
    hw_render.stencil = false;
    hw_render.bottom_left_origin = true;
    hw_render.version_major = 2;
    hw_render.version_minor = 1;
    hw_render.cache_context = true;

    log_printf("libretro: requesting HW render...\n"); fflush(stderr);
    if (!environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render)) {
        log_printf("libretro: HW render not available, using software\n"); fflush(stderr);
    } else {
        log_printf("libretro: HW render OK\n"); fflush(stderr);
    }

    // Find game file
    log_printf("libretro: FindGameFile(%s)...\n", game->path); fflush(stderr);
    std::string gamePath = FindGameFile(game->path);
    log_printf("libretro: FindGameFile returned: '%s'\n", gamePath.c_str()); fflush(stderr);
    if (gamePath.empty()) {
        log_printf("libretro: cannot find NUON game in %s\n", game->path);
        return false;
    }

    log_printf("libretro: game file found: %s\n", gamePath.c_str()); fflush(stderr);

    // Initialize emulator
    // Find bios files - check system directory BEFORE Init (bios.cof is loaded during Init)
    const char* sysDir = nullptr;
    environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &sysDir);
    if (sysDir) {
        std::string biosPath = std::string(sysDir) + "/nuance/bios.cof";
        if (access(biosPath.c_str(), F_OK) == 0) {
            chdir((std::string(sysDir) + "/nuance").c_str());
            log_printf("libretro: bios dir: %s/nuance\n", sysDir); fflush(stderr);
        }
    }
    // Also check build directory (for development)
    if (access("bios.cof", F_OK) != 0) {
        // Try the directory containing the .so
        Dl_info dl_info;
        if (dladdr((void*)retro_init, &dl_info) && dl_info.dli_fname) {
            std::string soDir(dl_info.dli_fname);
            size_t pos = soDir.rfind('/');
            if (pos != std::string::npos) {
                soDir = soDir.substr(0, pos);
                if (access((soDir + "/bios.cof").c_str(), F_OK) == 0) {
                    chdir(soDir.c_str());
                    log_printf("libretro: bios dir (from .so): %s\n", soDir.c_str()); fflush(stderr);
                }
            }
        }
    }

    log_printf("libretro: EmulatorCore::Init()...\n"); fflush(stderr);
    EmulatorCore::Init();
    log_printf("libretro: EmulatorCore::Init() done\n"); fflush(stderr);

    // Load game
    if (!EmulatorCore::LoadGame(gamePath.c_str())) {
        log_printf("libretro: failed to load %s\n", gamePath.c_str());
        return false;
    }
    for(int i=0;i<4;i++) log_printf("libretro: mpe%d inten1=%08X inten2sel=%u intsrc=%08X intctl=%08X mpectl=%08X\n", i, nuonEnv.mpe[i].inten1, nuonEnv.mpe[i].inten2sel, nuonEnv.mpe[i].intsrc, nuonEnv.mpe[i].intctl, nuonEnv.mpe[i].mpectl); fflush(stderr);
    // Process any pending comm requests from BIOS init before starting emulation
    while (nuonEnv.pendingCommRequests) {
        log_printf("libretro: processing %d pending comm requests from init\n", nuonEnv.pendingCommRequests); fflush(stderr);
        DoCommBusController();
    }
    bRun = true;
    game_loaded = true;

    log_printf("libretro: game loaded successfully\n");
    return true;
}

bool retro_load_game_special(unsigned, const struct retro_game_info*, size_t) { return false; }

void retro_unload_game(void)
{
    bRun = false;
    game_loaded = false;
    EmulatorCore::Shutdown();
}

void retro_run(void)
{
    if (!game_loaded) return;

    // Input
    input_poll_cb();
    EmulatorCore::ApplyController(1, GetInputButtons());

    // Run shared emulator loop until the next video field, with a wall-time
    // budget so we return control to retroarch on time for 60Hz pacing.
    // During BIOS init (before video is configured) give the loop longer
    // so init can finish. push_audio=false: libretro consumes via
    // DrainAudioRing below.
    nuonEnv.trigger_render_video = false;
    static int init_frames = 0;
    const uint64 frame_budget_us = (init_frames < 100) ? 50000 : 16000;
    init_frames++;
    EmulatorCore::RunUntilVideoFrame(frame_budget_us, /*push_audio=*/false);

    // Render video
    if (gl_initialized) {
        glBindFramebuffer(GL_FRAMEBUFFER, hw_render.get_current_framebuffer());
        glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
        RenderVideo(FB_WIDTH, FB_HEIGHT);
        video_cb(RETRO_HW_FRAME_BUFFER_VALID, FB_WIDTH, FB_HEIGHT, 0);
    } else {
        // Software fallback - black frame
        memset(framebuffer, 0, sizeof(framebuffer));
        video_cb(framebuffer, FB_WIDTH, FB_HEIGHT, FB_WIDTH * 4);
    }

    // Audio: drain whatever the emulator has pushed into NuonEnvironment's
    // ring buffer since the previous retro_run. Replaces the old direct
    // double-buffer read + audio_buffer_played flag that disappeared when
    // the audio path was generalized to miniaudio.
    const uint32 framesOut = nuonEnv.DrainAudioRing(audio_buffer, AUDIO_BUFFER_SIZE / 2);
    if (framesOut > 0)
        audio_batch_cb(audio_buffer, framesOut);
}

size_t retro_serialize_size(void) { return 0; }
bool retro_serialize(void*, size_t) { return false; }
bool retro_unserialize(const void*, size_t) { return false; }
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned, bool, const char*) {}
unsigned retro_get_region(void) { return RETRO_REGION_NTSC; }
void *retro_get_memory_data(unsigned) { return nullptr; }
size_t retro_get_memory_size(unsigned) { return 0; }

// Swap buffers stub for video.cpp
void SDL2_SwapWindow() {}
