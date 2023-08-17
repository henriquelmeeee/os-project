#include "Stack.h"
#include "../Utils/Base.h"

extern "C" __attribute__((interrupt)) void i_fpuerr(struct ExceptionState *s) {
  CLI;
  dbg("a");
  __asm__ volatile("hlt");
  //dbg("CPU: EXCEÇÃO ENCONTRADA!\nFPU-Error #MF\n");
  /*unsigned short control_word;

  __asm__ volatile("fstcw %0;" // FPU set control word (is an fpu register) 
      : "=m" (control_word)
      :
      :
      );


  control_word |= 0x3F; // disable all exceptions from FPU
  __asm__ volatile("fldcw %0;"
      :
      : "m" (control_word)
      :
      );
  */
  //dbg("saindo\n");
  __asm__ volatile(
      //"add $2, %%rsp;"
      "mov %%cr0, %%rax;"
      "mov $0xFFFFFFF7, %%rbx;"
      "and %%rbx, %%rax;"
      "mov %%rax, %%cr0;"
      "fnclex;"
      "fninit;"
      "fnclex;"
      :
      :
      :"%rax","cc","memory","%rbp","%rsp","%rbx" 
      );
}
