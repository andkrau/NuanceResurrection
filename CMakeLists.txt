cmake_minimum_required(VERSION 3.16)
project(Nuance CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# Find SDL2 and OpenGL
if(SDL2_INCLUDE_DIRS AND SDL2_LIBRARIES)
    # Manual SDL2 configuration (for static 32-bit builds)
    message(STATUS "Using manually specified SDL2: ${SDL2_LIBRARIES}")
else()
    find_package(SDL2 REQUIRED)
endif()
find_package(OpenGL REQUIRED)

# GLEW from bundled source
add_library(glew_static STATIC external/glew.c)
target_include_directories(glew_static PUBLIC external/glew-2.2.0/include)
target_compile_definitions(glew_static PUBLIC GLEW_STATIC GLEW_NO_GLU)
target_link_libraries(glew_static PUBLIC OpenGL::GL)
set_property(TARGET glew_static PROPERTY POSITION_INDEPENDENT_CODE ON)

## miniaudio (single-TU implementation)
add_library(miniaudio_impl STATIC external/miniaudio_impl.cpp)
target_include_directories(miniaudio_impl PUBLIC external)
set_property(TARGET miniaudio_impl PROPERTY POSITION_INDEPENDENT_CODE ON)

# miniz (single-file ZIP reader) - currently unused
# on Linux we don't currently use it (FUSE tools handle ZIPs)
#add_library(miniz_impl STATIC external/miniz.c)
#target_include_directories(miniz_impl PUBLIC external)
#set_property(TARGET miniz_impl PROPERTY POSITION_INDEPENDENT_CODE ON)

# asmjit (only used by the 64-bit JIT path; 32-bit keeps the legacy x86 emitter)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
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
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:src/asmjit_emit.cpp>
)

option(BUILD_LIBRETRO "Build libretro core instead of standalone" OFF)

if(BUILD_LIBRETRO)
    # Libretro core - shared library
    # Remove standalone-specific files for libretro build
    list(FILTER NUANCE_SOURCES EXCLUDE REGEX "NuanceMain|GLWindow|InputManager|NuanceUI")
    add_library(nuance_libretro SHARED ${NUANCE_SOURCES} src/libretro.cpp)
    set_target_properties(nuance_libretro PROPERTIES PREFIX "" OUTPUT_NAME "nuance_libretro")
    target_compile_definitions(nuance_libretro PRIVATE LIBRETRO)
    set(NUANCE_TARGET nuance_libretro)
else()
    add_executable(nuance ${NUANCE_SOURCES})
    set(NUANCE_TARGET nuance)
endif()

target_include_directories(${NUANCE_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glew-2.2.0/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/asmjit
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
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:USE_ASMJIT>
)

target_link_libraries(${NUANCE_TARGET} PRIVATE
    glew_static
    miniaudio_impl
    #miniz_impl
    #murmurhash # unused - see add_library above
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:asmjit_static>
    OpenGL::GL
    ${SDL2_LIBRARIES}
    pthread
    dl
    m
)

# Standalone build needs X11/GLX directly (NuanceUI uses Xlib, GLWindow uses GLX)
if(NOT BUILD_LIBRETRO AND NOT WIN32)
    find_package(X11 REQUIRED)
    target_link_libraries(${NUANCE_TARGET} PRIVATE ${X11_LIBRARIES} -lGLX)
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
target_compile_options(${NUANCE_TARGET} PRIVATE
    -msse2
    -Wno-multichar
    -Wno-write-strings
    -Wno-narrowing
    -Wno-unused-result
    -Wno-format-security
    -fpermissive
)
