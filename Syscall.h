#ifndef MPE_SYSCALL_H
#define MPE_SYSCALL_H

#include "basetypes.h"
#include "mpe.h"

void ExecuteSyscall(MPE * const mpe, const uint32 syscall);
void Syscall_InterruptTriggered(MPE * const mpe);

#endif