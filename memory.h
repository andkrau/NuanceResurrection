#ifndef memoryH
#define memoryH

class MPE;

void MemAlloc(MPE *);
void MemFree(MPE *);
void MemInit(MPE *);
void MemLocalScratch(MPE *the_mpe);

#endif