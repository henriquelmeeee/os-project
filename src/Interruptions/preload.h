#include "Stack.h"

extern "C" void i_spurious(struct State *s);
extern "C" void __attribute__((interrupt)) i_fpuerr(struct ExceptionState *s);
