#ifndef memoryH
#define memoryH

class MPE;

void MemAlloc(MPE * const);
void MemFree(MPE * const);
void MemInit(MPE * const);
void MemLocalScratch(MPE * const the_mpe);

#endif