#include "../lib/stdlib.h" // print, input, strcmp, etc

// o linker define que o entrypoint do programa está no endereço 0x1000 da memória virtual
// "main" sempre será o entrypoint

char input_buffer[64];

int main() {
  while(true) {
    print("Shell> ");
    // TODO input_buffer -> set it as String object from STDLIB to handle strings better.
    input(input_buffer); // std::input() -> wait for input -> syscall gets last key -> wait for a different key
    if(strcmp(input_buffer, "shutdown")) {
      print("Teste");
    }

  }
}
