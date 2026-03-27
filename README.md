# Nuance 0.6.7 — NUON Emulator

A cross-platform emulator for the [VM Labs NUON](https://en.wikipedia.org/wiki/Nuon_(DVD_technology)) multimedia processor, originally found in select Samsung and Toshiba DVD players (2000–2003). NUON was capable of running games alongside standard DVD playback.

> **Original author:** Mike Perry (2002–2007)
> **Resurrection:** Carsten Waechter / toxie (2020–2025)
> **Linux port:** WizzardSK (2026)

NUON is a trademark of Genesis Microchip, Inc.

## Game Compatibility

| Game | Status |
|------|--------|
| **Ballistic** | Close to perfect |
| **Space Invaders XL** | Playable (rare hangs in Battle/Time Attack) |
| **The Next Tetris** | Playable (distorted music/speech) |
| **Merlin Racing** | Playable (minor gfx glitches, random AI) |
| **Tempest 3000** | Playable with workarounds (see below) |
| **Iron Soldier 3** | Unplayable (requires MPEG) |
| **Freefall 3050 AD** | Unplayable (requires MPEG) |

**Tempest 3000 workarounds:**
1. Enter Options → Music Test and play a different track before starting a game
2. Enter Options → Vector Test before entering level select

## Requirements

- SSE2 CPU support
- 200+ MB free RAM
- OpenGL 1.5+ with GLSL support
- CPU intensive — performance depends heavily on host CPU speed

## Installation

### Windows

Extract all files into a directory and run `Nuance.exe`. Required files in the same directory:
- `bios.cof`, `minibios.cof`, `minibiosX.cof` — BIOS implementations
- `nuance.cfg` — configuration file
- `video_generic.vs`, `video_m32_o32.fs`, `video_m32_o32_crt.fs` — GLSL shaders
- `glew32.dll` — OpenGL extensions library

### Linux

A **32-bit build** is required for the x86 JIT dynamic recompiler (full speed):

```bash
# Prerequisites (Ubuntu/Debian)
sudo apt install gcc-multilib g++-multilib libgl1-mesa-dev:i386 libx11-dev:i386 libpulse0:i386

# Build
mkdir build32 && cd build32
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="-m32" -DCMAKE_CXX_FLAGS="-m32" -DCMAKE_EXE_LINKER_FLAGS="-m32"
make -j$(nproc)
```

A 64-bit build is also possible (uses IL interpreter, slower):
```bash
mkdir build64 && cd build64
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

BIOS and shader files are automatically copied to the build directory by CMake.

### Loading Games

**Windows:** Click "Load File" button, select a `.cof` or NUONROM-DISK file.

**Linux:** Pass the game file as argument. Supports direct loading from ZIP and ISO:
```bash
./nuance /path/to/game.zip        # ZIP containing ISO
./nuance /path/to/game.iso        # DVD ISO image
./nuance /path/to/NUON/nuon.run   # Extracted game
```

ZIP/ISO files are mounted via FUSE (`mount-zip`/`archivemount`) for instant loading. Game data files are read directly from the mounted ISO without extraction.

## Controls

### Default Keyboard Mapping

| Key | NUON Button |
|-----|-------------|
| Arrow keys | D-Pad |
| D | A |
| F | B |
| A | Start |
| S | NUON/Z |
| Q | L Shoulder |
| T | R Shoulder |
| W | C-Pad Left |
| E | C-Pad Down |
| R | C-Pad Right |
| 3 | C-Pad Up |
| Z | Switch controller 0/1 |

Mappings can be customized in `nuance.cfg` under `[Controller1Mappings]`. Format: `BUTTON = KEY_<vkcode>_0`

On Linux, press **F7** for interactive key configuration.

### Linux Shortcuts

| Key | Function |
|-----|----------|
| F1 | Toggle fullscreen |
| F2 | Load game (native file dialog) |
| F3 | Status info (MPE, comm bus, compiler) |
| F4 | Register dump (MPE3) |
| F5 | kprintf log |
| F6 | Dump MPE memory to .bin files |
| F7 | Configure input (interactive) |
| F8 | Reset all MPEs |
| F9 | Pause / Resume |
| F10 | Single step |
| F11 | Disassembly |

Window title bar shows **Kc/s** (kilo-cycles/sec) and **FPS** in real time.

## Configuration

The configuration file `nuance.cfg` supports these options:

| Section | Values | Description |
|---------|--------|-------------|
| `[DVDBase]` | path | Directory prepended to file paths (auto-set on load) |
| `[AudioInterrupts]` | Enabled/Disabled | Audio interrupt support (recommended: Enabled) |
| `[DynamicCompiler]` | Enabled/Disabled | JIT compilation (recommended: Enabled) |
| `[CompilerConstantPropagation]` | Enabled/Disabled | Optimization pass (experimental) |
| `[CompilerDeadCodeElimination]` | Enabled/Disabled | Optimization pass (experimental) |
| `[DumpCompiledBlocks]` | Enabled/Disabled | Debug: dump compiled blocks to files |
| `[T3KCompilerHack]` | Enabled/Disabled | Workaround for Tempest 3000 |
| `[AutomaticLoadPopup]` | Enabled/Disabled | Show file dialog on startup |
| `[UseCRTshader]` | Enabled/Disabled | CRT-style postprocessing shader |
| `[DebugLogFile]` | filename | Log kprintf output to file |

## Technical Details

### Architecture

The NUON processor contains 4 Media Processing Engines (MPEs), each with:
- 32 general-purpose 32-bit registers (organized as 8 vector registers)
- Instruction cache (IRAM) with overlay support
- DMA engines (linear and bilinear)
- Communications bus for inter-MPE messaging

The emulator assumes an **Aries 2** generation chip.

### Audio

- **Windows:** FMOD 3.75 library
- **Linux:** miniaudio (bundled) — auto-detects PulseAudio, ALSA, or other backends
- Supports all DAC configurations via NUON BIOS audio routines
- NISE mixer support (software, not native)
- libSYNTH is not supported

### Video

- 8-bit, 16-bit, and 32-bit pixel formats with optional Z-buffer
- YCrCb to RGB color conversion via GLSL pixel shaders
- Hardware-accelerated rendering via OpenGL
- CRT-style postprocessing shader (optional)

### Dynamic Compiler

The JIT compiler operates in multiple phases:
1. **Block selection** — identifies compilable packet sequences
2. **Constant propagation** — folds known values through the block
3. **Dead code elimination** — removes unused computations
4. **Code generation** — emits native x86 code or IL blocks

On 64-bit Linux, an [asmjit](https://github.com/asmjit/asmjit) integration is in progress for native x86-64 JIT support.

### Supported Features

- DMA: Linear (all modes) and BiLinear (most pixel modes)
- Interrupts: Comm, timer, VDG, software (hardware-specific VDP interrupts not emulated)
- Comm Bus: Full inter-MPE communication
- Memory Manager: MemAlloc/MemFree BIOS calls
- Flash EEPROM: Atmel AT49BV162A/163A emulation
- kprintf debugging via ring buffer and optional log file
- Overlay management with CRC-based caching (up to 32 overlays)
- Breakpoint support via `breakpoint.txt`

### printf Debugging

```c
#include <nuon/bios.h>
extern void kprintf(const char *fmt, ...);

int main(void) {
    kprintf("Hello, world! %d %d %d\n", 1, 2, 3);
    return 0;
}
```

Output appears in the kprintf Log (F5 on Linux, status dialog on Windows).

## Changelog

### 0.6.7
- Add Linux port with CMake build system, X11/GLX backend, and miniaudio audio
- 32-bit build supports x86 JIT via `__attribute__((fastcall))`
- Load games directly from ZIP/ISO files via FUSE mount
- Case-insensitive file matching for DVD data files
- ISO9660 reader for direct ISO access without extraction
- Full debug UI via keyboard shortcuts (F2–F11)
- Interactive input configuration dialog
- asmjit framework for future 64-bit JIT support (WIP)
- libretro core support (WIP, separate branch)

### 0.6.6 (03/21/2025)
- Implement (bi)linear address mirroring
- Optimize memory reads and DMA transfers
- Fix linear DMA edge cases
- Implement BDMA_Type8_Read_0

### 0.6.5 (01/04/2024)
- Fix nuance.cfg loading with drag and drop
- Hide mouse cursor in fullscreen
- Fix user breakpoints and 16/16z bilinear DMA
- Enable kprintf outputs

### 0.6.4 (01/10/2023)
- Fix command line file parsing and drag and drop
- Optimize and fix OpenGL output shaders
- Add CRT-like shader support

### 0.6.3 (01/04/2023)
- Add CRT postprocessing shader
- Add command line file loading with auto-fullscreen
- Add DirectInput joystick support and configuration dialog
- Fix MPE IRAM overlay caching
- Merlin Racing now playable

### 0.6.2 (11/27/2022)
- Audio interrupt improvements
- Buffer overflow protection

### 0.6.1 (09/19/2021)
- Optimizations and opcode fixes
- Improved timer precision

### 0.6.0 (5/28/2020)
- Major fixes for T3K, Ballistic, Merlin Racing
- Sound improvements
- Performance optimizations
- Pixel shader and VSync support
- Fullscreen toggle fix
- Rudimentary x64 support (interpreter only)

<details>
<summary>Earlier versions (0.1.0 – 0.5.0)</summary>

### 0.5.0 (5/30/2007)
- Compiler bug fixes, syscall interface, bootloader support

### 0.4.0 (2/05/2006)
- BIOS fixes, file I/O support, 4-bit DMA, native x86 recompilation

### 0.3.2 (10/16/2004)
- IntSetVector fixes, VDG interrupts, flash EEPROM emulation

### 0.3.1 (7/03/2004)
- Thread synchronization fix, cache optimization

### 0.3.0 (6/30/2004)
- Dynamic recompiler with constant propagation and dead code elimination

### 0.2.2 (4/03/2004)
- BIOS interrupt fix, switch to GLEW

### 0.2.1 (3/24/2004)
- Fix getenv returning NULL

### 0.2.0 (3/22/2004)
- MSB flag fix, FMOD audio, NISE library support, CommBus improvements

### 0.1.0 (11/25/2003)
- Major update: commercial games playable (Space Invaders XL, Tetris, Merlin Racing, T3K)

### 0.0.1B (11/01/2003)
- Bugfix: memory display, instruction handler fixes

### 0.0.1A (10/19/2003)
- Bugfix: scheduler, CMPWC, SUBWC, disassembly display

### 0.0.1 (10/12/2003)
- First release

</details>

## License

See [License.txt](License.txt) for details.
