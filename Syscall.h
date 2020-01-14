#ifndef MPE_SYSCALL_H
#define MPE_SYSCALL_H

#include "basetypes.h"
#include "mpe.h"

void ExecuteSyscall(MPE *mpe, uint32 syscall);
void Syscall_InterruptTriggered(MPE *mpe);

#endif