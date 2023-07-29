
char sys$get_last_key() {
  char ret_value;
  __asm__ volatile(
      "pusha;"
      "movl %0, %%eax;" // syscall id 0 = get last key
      "int $0x80;"
      "movb %%al, %0;"
      "popa;"
      : "=r" (ret_value)
      :
      : "eax"
      );
  return ret_value;
}

char inline input() {
  char last_key = sys$get_last_key();
  char actual_key = last_key;
  while(last_key == actual_key) {
    actual_key = sys$get_last_key();
  }
  return actual_key;
}

void input_wait_for_nullbyte(char* buffer) {
  char keys[64];
  int keys_number = 1;
  char actual_key = input();
  keys[0] = actual_key;
  while(actual_key != 0) {
    actual_key = input();
    keys[keys_number] = actual_key;
    ++keys_number;
  }
}
