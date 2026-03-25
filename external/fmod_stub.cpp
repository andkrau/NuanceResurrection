// FMOD 3.75 replacement using miniaudio for cross-platform audio output
// On Windows, link against the real FMOD library instead.

#ifdef _WIN32
// Windows uses real FMOD — this file should not be compiled on Windows
#else

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MA_NO_ENGINE
#define MA_NO_NODE_GRAPH
#define MA_NO_RESOURCE_MANAGER
#include "miniaudio.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <atomic>
#include "fmod-3.75/api/inc/fmod.h"

struct FSOUND_STREAM_IMPL {
    FSOUND_STREAMCALLBACK callback;
    void* userdata;
    int sampleRate;
    int bufferLen;
    unsigned int mode;
    std::atomic<bool> muted;
    uint8_t* transferBuf;
    ma_device device;
    bool deviceInited;
};

static FSOUND_STREAM_IMPL* activeStream = nullptr;
static bool fmodInitialized = false;
static int initMixRate = 32000;

static void ma_data_callback(ma_device* pDevice, void* pOutput, const void* /*pInput*/, ma_uint32 frameCount)
{
    FSOUND_STREAM_IMPL* s = (FSOUND_STREAM_IMPL*)pDevice->pUserData;
    if (!s || !s->callback) {
        memset(pOutput, 0, frameCount * 4); // 2ch * 16bit = 4 bytes/frame
        return;
    }

    int bytesNeeded = frameCount * 4;
    if (bytesNeeded > s->bufferLen) bytesNeeded = s->bufferLen;

    signed char ret = s->callback((FSOUND_STREAM*)s, s->transferBuf, bytesNeeded, s->userdata);

    if (ret && !s->muted.load()) {
        memcpy(pOutput, s->transferBuf, bytesNeeded);
    } else {
        memset(pOutput, 0, bytesNeeded);
    }
}

DLL_API signed char F_API FSOUND_SetDriver(int) { return 1; }
DLL_API signed char F_API FSOUND_SetMixer(int) { return 1; }

DLL_API signed char F_API FSOUND_Init(int mixrate, int, unsigned int)
{
    initMixRate = mixrate;
    fmodInitialized = true;
    return 1;
}

DLL_API void F_API FSOUND_Close()
{
    fmodInitialized = false;
}

DLL_API FSOUND_STREAM* F_API FSOUND_Stream_Create(FSOUND_STREAMCALLBACK callback, int length, unsigned int mode, int samplerate, void* userdata)
{
    if (!fmodInitialized) return nullptr;

    FSOUND_STREAM_IMPL* s = new FSOUND_STREAM_IMPL();
    s->callback = callback;
    s->userdata = userdata;
    s->sampleRate = samplerate > 0 ? samplerate : initMixRate;
    s->bufferLen = length;
    s->mode = mode;
    s->muted = false;
    s->deviceInited = false;
    s->transferBuf = new uint8_t[length];
    memset(s->transferBuf, 0, length);

    return (FSOUND_STREAM*)s;
}

DLL_API int F_API FSOUND_Stream_Play(int, FSOUND_STREAM* stream)
{
    if (!stream) return -1;
    FSOUND_STREAM_IMPL* s = (FSOUND_STREAM_IMPL*)stream;

    if (s->deviceInited) return 0;

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.sampleRate = s->sampleRate;
    config.dataCallback = ma_data_callback;
    config.pUserData = s;
    config.periodSizeInFrames = s->bufferLen / 4; // 4 bytes per frame (2ch * 16bit)

    if (ma_device_init(NULL, &config, &s->device) != MA_SUCCESS) {
        fprintf(stderr, "Audio: miniaudio device init failed\n");
        return -1;
    }

    if (ma_device_start(&s->device) != MA_SUCCESS) {
        fprintf(stderr, "Audio: miniaudio device start failed\n");
        ma_device_uninit(&s->device);
        return -1;
    }

    s->deviceInited = true;
    activeStream = s;
    fprintf(stderr, "Audio: started at %d Hz via %s\n", s->sampleRate, s->device.playback.name);

    return 0;
}

DLL_API signed char F_API FSOUND_Stream_Stop(FSOUND_STREAM* stream)
{
    if (!stream) return 0;
    FSOUND_STREAM_IMPL* s = (FSOUND_STREAM_IMPL*)stream;

    if (s->deviceInited) {
        ma_device_uninit(&s->device);
        s->deviceInited = false;
    }

    return 1;
}

DLL_API signed char F_API FSOUND_Stream_Close(FSOUND_STREAM* stream)
{
    if (!stream) return 0;
    FSOUND_STREAM_IMPL* s = (FSOUND_STREAM_IMPL*)stream;

    FSOUND_Stream_Stop(stream);
    delete[] s->transferBuf;
    if (activeStream == s) activeStream = nullptr;
    delete s;

    return 1;
}

DLL_API signed char F_API FSOUND_IsPlaying(int) { return activeStream && activeStream->deviceInited ? 1 : 0; }

DLL_API signed char F_API FSOUND_SetMute(int, signed char mute)
{
    if (activeStream) activeStream->muted = mute;
    return 1;
}

DLL_API signed char F_API FSOUND_SetFrequency(int, int) { return 1; }
DLL_API signed char F_API FSOUND_SetVolume(int, int) { return 1; }

#endif // !_WIN32
