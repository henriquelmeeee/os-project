#pragma once

#ifndef SYSCALL
#define SYSCALL

#include "../Utils/Base.h"

struct SyscallStack {
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rbp;
    u64 rdi;
    u64 rsi;
    u64 rdx;
    u64 rcx;
    u64 rbx;
    u64 rax;
};
extern "C" void handle_syscall_routine(u64 syscall_id, u64 arg2, u64 arg3, u64 arg4, u64 arg5);
extern "C" void syscall_handler_stub();
#endif
