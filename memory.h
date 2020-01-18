#ifndef memoryH
#define memoryH

class MPE;

void MemAlloc(MPE &mpe);
void MemFree(MPE &mpe);
void MemInit(MPE &mpe);
void MemLocalScratch(MPE &mpe);

#endif
