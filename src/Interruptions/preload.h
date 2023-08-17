#pragma once

#ifndef PRELOAD
#define PRELOAD

struct SpuriousInterrupt {

};

extern "C" void i_spurious(SpuriousInterrupt *s);
//extern "C" void __attribute__((interrupt)) i_fpuerr(struct ExceptionState *s);
#endif
