#ifndef memoryH
#define memoryH

class MPE;

void MemAlloc(MPE &mpe);
void MemFree(MPE &mpe);
void MemInit(MPE &mpe);
void MemLocalScratch(MPE &mpe);
void MemLoadCoff(MPE &mpe);
void DownloadCoff(MPE &mpe);
void StreamLoadCoff(MPE &mpe);

#endif
