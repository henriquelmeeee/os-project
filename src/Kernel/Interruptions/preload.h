#pragma once

#ifndef PRELOAD
#define PRELOAD

#include "../Utils/Base.h"

struct SpuriousInterrupt {

};

struct InterruptFrame {
  u64 rip;
  u64 cs;
  u64 rflags;
  u64 rsp;
  u64 ss;
};

#define SAVE_ALL_REGISTERS() \
    asm volatile ( \
        "pushq %%rax\n" \
        "pushq %%rbx\n" \
        "pushq %%rcx\n" \
        "pushq %%rdx\n" \
        "pushq %%rdi\n" \
        "pushq %%rsi\n" \
        "pushq %%rbp\n" \
        "pushq %%r8\n" \
        "pushq %%r9\n" \
        "pushq %%r10\n" \
        "pushq %%r11\n" \
        "pushq %%r12\n" \
        "pushq %%r13\n" \
        "pushq %%r14\n" \
        "pushq %%r15\n" \
        : /* No output */ \
        : /* No input */ \
        : "memory" /* Clobbered register */ \
    )

#define RESTORE_ALL_REGISTERS() \
    asm volatile ( \
        "popq %%r15\n" \
        "popq %%r14\n" \
        "popq %%r13\n" \
        "popq %%r12\n" \
        "popq %%r11\n" \
        "popq %%r10\n" \
        "popq %%r9\n" \
        "popq %%r8\n" \
        "popq %%rbp\n" \
        "popq %%rsi\n" \
        "popq %%rdi\n" \
        "popq %%rdx\n" \
        "popq %%rcx\n" \
        "popq %%rbx\n" \
        "popq %%rax\n" \
        : /* No output */ \
        : /* No input */ \
        : "memory" /* Clobbered register */ \
    )

#define CLEAR_ALL_REGISTERS() \
    asm volatile ( \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        "addq $8, %%rsp\n" \
        : /* No output */ \
        : /* No input */ \
        : "rsp" /* Clobbered register */ \
    )

extern "C" void i_spurious(SpuriousInterrupt *s);
//extern "C" void __attribute__((interrupt)) i_fpuerr(struct ExceptionState *s);
#endif
