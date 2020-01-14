Mike Perry
Application ID: 40592
Code Samples

This disc contains code samples from my Nuon emulator as follows:

NuonEmulationPresentation.ppt
----------------------------------------------

This is a PowerPoint format presentation that covers the Nuon architecture, Aries instruction set, basic dynamic binary translation methods and project summary.  The presentation was originally given at Starkey Labs in 2004 as part of the interview process.

Instruction test
===========

InstructionTest.c is a Nuon utility that uses the nuontest.s assembly instruction test code to verify correct behavior of a large portion of the emulated instruction set running in both interpreted mode and dynamic binary translation mode.

Nuance miscellaneous
=================

MemoryManager.cpp/MemoryManager.h
-----------------------------------------------------------
This class implements a block based memory manager used as the basis of the Nuon memory related BIOS routines.  The memory manager allows allocation of a specified memory size with optional alignment on a power-of-two memory address.

FlashEEPROM.cpp/FlashEEPROM.h
-------------------------------------------------------
This class provides an interface to emulate the Atmel AT49BV162A/163A flash memory on the Nuon with enough functionality that the Samsung BIOS update program can be run on the emulator to dump the Samsung N501 BIOS to a file.


