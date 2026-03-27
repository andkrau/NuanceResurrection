// asmjit wrapper for NuanceResurrection x86-64 JIT
// Maps existing X86Emit_* interface to asmjit code generation
// This enables 64-bit native code emission (replacing the 32-bit raw byte emitter)
#ifndef ASMJIT_EMIT_H
#define ASMJIT_EMIT_H

#ifdef USE_ASMJIT

#include <asmjit/asmjit.h>
#include "X86EmitTypes.h"

// Map Nuance x86Reg to asmjit registers
namespace NuanceJit {

using namespace asmjit;

// 32-bit GP register mapping
inline x86::Gp toGp32(x86Reg reg) {
    switch (reg) {
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

// 64-bit GP register mapping (extends 32-bit)
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

// XMM register mapping
inline x86::Vec toXmm(x86Reg reg) {
    int idx = (int)reg - (int)x86Reg::x86Reg_xmm0;
    if (idx < 0 || idx > 7) idx = 0;
    return x86::Vec::make_xmm(idx);
}

// Convert condition code
inline x86::CondCode toCC(uint32_t cc) {
    switch (cc) {
        case X86_CC_O:   return x86::CondCode::kO;
        case X86_CC_NO:  return x86::CondCode::kNO;
        case X86_CC_B:   return x86::CondCode::kB;
        case X86_CC_NB:  return x86::CondCode::kNB;
        case X86_CC_Z:   return x86::CondCode::kZ;
        case X86_CC_NZ:  return x86::CondCode::kNZ;
        case X86_CC_BE:  return x86::CondCode::kBE;
        case X86_CC_NBE: return x86::CondCode::kNBE;
        case X86_CC_S:   return x86::CondCode::kS;
        case X86_CC_NS:  return x86::CondCode::kNS;
        case X86_CC_L:   return x86::CondCode::kL;
        case X86_CC_NL:  return x86::CondCode::kNL;
        case X86_CC_LE:  return x86::CondCode::kLE;
        case X86_CC_NLE: return x86::CondCode::kNLE;
        default: return x86::CondCode::kZ;
    }
}

// Memory reference from absolute address (64-bit)
inline x86::Mem memAbs(uintptr_t addr) {
    return x86::ptr(addr);
}

// Memory reference from base register + displacement
inline x86::Mem memRegDisp(x86BaseReg base, int32_t disp) {
    return x86::ptr(toGp64((x86Reg)((int)x86Reg::x86Reg_eax + (int)base)), disp);
}

} // namespace NuanceJit

// Global asmjit runtime and assembler state
// These are initialized by NativeCodeCache when USE_ASMJIT is defined
struct AsmJitState {
    asmjit::JitRuntime runtime;
    asmjit::CodeHolder* code = nullptr;
    asmjit::x86::Assembler* as = nullptr;

    bool init() {
        code = new asmjit::CodeHolder();
        code->init(runtime.environment(), runtime.cpu_features());
        as = new asmjit::x86::Assembler(code);
        return true;
    }

    void* finalize(size_t& codeSize) {
        if (!as || !code) return nullptr;
        void* fn = nullptr;
        asmjit::Error err = runtime.add(&fn, code);
        if (err != asmjit::kErrorOk) return nullptr;
        codeSize = code->code_size();
        delete as; as = nullptr;
        delete code; code = nullptr;
        return fn;
    }

    void reset() {
        delete as; as = nullptr;
        delete code; code = nullptr;
    }
};

extern AsmJitState g_asmjit;

#endif // USE_ASMJIT
#endif // ASMJIT_EMIT_H
