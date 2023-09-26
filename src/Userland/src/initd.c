volatile int main() {
  __asm__ volatile("syscall");
  while(1==1);
}
