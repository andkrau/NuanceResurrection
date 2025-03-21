Nuance 0.6.7
Copyright 2002 - 2007 Mike Perry and 2020 - 2025 all the open source contributors (see separate license.txt)
Continued using the released source in honour of the original author by Carsten Waechter (toxie at ainc.de) in 2020
NUON is a trademark of Genesis Microchip, Inc.

Disclaimer
==========
This emulator was developed in a clean room setting, using only the Nuon SDK
architecture documentation, the llama assembler, vmdisasm disassembler, and
lots of hand crafted automation tools to help generate assembly files and
parse the output.  I cannot guarantee that this program will not blow up your
computer.  It is certainly capable of causing memory access violations.  If 
you lose unsaved data while playing with it, don't say that I didn't warn you.

Requirements
============
This emulator requires SSE2 instruction support, at least 200 Megabytes 
of free RAM, and an OpenGL 1.5 implementation with support for GLSL.

The emulator is completely and utterly CPU bound.  Machines 
that do not meet minimum requirements may even suffer reduced performance when 
running the less demanding SDK demos, or may hang on certain game situations
(in the latter case try disabling AudioInterrupts in the 'nuance.cfg', at
the cost of missing or screwed up sound).

Installation
============
Extract the distribution files into a new directory, click on Nuance.exe and you're
good to go.  The bios.cof, minibios.cof and minibiosX.cof files contain Aries assembly
implementations of some BIOS routines and a complete miniBIOS.  These files must be
kept in the same directory as the main executable.  Nuance also makes use of the
Glew OpenGL extensions library.  The Glew32 DLL is included with the Nuance
distribution but may not always be the most recent version of the DLL.  For this
reason, the Glew32 DLL should be kept in the same directory as Nuance unless your
system already has a more recent version of the DLL somewhere in the standard DLL
search path, in which case the Nuance provided Glew32 DLL may be safely deleted.

The emulator uses configuration files for initializing options.  The configuration
file to use can be specified by passing in the file name as a parameter to the 
emulator executable.  If no parameter is specified, the default filename 'nuance.cfg'
is assumed.

Each configuration file must be updated to select an appropriate base path to be
used for loading files from DVD.  The entry should point to the directory
containing the nuon.run and nuon.dat files.  The DVDBase config entry is prepended
on to the file name parameters passed to MediaOpen and FileOpen.  The path names
should have a trailing backslash.  If you wish to run of the commercial games off
of an original DVD, set the DVDReadBase entry to "<DriveLetter>:\Nuon\" where
<DriveLetter> is the drive letter of the DVD drive.  Dynamic
recompilation may be configured similarly using an entry labeled "DynamicCompiler."
Additional settings for the dynamic compiler are discussed
in the Dynamic Compiler section of this document.

Instructions
============
The emulator will display a control panel window where you can load a file, start 
and stop individual processors and start and stop the emulation loop.

The LED indicators at the top left of the control panel show which processors are
currently running.  You can double click the LEDs to toggle the running state
of each processor.  This should only be done while the main emulator loop is
stopped.

Use the Load File button to load a COFF executable or NUONROM-DISK file into
memory.  If the load is successful the emulation loop will automatically be
started as if the "Run" button were pressed.  Prior to starting the emulation
loop, the pcexec register of MPE3 will be set to the entry point of the
executable and the mpeGo bit of the mpectl register will be set.

Use either the "Run" or "Single Step" buttons to execute a sequence of code. 
If the Single Step button is used, the emulator will attempt to execute a
single instruction on each of the four MPEs.  If an MPE does not have its mpeGo
bit set, no instruction will be executed on that MPE.  If the Run button is
used, the emulator will execute instructions on all of the MPEs until it
detects that all MPEs are halted.  Use the stop button to stop the emulator loop.
When the "Stop" button is used, the running state of each processor is not
modified, only the flag that enables the emulation loop to execute another
iteration.  Note that when the dynamic recompiler is enabled, the processors
may execute blocks of code consisting of multiple packets.  This will behave
as if each processor has a code-dependent system clock.  If synchronous operation
is required during debugging, use the single step mode.

Use the "Single Step" button to execute a single instruction on each processor
that is currently running.  The single step mode will work even if the dynamic
recompiler is enabled.  Prior to executing each instruction, the code cache of
compiled blocks is invalidated so that only a single packet will be interpreted.
This may cause a brief reduction in performance when the "Run" mode is re-entered
due to the fact that some blocks will need to be recompiled.

When using single step mode and when the emulator loop is stopped, the display
window in the center of the screen will be updated with a disassembly of the
next packet to be executed.  To change the MPE that is used for disassembly,
single click one of the MPE labels next to the MPE LED indicators.  Note that
the disassembly may not represent the original ordering of instructions within
the packet and that the disassembly shows the instructions in their canonical form.

One user breakpoint address may be specified using a text file named
"breakpoint.txt" located in the Nuance working directory.  If this text file
is present, the emulator will halt any MPE that attempts to execute an
instruction at that address.  This can be used to emulate a 'run to
breakpoint' mode.  To run a program without the breakpoint, simply set the
breakpoint address to 0.  The text file may contain multiple lines, but only
the first line is parsed.  The first line should contain a 32-bit hexidecimal
address, e.g. "20301ABC" followed by a carriage return.

There are two interlaced fields per frame.  A real NTSC Nuon-based player displays
at 60 fields per second.

Joystick emulation via keyboard is active only in the video display window.
The video display window may be resized to any desired size and the video
output will be stretched to fit.  To quit the application, stop the processor
emulation and then click the close button of the Control Panel window.

Loading commercial games
========================
I know that many users are only going to care about playing the games.  This
set of steps should serve as a foolproof method for successfully runnning a 
commercial game.  Then again, I'm sure theres a fool waiting to prove me wrong.

(1) If you want to run one of the supported commercial games from a DVD player,
simply skip to step (3).  The emulator automatically sets the DVDBase variable to the
directory containing the selected program.

(2)  If you want to run of the supported commercial games from some other media
type, copy all files from the Nuon directory on the DVD to a directory on the
media from which you wish to load the files.  Most games will only require the
nuon.run and nuon.dat files found in the Nuon directory of the DVD.  Some games
may have additional data files that are required, but if the nuon.run and nuon.dat
files are not copied, you are doomed to certain failure.

The emulator does *not* support any form of image file.  The files must be
accessable via standard file system calls.  An image file mounted in a virtual
DVD device will work but the image must be a complete DVD image.

(3) Click the "Load File" button.  

Dynamic Compilation Options
===========================
Nuance is now capable of compiling basic blocks of code, performing optimizations
on the code block and interpreting the optimized instruction sequence.  This allows
enhanced performance due to code optimization and reduction of dispatch loop overhead,
particularly where multiprocessor synchronization and interrupt processing is involved.

This section describes the configuration file entries that apply to the dynamic compiler.
For more information on the implementation details of the compiler, please read the
'DynamicCompiler.txt' file included in the distribution package.

The configuration entries applicable to dynamic compilation are:

[DynamicCompiler]
  Enabled/Disabled: forces (slower) interpretation when disabled, recommended: Enabled

[CompilerConstantPropagation]
  Enabled/Disabled: toggles constant propagation phase that is performed after fetching
  the block instructions
  NOTE: Still buggy, recommended: Disabled

[CompilerDeadCodeElimination]
  Enabled/Disabled: toggles dead code elimination phase that is performed after constant
  propagation
  NOTE: Still buggy, recommended: Disabled

[DumpCompiledBlocks]
  NOTE: This option is only available in custom builds, not released versions!
  Enabled/Disabled: toggles file dump of blocks that are compiled during execution.  When
  enabled, the emulator will print the resulting optimized blocks in a readable format to
  the files SuperBlocks0.txt through SuperBlocks3.txt corresponding to MPE0 through MPE3.
  Note that these files can grow in size very quickly.  It is not uncommon for the files to
  grow beyond 500 megabytes or even wrap around the maximum file sise of 4 GB.  There are
  not too many text editors that will successfully open a file greater than 400 MB and so
  if a dump file grows this large it is not recommended to open it else the chances of
  locking up your system is high, recommended: Disabled

[T3KCompilerHack]
  Enable/Disabled: toggles a compiler hack that avoids errors in T3K that occur even without
  optimization enabled.  The penalty is an increase in non-compilable instructions and less
  chance for constant propagation.
  NOTE: This does not seem to be needed nowadays anymore, but i left it in for now, lets see, recommended: Disabled
        (so please use the recommendation in GameCompatibility.txt instead)

Flash ROM Support
=================
Nuance provides emulation support for the Atmel AT49BV162A/163A flash EEPROM chips.  These
chips provide 2MB of ROM space starting at address $F0000000.  The flash interface is compatible
with the VMLabs libflash library.  When the emulator is started, the virtual flash ROM will be
populated with data from the file "flashEEPROM.bin" if it exists.  When the emulator is exited,
the virtual flash ROM contents will be dumped back out to the "flashEEPROM.bin" file.

The virtual flash EEPROM memory range can be accessed via standard MEM unit load instructions.
The emulator cannot currently execute instructions out of ROM.  Please note that the ROM emulation
is not perfect and can cause corrupted bytes to be output in a repetitive fashion when running the
Samsung BIOS update program.

printf Debugging (kprintf) Support
==================================

The Nuon rombios library implements a function 'kprintf' which isn't included
in any of the SDK headers, but is defined as follows:

  extern void kprintf(const char *fmt, ...);

And behaves like one would expect printf to. Nuance implements this BIOS
function by storing the generated text in a ring buffer that can be viewed in
the status dialog by clicking the "kprintf Log" button. Additionally, Nuance
can log the kprintf output to a file. To enable logging, include the
[DebugLogFile] section in your nuance.cfg file, and include the filename (or
full path) of the desired log file on the next line, e.g.:

  [DebugLogFile]
  nuance_debug.log

Here is a small sample code listing demonstrating how to use the functionality
in a Nuon program:

  #include <nuon/bios.h>
  extern void kprintf(const char *fmt, ...);

  int main(void) {
      kprintf("Hello, world! %d %d %d\n", 1, 2, 3);
      return 0;
  }

Compiling and running this program in Nuance will output:

  Hello, world! 1 2 3

In the kprintf Log status and the debug log file, if configured.

Known Issues
============

Compiler issues:

The constant propagation optimizations are still being debugged.  Some programs can
be safely run with constant propagation turned on while others cannot.  Dead code
elimination is extremely buggy.  Some programs will appear to be working smoothly only
to freeze unexpectedly later on.  Play around with compiler options to determine the best
setting for a particular program.  The T3KCompilerHack option must be enabled in order for
T3K to avoid crashing during level selection, even if compiler optimizations are disabled.
NOTE: This hack does not seem to work anymore on nowadays systems, so please use the
recommendation in GameCompatibility.txt instead

Pixel shader issues:

The current pixel shader only fully supports pixel mode 2 (16-bit, no Z-buffer) & 4 (32-bit, no Z-buffer).
Others partially fall back to the CPU.

Speed issues:

This is a CPU intensive emulator and there is no way around that.

NISE support:

The emulator does not use a native implementation of the NISE mixer.  This means that
if your system cannot run Nuance reasonably fast, you can expect audio to sound
distorted, slow and possibly several octaves lower than expected.  When running with
dynamic recompilation enabled, sound output is reasonably good and so audio interrupts
are enabled by default.  Audio interrupts can be disabled via the configuration file
but some programs may not work correctly if audio interrupts are not enabled.  In
particular, some games will wait for a particular sound to complete where the completion
is determined by NISE having had played all samples.  If audio interrupts are disabled,
NISE will never update the sample pointer and hence will never update the sound channel as
being available.  This is observable in Chomp where running into a ghost will lock up the
game as the "player died" sound never "finishes" causing a wait loop to execute indefinitely.

SYNTH support:

At this point in time, libSYNTH is not supported.  Unlike libNISE, the synth code 
configures the audio DAC to interrupt after each sample.  This is impossible to
emulate using a timer based approach.  It may be possible to use a performance timer
(e.g. timestamp counter) but this method of timing is not currently supported.

Sound support:

Audio is implemented using the FMOD library.  The emulator supports all DAC
configurations made possible through the Nuon BIOS audio routines.

Multi-cycle instructions:

The dot product, multiplication and load instructions take two cycles to
execute on the real Aries processor.  As with most emulators, this behavior
will not be emulated.  The implementations of these instructions will be such
there is an imaginary pipeline stall after a packet containing one of these
instructions.  Since code must be written to assume that there is no stall,
this implementation choice is not a problem since code should not be accessing
the destination register until two cycles after a dot product, multiply or
load is issued.  

BIOS calls:

The emulator supports BIOS calls but not all BIOS functions are implemented
at this time.  It is safe to call unimplemented BIOS functions but they will
not perform any useful actions.  Please read bios_support.txt for a list of
implemented BIOS calls.  The current BIOS implementation does not passing
pointer parameters that point to a remote MPE address.  For example, if
MPE3 makes a BIOS call that takes a pointer parameter, the pointer must point
to local memory on MPE3.  

MINIbios:

Two minibios implementations allow MediaInitMPE to install the minibios on
any of the four MPEs.  The use of the minibios on MPEs other than MPE0 has
not been tested but should work.  The only difference between the two
implementations minibios and minibiosX is that minibiosX addresses are shifted
down by 4KB as required by the NISE library.

DMA support:

DMALinear:
All transfer modes have been implemented but only contiguous scalar mode has
been tested.  Byte mode transfers and contiguous word transfers are supported 
but may not behave true to life since no example of non-scalar transfers is
known.  Skah_T has attempted to perform byte transfers on a real Nuon player
using what looked like reasonable DMA flags but the result was a locked up
player.  Without knowing how byte transfers are really supposed to work, I have
implemented byte mode in the same fashion as scalar mode except that the
destination stride is one word and the LENGTH/XSIZE field is interpreted as
the number of words to be transfered rather than the number of scalars to be
transfered.  In this mode, the source address and destination address are both
incremented by one word for each word or partial word transfered.  This routine
will work with both Other Bus and System Bus addresses.

DMABiLinear:
Bilinear DMA is supported but only standard left-to-right, top-to-bottom DMA
direction flags have been tested beyond what is used by demos and commercial
games.  Almost all pixel modes are supported with the exception of Z-value
only reads and writes.

DMA transfer is implemented as a loop of simple memory copies.  Transfers to
and from control register memory areas will not trigger the standard register
behavior that is obtained via load and store commands.  I could implement
this feature but then I'd have to kill you.  

Single and batch mode transfers are allowed but chained DMA transfers are not
yet implemented.

Interrupts:

Interrupts are fully supported but hardware specific interrupts such as
VDP interrupts are not emulated.  Comm, timer, VDG and software interrupts
should behave as expected. Note though that timer, video and audio interrupts
(and also VSyncs) are triggered (mostly) at an 'ideal'/PC-based-timing, so that the
emulator will behave as an "over- or under-clocked" Nuon to Nuon-apps, which so far
all tested apps seem to be able to cope with though, as they feature some variant
of a syncing mechanism to not run too slow or fast. The benefit of doing emulation
like this is, that sound and gameplay will (mostly) run at the intended speed even
when running the emulator on a PC that will not be able to match 50/60 fps (full
cycle emulation speed).

Comm Bus support:

The communications bus is fully supported for communications between MPEs.
Similar to DMA implementation, the real world latency of the Comm Bus is not
emulated and it is possible for a target MPE to receive a sent packet in the
same cycle in which it was sent.  If no commrecv handlers take care of a received
packet, the packet is placed into a one entry queue for future retrieval.  If the
queue is full then the packet is dropped.

Most hardware comm bus targets are unimplemented.  Only the VDP clut and audio
configuration register read and write commands are supported.

Memory management support:

A simple memory manager is implemented that supports the necessary functionality
for the MemAlloc and MemFree BIOS calls.  Alignment requests may be any power
of two including one.  MemAlloc requests for kernel memory are not currently
allowed.  This should not be an issue because user applications are not
supposed to request kernel memory.  As with the real Nuon BIOS, the 64K range
starting at $80000000 and the 640K range at $80760000 is not made available to
user programs.

The current implementation of MemLocalScratch always returns $20100D00 and a
size of 512 bytes.  This is due to a flaw in some commercially compiled versions
of MML2D where an environment variable field will overflow for values greater
than 3968 bytes.

Framebuffer support:

The 8-bit, 16 bit, and 32 bit pixel formats are supported, with or without Z.
A real Nuon system requires that the frame buffer reside in Main Bus SDRAM.
The emulator can also handle frame buffers in System Bus DRAM.  You can also
write directly to frame buffer memory when running emulated code but on a real
Nuon system, the frame buffer should only be accessed via DMA.  The emulator
stores the frame buffer in a standard linear fashion but the Nuon video
processor stores frame buffer data in an interleaved, cache optimized fashion
in order to speed up retrieval, particularly when filtering is used.

The emulator performs YCrCb to RGB color conversion using pixel shaders.
This allows complete hardware acceleration of
the color conversion process, saving several hundred million CPU cycles per
second that are now available for use by the processor emulation thread.

Joystick support:

By default, the keyboard may be used to control Controller slots 0 and 1 using
the following keys:

A: Button Start
S: Button Nuon/Z
D: Button A
F: Button B
Q: Button LShoulder
T: Button RShoulder
W: Button C_Left
E: Button C_Down
R: Button C_Right
3: Button C_Up
Up: DPad Up
Down: DPad Down
Left: Dpad Left
Right: DPad Right
Z: switch between controller 0 and controller 1

The default controller bindings can be modified by including a
[Controller1Mappings] section in the configuration file, followed by a list of
<Nuon Controller button> = <joystick or keyboard binding> pairs. The available
controller button names are:

CPAD_UP
CPAD_RIGHT
CPAD_DOWN
CPAD_LEFT
A
B
L
R
DPAD_UP
DPAD_RIGHT
DPAD_DOWN
DPAD_LEFT
NUON
START

The format for joystick/keyboard bindings is:

<type>_<index>_<subindex>

Where <type> can be:

KEY - A keyboard key
JOYBUT - Joystick button
JOYAXIS - Joystick axis, usuallly an analog stick axis.
JOYPOV - Joystick point of view direction, e.g., a D-pad button.

<index> can take on the following values, depending on <type>:

KEY - A win32 Virtual Keycode. See
    https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    for a list of valid values. Note the value must be in decimal, not hex.
JOYBUT - A Joystick button number.
JOYAXIS - A Joystick axis index.
JOYPOV - A Joystick POV control index. Usually 0.

<subindex> cantake on the following values, depending on <type>:

KEY - Must be '0'
JOYBUT - Must be '0'
JOYAXIS - '0' = axis min direction, '1' = axis max direction
JOYPOV - '0' = up, '1' = right, '2' = down, '3' = left.

To figure which buttons/controls are which, you can use the "Properties"
button on the "Set up USB game controllers" control panel widget. Note the
buttons in the config file are indexed from zero, while the numbering in the
control panel widget starts at 1.

Aries version:

At the moment the emulator is hardwired to assume an Aries 2 generation chip

History
=======
version 0.6.7:

03/21/2025 version 0.6.6:
Implement (bi)linear address mirroring properly and enable it
Optimize (bi)linear memory reads, and some memory stores
Optimize DMA transfers (especially all that are triggered by T3K)
Fix linear DMA if Write & Dup & Direct mode
Fix linear DMA when writing to local control register
Fix missing 0x7F mask for shifts in certain mul/shift ops when using the dynamic recompiler
Implement BDMA_Type8_Read_0 (e.g. fixes Space Invaders 'Overlay' play mode)
Add zero 'area' return to GetPointerToMemory(), as some games read 'invalid' mem regions (like $0 which returns 0 on Nuon HW)

01/04/2024 version 0.6.5:
Fix loading of nuance.cfg when drag and drop is used
Hide mouse cursor when in fullscreen mode
Make rendering window behave like the others when trying to close it (e.g. clicking 'X' and Alt-F4 work now)
Fix user breakpoints
Fix 16/16z pixel format bilinear DMA
Enable kprintf outputs (also new documentation about it within this readme file)
Fix regression in code interpreter (shift+XXX opcodes)
Slightly optimize code interpreter (ALU add+/sub+XXX opcodes)

01/10/2023 version 0.6.4:
Fix command line file parsing, also drag and drop will now work properly
Fix wrong handling of global overlay alpha values > 0
Optimize OpenGL output shaders
Fix CRT-like shader for very high resolutions (e.g. 4K)
Retweak CRT-like shader to be a bit more realistic compared to common CRT TVs back in the days

01/04/2023 version 0.6.3:
Add new (optional) postprocessing CRT-like shader ([UseCRTshader] in the .cfg), enabled by default, looks best in fullscreen
Its now possible to specify a file that should be loaded directly from the command line,
 e.g. nuance.exe C:\NuonGame\nuon.run
 this will also automatically launch in fullscreen mode, too
Add new [Controller1Mappings] section in the .cfg along with DirectInput support for joypads/sticks
 and an additional UI dialog to configure keys/buttons
Fix a lot of problems with the MPE IRAM overlay areas and the code caching of these
Make MPE IRAM overlay caching more efficient
Merlin Racing now is playable (although the AI carts are still driving around randomly)
Update status window on single stepping, too
Add missing registers to status window
Add debug output for BIOS calls
Add new [AutomaticLoadPopup] section to configure if the automatic file selection pops up on startup or not

11/27/2022 version 0.6.2:
Tweak audio interrupts some more
Correctly set DST
Some internal buffer overflow protection
Improve debugging output a bit

09/19/2021 version 0.6.1:
Optimizations
Fix some more opcode implementations
Improve Video- and SysTimer precision

5/28/2020, version 0.6.0:
Fix corrupted gfx in (at least) T3K
Fix wrong DMAs in Ballistic and Merlin Racing (so that these display mostly (always?) correct now)
(Mostly) fix distorted sound
Optimizations all over the place (memory usage and performance)
Always enable Pixel Shaders and fully support pixel mode 2 for more performance
Remove fields-per-second and always-update-video settings in the 'nuance.cfg'
Support Nuons VSyncing mechanism if a PC is emulating 'too fast'
Resolve a lot of hangs during gameplay, although this can still happen in some demanding situations (especially on low end PCs)
Fix fullscreen toggle (via F1/ESC)
Rudimentary support for x64/64bit compiles, needs to force-disable dynamic compiler though
Enable Reshade support by using double buffering

5/30/2007, version 0.5.0:
  Fixed tons of compiler bugs.  Fixed TimeToSleep routine, allowing Snake to work again.
Added compiler support for all remaining instructions including delayed branches.  Implemented
syscall interface.  Added SetISRExitHook BIOS call.  Fixed problem with minibios MPE hanging when
audio library initialized prior to sprite library.  Added functionality required to support
bootloader.

2/05/2006, version 0.4.0:
  Fixed most BIOS related bugs to the point where the only demos that do not run
correctly are those that use libSYNTH or fancy z-buffer modes.  Added support
for FileOpen, FileClose, FileRead, FileWrite, FileLSeek, and FileStat.  Added
support for 4-bit DMA and 4-bit/8-bit framebuffer modes, fixing corrupted textures
in Tetris.  Implemented ability to recompile to native x86 machine code.

10/16/2004, version 0.3.2:
  Fixed critical IntSetVector errors which were breaking user ISRs, particularly comm
handlers.  Added support for VDG interrupts.  Fixed code in video_m32_o32 fragment
shader and made it the default fragment shader instead of video_m32.fs.  Added Atmel
AT49BV162A/163A flash emulation to duplicate the flash functionality of the Samsung N501.

7/03/2004, version 0.3.1:
  Fixed thread synchronization oversight that caused abnormally slow execution of
all programs using multiple framebuffers, namely commercial games.  Tweaked internal
cache routines to significantly reduce cache invalidation overhead and greatly improve
performance of programs with large working sets, e.g. most stuff running out of external
memory on mpe3.

6/30/2004, version 0.3.0:
  Added dynamic recompiler with support for block coalescing, constant propagation
and dead code elimination. Added support for standard non-rectangle textures.  Added
pixel shader support for pixel mode 4 (GLSL required, no overlay support).  Added
optional command line parameter for specifying configuration file.  Added compiler
options and video options to configuration file.

6/30/2004, version 0.3.0:
  Added dynamic recompiler with support for block coalescing, constant propagation
and dead code elimination. Added support for standard non-rectangle textures.  Added
pixel shader support for pixel mode 4 (GLSL required, no overlay support).  Added
optional command line parameter for specifying configuration file.  Added compiler
options and video options to configuration file.
 
4/03/2004, Version 0.2.2:
  Fixed BIOS interrupt level1 comm handler bug introduced in the previous bug
fix release.  Switched from Extgl to Glew for OpenGL extensions management.

3/24/2004, Version 0.2.1:
  Repaired bug in BIOS implementation that caused getenv to always return NULL.

3/22/2004, Version 0.2.0:
  Fixed flag bug in MSB instruction handler.  This fixes Decaying Orbit
and the NISE audio library.  Added audio playback support via the FMOD library.
Updated the minibios bios implementation to fix potential issues with the NISE
audio library.  Added a second minibios implementation as required by the NISE
audio library.  Disabled stdout and stderr edit boxes due to memory sharing
problems with FMOD.  Added BIOS support for CommmSendRecvInfo, CommSendRecv,
CommSendRecvInfo, TimerInit and SpinWait.  Rewrote TimeToSleep in Nuon assembly
to support a non-blocking version of the function that blocks only the MPE that
called the function.
  
11/25/2003, Version 0.1.0:
  Major update from 0.0.1B.  Honestly there are too many to list and I stopped
keeping track somewhere in the middle.  Dozens of critical bugs were squashed
and the process, most of the remaining SDK demos began to work including MML2D,
MML3D, M3DL and MGL demos.  Most importantly, commercial games are now playable!
In chronological order, the games that are playable are: Space Invaders XL, The
Next Tetris, Merlin Racing and Tempest 3000. The Next Tetris suffers from texture
corruption but the other three games are perfectly emulated.  This release is a
major milestone in the history of the emulator.

11/01/2003, Version 0.0.1B:
  Bugfix release of 0.0.1A.  Added a memory display to the register window
which allows MPE local memory, main bus RAM and system BUS ram to be examined
in single step mode.  The display will not update during normal emulation.
  Decoding and output dependency bugs were fixed for ST_S #nnnn,<label D>,
LD_P <label>,Vk, LD_PZ <label>,Vk, and LD_SV <label>,Vk.
  Critical instruction handler bugs were fixed in the DOTP Vi,Vj,>>svshift,Sk
and DOTP Vi,Vj,#m,Sk handlers.
  The behavior of the MPERun and MPERunThread BIOS routines were modified to
behave more like the official implementation.  This fixes graphics banding
issues in the raytrace demo.
  MPEAlloc was rewritten, eliminating faulty logic that did not seem to be
frequently encountered.
  Support for the mpectl register cycle type bits was added.  A potential
mpeWasReset bit bug was fixed.

10/19/2003, Version 0.0.1A:
  Bugfix release of 0.0.1.  A scheduling bug caused the instruction scheduler
to choose a non-optimal instruction order for packets containing three or more
instructions.  Fixing this bug drastically improves emulation performance for
affected packets.  
  A cut and paste error in the CMPWC immediate instruction
handler triggered an attempt to write back the result of the comparison to an
indeterminate register.  This caused a program crash or register file
corruption in some cases.  
  The DecodeALU32 routine incorrectly decoded the operands for the SUBWC
immediate reverse instruction causing corrupted program execution.
  The control panel window will now display a disassembly of the next
instruction packet to be executed by the currently selected MPE.  The
disassembly output is updated whenever a new MPE is selected, an MPE is
started or stopped, and when the single step button is used.  The disassembly
is not updated during normal emulation execution using the run button.

10/12/2003, Version 0.0.1: 
  The first release of Nuance.  Think of it as NuonEm out for vengeance.
The emulator is now compiled under VC7 rather than BCB.  The user interface
now uses the Qt library in place of Win32.  Additional optimizations make
Nuance considerably faster than NuonEm 0.1.6F.
