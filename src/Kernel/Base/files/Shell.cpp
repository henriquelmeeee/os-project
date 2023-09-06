//#include "../lib/stdlib.h" // print, input, strcmp, etc

// o linker define que o entrypoint do programa está no endereço 0x1000 da memória virtual
// "main" sempre será o entrypoint


int main() {
  asm volatile (
        "mov $0x2000, %%dx;" // Carrega o porto de I/O 0x2000 em DX
        "mov $0x2000, %%al;" // Carrega o valor 0x2000 em AL
        "out %%al, %%dx;"     // Escreve AL para o porto de I/O em DX
        :
        :
        : "dx", "al"
    );
}
