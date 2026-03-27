// asmjit emit wrapper - proof of concept
// This file provides the asmjit global state and a simple test
// Full integration will replace X86Emit_* functions in NativeCodeCache

#ifdef USE_ASMJIT

#include "asmjit_emit.h"
#include <cstdio>

AsmJitState g_asmjit;

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

    // Test 2: generate function that calls a C function
    CodeHolder code2;
    code2.init(rt.environment(), rt.cpu_features());
    x86::Assembler a2(&code2);

    // Test loading a 64-bit pointer into register and calling function through it
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

#endif // USE_ASMJIT
