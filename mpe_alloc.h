#ifndef mpe_allocH
#define mpe_allocH

class MPE;

/* definition to use in _MPEAlloc to get any MPE */
#define MPE_ANY           0

/* definitions for specific MPE capabilities */
#define MPE_HAS_ICACHE    0x01
#define MPE_HAS_DCACHE    0x02
#define MPE_HAS_CACHES    (MPE_HAS_ICACHE | MPE_HAS_DCACHE)

#define MPE_IRAM_8K       0x04
#define MPE_DTRAM_8K      0x08

#define MPE_HAS_MINI_BIOS 0x10

#define MPE_USER_FLAGS 0x00ffffff

/* the flags that are not "MPE_USER_FLAGS" are reserved for BIOS
 * use and cannot be passed to _MPEAlloc
 */

/* special flags for MPE's being allocated */
#define MPE_ALLOC_USER 0x01000000  /* allocated by user */
#define MPE_ALLOC_BIOS 0x02000000  /* allocated by BIOS (e.g. for CDI) */
#define MPE_ALLOC_ANY (MPE_ALLOC_USER|MPE_ALLOC_BIOS)

/* function definitions */

void ResetMPEFlags(MPE *mpe);
void MPEAlloc(MPE *mpe);
void MPEAllocSpecific(MPE *mpe);
void MPEFree(MPE *mpe);
void MPEStatus(MPE *mpe);
void MPEsAvailable(MPE *mpe);
void MPERun(MPE *mpe);
void MPERunThread(MPE *the_mpe);
void MPEStop(MPE *mpe);
void MPELoad(MPE *mpe);
void MPEReadRegister(MPE *the_mpe);
void MPEWriteRegister(MPE *the_mpe);

#endif
