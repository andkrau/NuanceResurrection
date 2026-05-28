// asmjit wrapper for NuanceResurrection x86-64 JIT
// Maps existing X86Emit_* interface to asmjit code generation
// This enables 64-bit native code emission (replacing the 32-bit raw byte emitter)
#ifndef ASMJIT_EMIT_H
#define ASMJIT_EMIT_H

#ifdef USE_ASMJIT

// X11/Xlib.h defines "Bool" as a macro which conflicts with asmjit::Type::Bool
#ifdef Bool
#undef Bool
#endif

#include <asmjit/asmjit.h>
#include "X86EmitTypes.h"

#define MAX_ASMJIT_LABELS 64

// Map Nuance x86Reg/x86BaseReg/x86IndexReg to asmjit registers
namespace NuanceJit {

using namespace asmjit;

// 32-bit GP register mapping from x86Reg enum
inline x86::Gp toGp32(x86Reg reg) {
    switch (reg) {
        case x86Reg::x86Reg_al:  return x86::al;
        case x86Reg::x86Reg_cl:  return x86::cl;
        case x86Reg::x86Reg_dl:  return x86::dl;
        case x86Reg::x86Reg_bl:  return x86::bl;
        case x86Reg::x86Reg_ah:  return x86::ah;
        case x86Reg::x86Reg_ch:  return x86::ch;
        case x86Reg::x86Reg_dh:  return x86::dh;
        case x86Reg::x86Reg_bh:  return x86::bh;
        case x86Reg::x86Reg_ax:  return x86::ax;
        case x86Reg::x86Reg_cx:  return x86::cx;
        case x86Reg::x86Reg_dx:  return x86::dx;
        case x86Reg::x86Reg_bx:  return x86::bx;
        case x86Reg::x86Reg_sp:  return x86::sp;
        case x86Reg::x86Reg_bp:  return x86::bp;
        case x86Reg::x86Reg_si:  return x86::si;
        case x86Reg::x86Reg_di:  return x86::di;
        case x86Reg::x86Reg_eax: return x86::eax;
        case x86Reg::x86Reg_ecx: return x86::ecx;
        case x86Reg::x86Reg_edx: return x86::edx;
        case x86Reg::x86Reg_ebx: return x86::ebx;
        case x86Reg::x86Reg_esp: return x86::esp;
        case x86Reg::x86Reg_ebp: return x86::ebp;
        case x86Reg::x86Reg_esi: return x86::esi;
        case x86Reg::x86Reg_edi: return x86::edi;
        default: return x86::eax;
    }
}

// Determine if an x86Reg is 8-bit
inline bool isReg8(x86Reg reg) {
    return reg >= x86Reg::x86Reg_al && reg <= x86Reg::x86Reg_bh;
}

// Determine if an x86Reg is 16-bit
inline bool isReg16(x86Reg reg) {
    return reg >= x86Reg::x86Reg_ax && reg <= x86Reg::x86Reg_di;
}

// Determine if an x86Reg is 32-bit
inline bool isReg32(x86Reg reg) {
    return reg >= x86Reg::x86Reg_eax && reg <= x86Reg::x86Reg_edi;
}

// 64-bit GP register mapping (extends 32-bit registers to 64-bit)
inline x86::Gp toGp64(x86Reg reg) {
    switch (reg) {
        case x86Reg::x86Reg_eax: return x86::rax;
        case x86Reg::x86Reg_ecx: return x86::rcx;
        case x86Reg::x86Reg_edx: return x86::rdx;
        case x86Reg::x86Reg_ebx: return x86::rbx;
        case x86Reg::x86Reg_esp: return x86::rsp;
        case x86Reg::x86Reg_ebp: return x86::rbp;
        case x86Reg::x86Reg_esi: return x86::rsi;
        case x86Reg::x86Reg_edi: return x86::rdi;
        default: return x86::rax;
    }
}

// Convert x86BaseReg (0-7) to asmjit 64-bit GP register
inline x86::Gp baseToGp64(int base) {
    static const x86::Gp regs[] = {
        x86::rax, x86::rcx, x86::rdx, x86::rbx,
        x86::rsp, x86::rbp, x86::rsi, x86::rdi
    };
    return regs[base & 7];
}

// Convert x86IndexReg to asmjit 64-bit GP register
inline x86::Gp indexToGp64(x86IndexReg idx) {
    static const x86::Gp regs[] = {
        x86::rax, x86::rcx, x86::rdx, x86::rbx,
        x86::rsp, // x86IndexReg_none = 4 (shouldn't be called with this)
        x86::rbp, x86::rsi, x86::rdi
    };
    return regs[(int)idx & 7];
}

// Convert x86ScaleVal to shift amount for asmjit
inline uint32_t scaleToShift(x86ScaleVal scale) {
    return (uint32_t)scale; // x86Scale_1=0, x86Scale_2=1, x86Scale_4=2, x86Scale_8=3
}

// XMM register mapping (returns Vec which is xmm-sized)
inline x86::Vec toXmm(x86Reg reg) {
    int idx = (int)reg - (int)x86Reg::x86Reg_xmm0;
    if (idx < 0 || idx > 7) idx = 0;
    return x86::xmm(idx);
}

// MMX register mapping
inline x86::Mm toMm(x86Reg reg) {
    int idx = (int)reg - (int)x86Reg::x86Reg_mm0;
    if (idx < 0 || idx > 7) idx = 0;
    return x86::mm(idx);
}

// Convert X86_CC_* condition code to asmjit CondCode
inline x86::CondCode toCC(int cc) {
    // X86_CC values match asmjit encoding directly
    return (x86::CondCode)(cc & 0xF);
}

// Get size in bytes from x86MemPtr type
inline uint32_t ptrSize(x86MemPtr ptr) {
    switch (ptr) {
        case x86MemPtr::x86MemPtr_byte:  return 1;
        case x86MemPtr::x86MemPtr_word:  return 2;
        case x86MemPtr::x86MemPtr_dword: return 4;
        case x86MemPtr::x86MemPtr_qword: return 8;
        default: return 4;
    }
}

// Build a memory operand from base/index/scale/disp.
// If base == X86_NO_BASE, it's [index*scale + disp] (no base register).
// If base > 7, it's an absolute 64-bit address - load into R15 scratch register first.
// The assembler reference is needed to emit the mov r15 instruction.
// Returns an asmjit Mem operand suitable for use in the next instruction.
inline x86::Mem buildMem(x86::Assembler& a, uintptr_t base,
                         x86IndexReg index, x86ScaleVal scale, int32_t disp,
                         uint32_t size = 4)
{
    if (base == X86_NO_BASE) {
        // Index-only: [disp + index*scale]. Encoded by x86 with SIB.base=ebp/mod=00
        return x86::ptr_abs((uint64_t)(int64_t)disp, indexToGp64(index), scaleToShift(scale), size);
    }
    if (base > 7) {
        // Absolute address - load into R15 scratch register
        a.mov(x86::r15, (uint64_t)base);
        if (index != x86IndexReg::x86IndexReg_none) {
            return x86::ptr(x86::r15, indexToGp64(index), scaleToShift(scale), disp, size);
        }
        return x86::ptr(x86::r15, disp, size);
    } else {
        // Register-based addressing
        x86::Gp baseReg = baseToGp64((int)base);
        if (index != x86IndexReg::x86IndexReg_none) {
            return x86::ptr(baseReg, indexToGp64(index), scaleToShift(scale), disp, size);
        }
        if (disp == 0 && base != 5) { // rbp needs explicit disp=0
            return x86::ptr(baseReg, 0, size);
        }
        return x86::ptr(baseReg, disp, size);
    }
}

// Build memory operand with x86MemPtr size
inline x86::Mem buildMemPtr(x86::Assembler& a, uintptr_t base,
                            x86IndexReg index, x86ScaleVal scale, int32_t disp,
                            x86MemPtr ptrType)
{
    return buildMem(a, base, index, scale, disp, ptrSize(ptrType));
}

} // namespace NuanceJit

#endif // USE_ASMJIT
#endif // ASMJIT_EMIT_H
