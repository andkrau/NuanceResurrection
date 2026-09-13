cmake_minimum_required(VERSION 3.16)
project(Nuance CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

# Find SDL2 and OpenGL. SDL2 is a standalone-only dependency (windowing +
# audio); the libretro core uses neither, so don't require it for that build
# (the libretro Windows/MinGW CI has no SDL2 in its sysroot).
if(SDL2_INCLUDE_DIRS AND SDL2_LIBRARIES)
    # Manual SDL2 configuration (for static 32-bit builds)
    message(STATUS "Using manually specified SDL2: ${SDL2_LIBRARIES}")
elseif(BUILD_LIBRETRO)
    find_package(SDL2 QUIET)
else()
    find_package(SDL2 REQUIRED)
endif()
# Desktop OpenGL and GLEW, the loader for it, exist on Linux, Windows and macOS
# and nowhere else: Android and iOS have OpenGL ES, which needs no loader and
# has none of the fixed-function entry points this renderer uses. Asking for
# them unconditionally is a configure-time failure on those platforms, before
# any of that becomes the real question.
if(IOS OR CMAKE_SYSTEM_NAME MATCHES "^(iOS|tvOS|watchOS)$")
    set(NUANCE_APPLE_MOBILE ON)
    # A framework is one linker argument in two words, which target_link_libraries
    # splits into two items and then mangles into `-framework -lOpenGLES`. Asking
    # find_library for it yields the path to link, which has no such problem.
    find_library(NUANCE_OPENGLES_FRAMEWORK OpenGLES REQUIRED)
else()
    set(NUANCE_APPLE_MOBILE OFF)
endif()

if(ANDROID OR NUANCE_APPLE_MOBILE)
    set(NUANCE_GLES ON)
else()
    set(NUANCE_GLES OFF)
endif()
message(STATUS "NUANCE_GLES (OpenGL ES target) = ${NUANCE_GLES} (system: ${CMAKE_SYSTEM_NAME})")

if(NOT NUANCE_GLES)
    find_package(OpenGL REQUIRED)

    # GLEW from bundled source
    add_library(glew_static STATIC external/glew.c)
    target_include_directories(glew_static PUBLIC external/glew-2.3.1/include)
    target_compile_definitions(glew_static PUBLIC GLEW_STATIC GLEW_NO_GLU)
    target_link_libraries(glew_static PUBLIC OpenGL::GL)
    set_property(TARGET glew_static PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

## miniaudio (single-TU implementation)
add_library(miniaudio_impl STATIC external/miniaudio_impl.cpp)
target_include_directories(miniaudio_impl PUBLIC external)
set_property(TARGET miniaudio_impl PROPERTY POSITION_INDEPENDENT_CODE ON)

# libchdr (vendored CHD reader for archive.cpp). Also gives us a single
# miniz copy that both libchdr's zlib codec and archive.cpp's Windows ZIP
# path share — turning on miniz's archive APIs replaces the older inline
# external/miniz copy we used to ship.
set(MINIZ_ARCHIVE_APIS ON CACHE BOOL "" FORCE)
set(MINIZ_STDIO        ON CACHE BOOL "" FORCE)
set(WITH_LZMA_ASM      OFF CACHE BOOL "" FORCE)  # avoid MASM/arm64 asm path
set(BUILD_SHARED_LIBS  OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/libchdr EXCLUDE_FROM_ALL)
set_property(TARGET chdr-static  PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET miniz        PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET chdr-lzma    PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET zstd         PROPERTY POSITION_INDEPENDENT_CODE ON)

# The asmjit JIT path emits x86 machine code and the -mssse3 flag are x86-only.
# On other architectures (e.g. aarch64) build interpreter-only without them.
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
    set(NUANCE_X86_64 TRUE)
else()
    set(NUANCE_X86_64 FALSE)
endif()
message(STATUS "NUANCE_X86_64 (asmjit JIT enabled) = ${NUANCE_X86_64} (processor: ${CMAKE_SYSTEM_PROCESSOR})")

# The SSE/SSSE3 intrinsics in the emitters and the byteswap helpers are compiled
# on every x86 target, not only the 64-bit one — on aarch64 they go through the
# vendored sse2neon. x86_64 has SSE2 in its baseline so it only ever needed the
# SSSE3 switch, but a 32-bit x86 build has neither on by default and fails in
# emmintrin.h ("inlining failed in call to always_inline _mm_set_epi32").
# Asking the compiler whether it takes the flag covers both without having to
# recognise every spelling of the processor name; it is simply absent elsewhere.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-mssse3 NUANCE_HAS_MSSSE3)

# asmjit (only used by the 64-bit JIT path; 32-bit keeps the legacy x86 emitter)
if(NUANCE_X86_64)
    file(GLOB_RECURSE ASMJIT_SRC
        external/asmjit/asmjit/core/*.cpp
        external/asmjit/asmjit/x86/*.cpp
        external/asmjit/asmjit/support/*.cpp
        external/asmjit/asmjit/ujit/*.cpp
    )
    add_library(asmjit_static STATIC ${ASMJIT_SRC})
    target_include_directories(asmjit_static PUBLIC external/asmjit)
    target_compile_definitions(asmjit_static PUBLIC ASMJIT_STATIC ASMJIT_NO_AARCH64)
    set_property(TARGET asmjit_static PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# MurmurHash3 - currently unused (the codebase uses xxHash instead). Kept on disk
# in case OverlayManager ever switches back; uncomment to re-enable
#add_library(murmurhash STATIC external/MurmurHash3.cpp)
#target_include_directories(murmurhash PUBLIC external)
#set_property(TARGET murmurhash PROPERTY POSITION_INDEPENDENT_CODE ON)

# Dear ImGui (vendored, Linux standalone uses it for the live status overlay)
if(NOT WIN32 AND NOT BUILD_LIBRETRO)
    add_library(imgui_static STATIC
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui_static PUBLIC
        external/imgui
        external/imgui/backends
    )
    set_property(TARGET imgui_static PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# Main executable sources
set(NUANCE_SOURCES
    src/archive.cpp
    src/audio.cpp
    src/bdma_type12.cpp
    src/bdma_type5.cpp
    src/bdma_type8.cpp
    src/bdma_type9.cpp
    src/bios.cpp
    src/byteswap.cpp
    src/coff.cpp
    src/comm.cpp
    src/CriticalSection.cpp
    src/DecodeALU.cpp
    src/DecodeECU.cpp
    src/DecodeMEM.cpp
    src/DecodeMUL.cpp
    src/DecodeRCU.cpp
    src/dma.cpp
    src/EmitALU.cpp
    src/EmitECU.cpp
    src/EmitMEM.cpp
    src/EmitMisc.cpp
    src/EmitMUL.cpp
    src/EmitRCU.cpp
    src/ExecuteALU.cpp
    src/ExecuteECU.cpp
    src/ExecuteMEM.cpp
    src/ExecuteMisc.cpp
    src/ExecuteMUL.cpp
    src/ExecuteRCU.cpp
    src/file.cpp
    src/FlashEEPROM.cpp
    $<IF:$<PLATFORM_ID:Windows>,src/GLWindow.cpp,src/GLWindow_x11.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:src/imgui_impl_x11.cpp>
    $<IF:$<PLATFORM_ID:Windows>,src/InputManager.cpp,src/InputManager_x11.cpp>
    src/InstructionCache.cpp
    src/joystick.cpp
    src/media.cpp
    src/memory.cpp
    src/MemoryManager.cpp
    src/mpe.cpp
    src/MPEControlRegisters.cpp
    src/mpe_alloc.cpp
    src/nativecodecache.cpp
    $<IF:$<PLATFORM_ID:Windows>,src/NuanceMain.cpp,src/NuanceMain_linux.cpp>
    src/NuonEnvironment.cpp
    src/NuonMemoryManager.cpp
    src/nuonrom.cpp
    src/OverlayManager.cpp
    src/PageMap.cpp
    src/PatchManager.cpp
    src/PerformanceTimer.cpp
    src/PresentationEngine.cpp
    src/PrintALU.cpp
    src/PrintECU.cpp
    src/PrintMEM.cpp
    src/PrintMisc.cpp
    src/PrintMUL.cpp
    src/PrintRCU.cpp
    src/PropagateConstantsHandlers.cpp
    src/PropagateConstants_ALU.cpp
    src/PropagateConstants_ECU.cpp
    src/PropagateConstants_MEM.cpp
    src/PropagateConstants_MUL.cpp
    src/PropagateConstants_RCU.cpp
    src/ShaderProgram.cpp
    src/SuperBlock.cpp
    src/SuperBlockConstants.cpp
    src/Syscall.cpp
    src/TestConditionCode.cpp
    src/timer.cpp
    src/Utility.cpp
    src/video.cpp
    $<IF:$<PLATFORM_ID:Windows>,,src/NuanceUI.cpp>
    $<$<BOOL:${NUANCE_X86_64}>:src/asmjit_emit.cpp>
)

option(BUILD_LIBRETRO "Build libretro core instead of standalone" OFF)

if(BUILD_LIBRETRO)
    # Libretro core - shared library
    # Remove standalone-specific files for libretro build. archive.cpp is kept:
    # the core uses its ResolveGameFile() to resolve ISO/ZIP/CHD inputs and to
    # read data files straight from the ISO at runtime (no whole-disc extraction),
    # so libchdr + miniz are linked in below for the libretro build too.
    # imgui_impl_x11 belongs with the other standalone-only files: the libretro
    # build has no imgui (imgui_static is standalone-only) and pulling it in
    # dragged X11 headers into a build that has none on macOS.
    list(FILTER NUANCE_SOURCES EXCLUDE REGEX "NuanceMain|GLWindow|InputManager|NuanceUI|imgui_impl")
    add_library(nuance_libretro SHARED ${NUANCE_SOURCES} src/libretro.cpp)
    set_target_properties(nuance_libretro PROPERTIES PREFIX "" OUTPUT_NAME "nuance_libretro")
    target_compile_definitions(nuance_libretro PRIVATE LIBRETRO)
    set(NUANCE_TARGET nuance_libretro)
else()
 if(NOT WIN32)
    add_executable(nuance ${NUANCE_SOURCES})
 else()
    add_executable(nuance WIN32 ${NUANCE_SOURCES})
 endif()
    set(NUANCE_TARGET nuance)
endif()

# The GLSL shaders are embedded into the binary via src/embedded_shaders.h,
# which #includes the real .vs/.fs files as C++ raw strings (they carry their
# own #ifdef EMBED_HLSL guards, so the files stay valid standalone GLSL too).
# No generated header: the .vs/.fs files are the single source of truth.
# ShaderProgram still loads external shader files first and only falls back to
# the embedded copy when none are found next to the binary. The repo root is on
# the include path below so the #include "video_*.vs/.fs" lines resolve.

target_include_directories(${NUANCE_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glew-2.3.1/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/asmjit
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libchdr/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libchdr/deps/miniz-3.1.1
    ${SDL2_INCLUDE_DIRS}
)

# compat/ contains Linux shims for Win32 headers (windows.h, dinput.h, etc.).
# On Windows, NTFS is case-insensitive, so #include <windows.h> would resolve
# to the lowercase compat/windows.h shim before the real MinGW SDK header and
# break the build (see issue #48).
if(NOT WIN32)
    target_include_directories(${NUANCE_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/compat
    )
endif()

target_compile_definitions(${NUANCE_TARGET} PRIVATE
    GLEW_STATIC
    GLEW_NO_GLU
    _CRT_SECURE_NO_WARNINGS
    _LARGEFILE64_SOURCE
    $<$<BOOL:${NUANCE_X86_64}>:USE_ASMJIT>
    # Tells the sources they are building against OpenGL ES: no loader, no
    # fixed-function matrix stack, and a GLES2 hardware context to ask for.
    $<$<BOOL:${NUANCE_GLES}>:NUANCE_GLES>
    $<$<BOOL:${NUANCE_APPLE_MOBILE}>:NUANCE_NO_SUBPROCESS>
    # Apple deprecated the whole of OpenGL ES in iOS 12 and offers Metal
    # instead. Until someone writes that renderer, every GL call this core
    # makes is a warning - ninety-nine of them, which is enough to hide a real
    # one. The deprecation is noted; silence it rather than reading past it.
    $<$<BOOL:${NUANCE_APPLE_MOBILE}>:GLES_SILENCE_DEPRECATION>
)

# NuanceMain.cpp on Windows provides its own WinMain entry point.
# SDL2main.lib also defines WinMain, so linking both produces a duplicate
# WinMain. Tell SDL2 not to remap our main() and strip SDL2main from the
# link line (the user-supplied SDL2_LIBRARIES on a typical Windows SDL2
# install includes it).
if(WIN32 AND NOT BUILD_LIBRETRO)
    target_compile_definitions(${NUANCE_TARGET} PRIVATE SDL_MAIN_HANDLED)
    list(FILTER SDL2_LIBRARIES EXCLUDE REGEX "(SDL2main|sdl2main)")
endif()

target_link_libraries(${NUANCE_TARGET} PRIVATE
    $<$<NOT:$<BOOL:${NUANCE_GLES}>>:glew_static>
    # Standalone only, like SDL2: the libretro core hands its audio to
    # RetroArch and never opens a device. On iOS miniaudio also reaches for the
    # Objective-C audio backends, which a C++ build cannot compile.
    $<$<NOT:$<BOOL:${BUILD_LIBRETRO}>>:miniaudio_impl>
    # libchdr (CHD reader) + its bundled miniz 3.1.1 (also doubles as the ZIP
    # reader for archive.cpp's Windows #ifdef path - configured with
    # MINIZ_ARCHIVE_APIS=ON above so mz_zip_reader_* is available). Needed by
    # archive.cpp's ResolveGameFile() in both the standalone and libretro builds.
    chdr-static
    miniz
    #murmurhash # unused - see add_library above
    $<$<BOOL:${NUANCE_X86_64}>:asmjit_static>
    # Desktop GL through the loader, or GLES2 linked straight in - on Android
    # and iOS the library is part of the platform and there is nothing to load.
    $<$<NOT:$<BOOL:${NUANCE_GLES}>>:OpenGL::GL>
    # GLESv3 for the sized texture formats the video buffers use; the library
    # also carries the ES 2 entry points, so there is nothing else to link.
    # Apple has no such library - the same entry points live in a framework.
    $<$<AND:$<BOOL:${NUANCE_GLES}>,$<NOT:$<BOOL:${NUANCE_APPLE_MOBILE}>>>:GLESv3>
    $<$<BOOL:${NUANCE_APPLE_MOBILE}>:${NUANCE_OPENGLES_FRAMEWORK}>
    # SDL2 is a standalone-only dependency (windowing + audio); the libretro core
    # uses neither. Guarding it out also avoids a CMP0004 hard error when the
    # libretro build infra passes SDL2_LIBRARIES as a raw `sdl2-config --libs`
    # string (e.g. "-L/usr/lib/... -lSDL2 ") that has trailing whitespace.
    $<$<NOT:$<BOOL:${BUILD_LIBRETRO}>>:${SDL2_LIBRARIES}>
    # Bionic has the pthread functions in libc itself and ships no libpthread
    # to link, so asking for one is an error rather than a no-op there.
    $<$<AND:$<NOT:$<PLATFORM_ID:Windows>>,$<NOT:$<BOOL:${ANDROID}>>>:pthread>
    $<$<NOT:$<PLATFORM_ID:Windows>>:dl>
    $<$<NOT:$<PLATFORM_ID:Windows>>:m>
)

# Standalone build needs X11/GLX directly (NuanceUI uses Xlib, GLWindow uses GLX)
# and Dear ImGui for the live status overlay panel.
if(NOT BUILD_LIBRETRO AND NOT WIN32)
    find_package(X11 REQUIRED)
    target_link_libraries(${NUANCE_TARGET} PRIVATE ${X11_LIBRARIES} -lGLX imgui_static)
endif()

# timer.cpp uses the Windows multimedia timer API (timeBeginPeriod /
# timeEndPeriod / timeGetDevCaps), which lives in winmm — needed by both the
# standalone exe and the libretro core.
if(WIN32)
    target_link_libraries(${NUANCE_TARGET} PRIVATE winmm)
endif()

# Windows standalone additionally needs DirectInput8 (InputManager.cpp) and the
# DirectX GUIDs (IID_IDirectInput8A / c_dfDIJoystick2); the libretro core has
# no DirectInput path.
if(WIN32 AND NOT BUILD_LIBRETRO)
    target_link_libraries(${NUANCE_TARGET} PRIVATE dinput8 dxguid)
endif()

# On a MinGW build, statically link the GCC/libstdc++/winpthread runtimes so the
# core (or exe) is self-contained and doesn't drag libgcc_s_seh-1.dll /
# libstdc++-6.dll / libwinpthread-1.dll alongside it.
if(WIN32 AND MINGW)
    target_link_options(${NUANCE_TARGET} PRIVATE
        -static-libgcc -static-libstdc++ -static -lwinpthread)
endif()

# For 32-bit static SDL2, provide samplerate stub
if(EXISTS "/tmp/libsamplerate.a" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(${NUANCE_TARGET} PRIVATE /tmp/libsamplerate.a)
endif()

# Copy required data files to build directory
set(NUANCE_DATA_FILES
    bios.cof minibios.cof minibiosX.cof nuance.cfg
    video_generic.vs video_m32_o32.fs video_m32_o32_crt.fs
)
foreach(f ${NUANCE_DATA_FILES})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${f})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${f} ${CMAKE_CURRENT_BINARY_DIR}/${f} COPYONLY)
    endif()
endforeach()

# Compiler flags
if (MSVC)
    target_compile_options(${NUANCE_TARGET} PRIVATE
        /arch:SSE2
        /permissive-
        /EHsc     # standard C++ exceptions

        # optimization
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/Ob2>
        $<$<CONFIG:Release>:/Oy>
        $<$<CONFIG:Release>:/GL>
        $<$<CONFIG:Release>:/GF>
        $<$<CONFIG:Release>:/GS->
        $<$<CONFIG:Release>:/Gy->
        $<$<CONFIG:Release>:/fp:fast>
        $<$<CONFIG:Release>:/fp:except->

        # debug
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/DEBUG>
        $<$<CONFIG:Debug>:/Zi>
        $<$<CONFIG:Debug>:/GS->
        $<$<CONFIG:Debug>:/fp:fast>
    )
    target_link_options(${NUANCE_TARGET} PRIVATE
        $<$<CONFIG:Release>:/LTCG>
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
    )
else()
    target_compile_options(${NUANCE_TARGET} PRIVATE
        -O3
        -ffast-math
        $<$<BOOL:${NUANCE_HAS_MSSSE3}>:-mssse3>
        -Wno-multichar
        -Wno-write-strings
        -Wno-narrowing
        -Wno-unused-result
        -Wno-format-security
        -fpermissive
    )
endif()

get_target_property(MY_COMPILE_OPTIONS ${NUANCE_TARGET} COMPILE_OPTIONS)
message(STATUS "Compile options: ${MY_COMPILE_OPTIONS}")
