#ifndef dmaH
#define dmaH

#include "basetypes.h"

class MPE;

#define GetPixBaseAddr(base,offset,shift) (base + (offset << shift))

typedef void (* BilinearDMAHandler)(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr);
typedef void BilinearDMAHandlerProto(MPE *, uint32, uint32, uint32, uint32, uint32);

void DMALinear(MPE *mpe, uint32 flags, uint32 baseaddr, uint32 intaddr);
void DMALinear(MPE *mpe);
void DMABiLinear(MPE *the_mpe, uint32 flags, uint32 baseaddr, uint32 xinfo, uint32 yinfo, uint32 intaddr);
void DMABiLinear(MPE *mpe);
void DMAWait(MPE *mpe);
void DMADo(MPE *mpe);

#endif
