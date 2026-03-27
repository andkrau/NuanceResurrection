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

# FMOD stub
add_library(fmod_stub STATIC external/fmod_stub.cpp)
target_include_directories(fmod_stub PUBLIC external)
target_compile_definitions(fmod_stub PUBLIC F_API= DLL_API=)

# asmjit (for 64-bit JIT)
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
    if(BUILD_LIBRETRO)
        target_compile_options(asmjit_static PRIVATE -fPIC)
    endif()
endif()

# MurmurHash3
add_library(murmurhash STATIC external/MurmurHash3.cpp)
target_include_directories(murmurhash PUBLIC external)

# Main executable sources
set(NUANCE_SOURCES
    audio.cpp
    bdma_type12.cpp
    bdma_type5.cpp
    bdma_type8.cpp
    bdma_type9.cpp
    bios.cpp
    byteswap.cpp
    coff.cpp
    comm.cpp
    CriticalSection.cpp
    DecodeALU.cpp
    DecodeECU.cpp
    DecodeMEM.cpp
    DecodeMUL.cpp
    DecodeRCU.cpp
    dma.cpp
    EmitALU.cpp
    EmitECU.cpp
    EmitMEM.cpp
    EmitMisc.cpp
    EmitMUL.cpp
    EmitRCU.cpp
    ExecuteALU.cpp
    ExecuteECU.cpp
    ExecuteMEM.cpp
    ExecuteMisc.cpp
    ExecuteMUL.cpp
    ExecuteRCU.cpp
    file.cpp
    FlashEEPROM.cpp
    $<IF:$<PLATFORM_ID:Windows>,GLWindow.cpp,GLWindow_x11.cpp>
    $<IF:$<PLATFORM_ID:Windows>,InputManager.cpp,InputManager_x11.cpp>
    InstructionCache.cpp
    joystick.cpp
    media.cpp
    memory.cpp
    MemoryManager.cpp
    mpe.cpp
    MPEControlRegisters.cpp
    mpe_alloc.cpp
    nativecodecache.cpp
    $<IF:$<PLATFORM_ID:Windows>,NuanceMain.cpp,NuanceMain_linux.cpp>
    NuonEnvironment.cpp
    NuonMemoryManager.cpp
    nuonrom.cpp
    OverlayManager.cpp
    PageMap.cpp
    PatchManager.cpp
    PerformanceTimer.cpp
    PresentationEngine.cpp
    PrintALU.cpp
    PrintECU.cpp
    PrintMEM.cpp
    PrintMisc.cpp
    PrintMUL.cpp
    PrintRCU.cpp
    PropagateConstantsHandlers.cpp
    PropagateConstants_ALU.cpp
    PropagateConstants_ECU.cpp
    PropagateConstants_MEM.cpp
    PropagateConstants_MUL.cpp
    PropagateConstants_RCU.cpp
    ShaderProgram.cpp
    SuperBlock.cpp
    SuperBlockConstants.cpp
    Syscall.cpp
    TestConditionCode.cpp
    timer.cpp
    Utility.cpp
    video.cpp
    $<IF:$<PLATFORM_ID:Windows>,,NuanceUI.cpp>
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:asmjit_emit.cpp>
)

add_executable(nuance ${NUANCE_SOURCES})

target_include_directories(nuance PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/compat
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glew-2.2.0/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/fmod-3.75/api/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/asmjit
    ${SDL2_INCLUDE_DIRS}
)

target_compile_definitions(nuance PRIVATE
    GLEW_STATIC
    GLEW_NO_GLU
    _CRT_SECURE_NO_WARNINGS
    _LARGEFILE64_SOURCE
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:USE_ASMJIT>
)

target_link_libraries(nuance PRIVATE
    glew_static
    fmod_stub
    murmurhash
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:asmjit_static>
    OpenGL::GL
    ${SDL2_LIBRARIES}
    pthread
    dl
    m
)

# For 32-bit static SDL2, provide samplerate stub and GLX
if(EXISTS "/tmp/libsamplerate.a" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(nuance PRIVATE /tmp/libsamplerate.a -lGLX)
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
target_compile_options(nuance PRIVATE
    -msse2
    -Wno-multichar
    -Wno-write-strings
    -Wno-narrowing
    -Wno-unused-result
    -Wno-format-security
    -fpermissive
)
