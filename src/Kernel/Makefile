# Variáveis de compilação
CC = g++
CFLAGS = -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti
ASM = nasm
ASMFLAGS = -f elf64
LD = ld
LDFLAGS = -nostdlib -static -T Core/KernelLinker.ld
BIN = bin
OBJ = $(BIN)/tmp
KERNEL_LOCATION = 10485760

# Arquivos
C_SRC = $(wildcard *.cpp Memory/*.cpp Core/*.cpp Drivers/*.cpp Interruptions/*.cpp Syscalls/*.cpp Tasks/*.cpp Utils/*.cpp Filesystem/*.cpp)
ASM_SRC = $(wildcard Interruptions/*.asm Syscalls/*.asm)
OBJ_FILES = $(patsubst %.cpp,$(OBJ)/%.o,$(C_SRC)) $(patsubst %.asm,$(OBJ)/%.o,$(ASM_SRC))

# Regra padrão
all: dirs kernel

# Cria os diretórios necessários
dirs:
	mkdir -p $(OBJ)

# Compila o kernel
kernel: $(OBJ_FILES)
	$(LD) $(LDFLAGS) $^ -o $(BIN)/kernel.bin

# Regras de compilação
$(OBJ)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Limpar arquivos
clean:
	rm -rf $(BIN)

