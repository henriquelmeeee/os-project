nasm -f elf64 start.asm -o tmp/start.o

gcc -nostdlib -fno-builtin -o tmp/initd.o -c initd.c

ld -m elf_x86_64 -o ../apps/initd tmp/start.o tmp/initd.o -T linker.ld
