#ifndef PRESENTATION_ENGINE_H
#define PRESENTATION_ENGINE_H

#include "basetypes.h"
class MPE;

void InitDVDJumpTable();
void CallPEHandler(MPE *mpe, uint32 address);
#endif
