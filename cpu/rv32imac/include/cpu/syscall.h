
// SPDX-License-Identifier: MIT

#include "attributes.h"



// System call with no arguments.
static inline long long syscall_0(long _sysnum) FORCEINLINE;
static inline long long syscall_0(long _sysnum) {
    register long      a7 asm("a7") = _sysnum;
    register long long out asm("a0");
    asm volatile("ecall" : "=r"(out) : "r"(a7));
    return out;
}

// System call with 1 argument.
static inline long long syscall_1(long _sysnum, long _a0) FORCEINLINE;
static inline long long syscall_1(long _sysnum, long _a0) {
    register long      a0 asm("a0") = _a0;
    register long      a7 asm("a7") = _sysnum;
    register long long out asm("a0");
    asm volatile("ecall" : "=r"(out) : "r"(a7), "r"(a0));
    return out;
}

// System call with 2 arguments.
static inline long long syscall_2(long _sysnum, long _a0, long _a1) FORCEINLINE;
static inline long long syscall_2(long _sysnum, long _a0, long _a1) {
    register long      a0 asm("a0") = _a0;
    register long      a1 asm("a1") = _a1;
    register long      a7 asm("a7") = _sysnum;
    register long long out asm("a0");
    asm volatile("ecall" : "=r"(out) : "r"(a7), "r"(a0), "r"(a1));
    return out;
}

// System call with 3 arguments.
static inline long long syscall_3(long _sysnum, long _a0, long _a1, long _a2) FORCEINLINE;
static inline long long syscall_3(long _sysnum, long _a0, long _a1, long _a2) {
    register long      a0 asm("a0") = _a0;
    register long      a1 asm("a1") = _a1;
    register long      a2 asm("a2") = _a2;
    register long      a7 asm("a7") = _sysnum;
    register long long out asm("a0");
    asm volatile("ecall" : "=r"(out) : "r"(a7), "r"(a0), "r"(a1), "r"(a2));
    return out;
}

// System call with 4 arguments.
static inline long long syscall_4(long _sysnum, long _a0, long _a1, long _a2, long _a3) FORCEINLINE;
static inline long long syscall_4(long _sysnum, long _a0, long _a1, long _a2, long _a3) {
    register long      a0 asm("a0") = _a0;
    register long      a1 asm("a1") = _a1;
    register long      a2 asm("a2") = _a2;
    register long      a3 asm("a3") = _a3;
    register long      a7 asm("a7") = _sysnum;
    register long long out asm("a0");
    asm volatile("ecall" : "=r"(out) : "r"(a7), "r"(a0), "r"(a1), "r"(a2), "r"(a3));
    return out;
}

// System call with 5 arguments.
static inline long long syscall_5(long _sysnum, long _a0, long _a1, long _a2, long _a3, long _a4) FORCEINLINE;
static inline long long syscall_5(long _sysnum, long _a0, long _a1, long _a2, long _a3, long _a4) {
    register long      a0 asm("a0") = _a0;
    register long      a1 asm("a1") = _a1;
    register long      a2 asm("a2") = _a2;
    register long      a3 asm("a3") = _a3;
    register long      a4 asm("a4") = _a4;
    register long      a7 asm("a7") = _sysnum;
    register long long out asm("a0");
    asm volatile("ecall" : "=r"(out) : "r"(a7), "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
    return out;
}

// System call with 6 arguments.
static inline long long syscall_6(long _sysnum, long _a0, long _a1, long _a2, long _a3, long _a4, long _a5) FORCEINLINE;
static inline long long syscall_6(long _sysnum, long _a0, long _a1, long _a2, long _a3, long _a4, long _a5) {
    register long      a0 asm("a0") = _a0;
    register long      a1 asm("a1") = _a1;
    register long      a2 asm("a2") = _a2;
    register long      a3 asm("a3") = _a3;
    register long      a4 asm("a4") = _a4;
    register long      a5 asm("a5") = _a5;
    register long      a7 asm("a7") = _sysnum;
    register long long out asm("a0");
    asm volatile("ecall" : "=r"(out) : "r"(a7), "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
    return out;
}

// System call with 7 arguments.
static inline long long syscall_7(long _sysnum, long _a0, long _a1, long _a2, long _a3, long _a4, long _a5, long _a6)
    FORCEINLINE;
static inline long long syscall_7(long _sysnum, long _a0, long _a1, long _a2, long _a3, long _a4, long _a5, long _a6) {
    register long      a0 asm("a0") = _a0;
    register long      a1 asm("a1") = _a1;
    register long      a2 asm("a2") = _a2;
    register long      a3 asm("a3") = _a3;
    register long      a4 asm("a4") = _a4;
    register long      a5 asm("a5") = _a5;
    register long      a6 asm("a6") = _a6;
    register long      a7 asm("a7") = _sysnum;
    register long long out asm("a0");
    asm volatile("ecall" : "=r"(out) : "r"(a7), "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6));
    return out;
}
