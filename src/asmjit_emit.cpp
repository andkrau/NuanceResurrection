// asmjit emit implementation for NuanceResurrection x86-64 JIT
// Provides:
// - AsmJitState global and self-test
// - NativeCodeCache asmjit block management (BeginBlock/EndBlock)
// - Label management for asmjit

#ifdef USE_ASMJIT

#include "asmjit_emit.h"
#include "NativeCodeCache.h"
#include <cstdio>
#include <cstring>

#ifndef _WIN32
// Self-test: generate a simple function that returns 42
bool asmjit_selftest()
{
    using namespace asmjit;

    JitRuntime rt;
    CodeHolder code;
    code.init(rt.environment(), rt.cpu_features());

    x86::Assembler a(&code);

    // Generate: int test() { return 42; }
    a.mov(x86::eax, 42);
    a.ret();

    typedef int (*TestFunc)();
    TestFunc fn;
    Error err = rt.add(&fn, &code);
    if (err != kErrorOk) {
        fprintf(stderr, "asmjit selftest: code generation failed (err=%d)\n", err);
        return false;
    }

    int result = fn();
    rt.release(fn);

    if (result != 42) {
        fprintf(stderr, "asmjit selftest: expected 42, got %d\n", result);
        return false;
    }

    fprintf(stderr, "asmjit selftest: PASSED (generated function returned 42)\n");

    // Test 2: generate function that writes to a 64-bit address
    CodeHolder code2;
    code2.init(rt.environment(), rt.cpu_features());
    x86::Assembler a2(&code2);

    static volatile int testVal = 0;

    // mov rax, <64-bit address>
    a2.mov(x86::rax, (uint64_t)(uintptr_t)&testVal);
    // mov dword [rax], 99
    a2.mov(x86::dword_ptr(x86::rax), 99);
    a2.ret();

    typedef void (*WriteFunc)();
    WriteFunc fn2;
    err = rt.add(&fn2, &code2);
    if (err != kErrorOk) {
        fprintf(stderr, "asmjit selftest2: code generation failed\n");
        return false;
    }

    fn2();
    rt.release(fn2);

    if (testVal != 99) {
        fprintf(stderr, "asmjit selftest2: expected 99, got %d\n", testVal);
        return false;
    }

    fprintf(stderr, "asmjit selftest2: PASSED (wrote 99 to 64-bit address)\n");

    return true;
}
#endif

// --- NativeCodeCache asmjit block management ---

void NativeCodeCache::AsmJit_BeginBlock()
{
    // Clean up any previous block
    if (asmjitCode) {
        delete asmjitAs;
        asmjitAs = nullptr;
        delete asmjitCode;
        asmjitCode = nullptr;
    }

    asmjitCode = new asmjit::CodeHolder();
    asmjitCode->init(asmjitRuntime.environment(), asmjitRuntime.cpu_features());
    asmjitAs = new asmjit::x86::Assembler(asmjitCode);

    // Reset labels
    for (int i = 0; i < MAX_ASMJIT_LABELS; i++) {
        asmjitLabels[i] = asmjit::Label();
        asmjitLabelBound[i] = false;
    }

    asmjitBlockActive = true;
}

uint32 NativeCodeCache::AsmJit_EndBlock()
{
    if (!asmjitAs || !asmjitCode) {
        asmjitBlockActive = false;
        return 0;
    }

    // Flatten code and get size
    asmjitCode->flatten();
    asmjitCode->resolve_cross_section_fixups();

    size_t codeSize = asmjitCode->code_size();
    if (codeSize == 0) {
        delete asmjitAs; asmjitAs = nullptr;
        delete asmjitCode; asmjitCode = nullptr;
        asmjitBlockActive = false;
        return 0;
    }

    // Relocate code to the current emit position in our buffer
    asmjitCode->relocate_to_base((uint64_t)(uintptr_t)pEmitLoc);

    // Copy the code to our executable buffer
    asmjitCode->copy_flattened_data(pEmitLoc, codeSize);

    // Clean up asmjit state
    delete asmjitAs; asmjitAs = nullptr;
    delete asmjitCode; asmjitCode = nullptr;
    asmjitBlockActive = false;

    return (uint32)codeSize;
}

void NativeCodeCache::AsmJit_BindLabel(uint32 labelIndex)
{
    if (!asmjitAs || labelIndex >= MAX_ASMJIT_LABELS) return;

    if (!asmjitLabelBound[labelIndex]) {
        // First use - create and bind
        if (!asmjitLabels[labelIndex].is_valid()) {
            asmjitLabels[labelIndex] = asmjitAs->new_label();
        }
        asmjitAs->bind(asmjitLabels[labelIndex]);
        asmjitLabelBound[labelIndex] = true;
    } else {
        // Already bound - just bind again (asmjit will error, but shouldn't happen)
        asmjitAs->bind(asmjitLabels[labelIndex]);
    }
}

asmjit::Label& NativeCodeCache::AsmJit_GetLabel(uint32 labelIndex)
{
    assert(labelIndex < MAX_ASMJIT_LABELS);

    // If the label was already bound (by a previous Emit_* function within the
    // same SuperBlock), create a fresh one - Emit_* helpers assume the small
    // label indices 0,1,2,... are fresh after patchMgr.Reset() at the top of
    // each handler. Without this, a JCC_Label reference resolves to the OLD
    // bind site, producing a backward jump and an infinite host-side loop.
    if (asmjitLabelBound[labelIndex] || !asmjitLabels[labelIndex].is_valid()) {
        asmjitLabels[labelIndex] = asmjitAs->new_label();
        asmjitLabelBound[labelIndex] = false;
    }
    return asmjitLabels[labelIndex];
}

#endif // USE_ASMJIT
