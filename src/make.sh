cd Kernel

echo "Criando backup..."
mkdir /tmp/backup 2> /dev/null
mkdir /tmp/backup/src 2>/dev/null
cp -r ../src/* /tmp/backup/src/

rm ../Build/disk.img

rm tmp.prekernel.asm
rm tmp.bootloader.asm

cp Prekernel/BIOS/bootloader.asm tmp.bootloader.asm
cp Prekernel/BIOS/prekernel.asm tmp.prekernel.asm

cp Prekernel/BIOS/bootloader.asm bootloader.asm
cp Prekernel/BIOS/prekernel.asm prekernel.asm

KERNEL_LOCATION="10485760"

mkdir bin
mkdir bin/tmp

echo "Compilando kernel.cpp & panic.cpp"
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c kernel.cpp -o bin/tmp/kernel.o
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c panic.cpp -o bin/tmp/panic.o
echo "Compilando Memory/*"
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c Memory/Heap/Heap.cpp -o bin/tmp/heap.o
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c Memory/Base.cpp -o bin/tmp/mbase.o


#g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c Filesystem/Filesystem.cpp -o bin/tmp/fs.o


echo "Compilando Drivers"
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c Drivers/VIDEO/Video.cpp -o bin/tmp/video.o
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c Drivers/Disk.cpp -o bin/tmp/disk.o
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c Drivers/Mouse.cpp -o bin/tmp/mouse.o

echo "Compilando interrupções"
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -mno-sse -mno-mmx -mno-80387 -c Interruptions/SpuriousInterrupt.cpp -o bin/tmp/spuriousi.o
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -mno-sse -mno-mmx -mno-80387 -c Drivers/Keyboard.cpp -o bin/tmp/driver_kb.o
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -O2 -fomit-frame-pointer -mno-sse -mno-mmx -mno-80387 -c Interruptions/FPUErr.cpp -o bin/tmp/fpuerr.o
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -mno-sse -mno-mmx -mno-80387 -c Syscalls/Syscall.cpp -o bin/tmp/syscalls.o


echo "Compilando Tasks/*"
g++ -m64 -O0 -fno-builtin -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -O0 -mgeneral-regs-only -c Tasks/Process.cpp -o bin/tmp/process.o
g++ -m64 -O0 -fno-builtin -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -mgeneral-regs-only -c Tasks/KernelTasks/Watchdog.cpp -o bin/tmp/watchdog.o

g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c Utils/Base.cpp -o bin/tmp/base.o

ld -nostdlib -static -T KernelLinker.ld bin/tmp/kernel.o bin/tmp/heap.o bin/tmp/watchdog.o bin/tmp/fpuerr.o bin/tmp/syscalls.o bin/tmp/mouse.o bin/tmp/disk.o bin/tmp/driver_kb.o bin/tmp/panic.o bin/tmp/base.o bin/tmp/video.o bin/tmp/spuriousi.o bin/tmp/process.o bin/tmp/mbase.o -o bin/kernel.bin

echo "Compilando shell"

g++ -m64 -ffreestanding -fno-exceptions -fno-rtti -c Base/files/Shell.cpp -o bin/tmp/shell.o
ld -nostdlib -static -T Base/files/linker.ld bin/tmp/shell.o -o bin/shell.bin

#dd if=/dev/zero of=../Build/disk.img bs=1M count=50

SECTORS_KERNEL=$(stat -c%s "bin/kernel.bin")
SECTORS_KERNEL=$(($SECTORS_KERNEL / 512))
if [ $(($SECTORS_KERNEL * 512)) -lt $(stat -c%s "bin/kernel.bin") ] ; then
  SECTORS_KERNEL=$((SECTORS_KERNEL + 1))
fi

sed -i "s/SECTORS_KERNEL/$SECTORS_KERNEL/g" prekernel.asm
sed -i "s/KERNEL_LOCATION/$KERNEL_LOCATION/g" prekernel.asm
sed -i "s/SECTORS_KERNEL/$SECTORS_KERNEL/g" bootloader.asm
#cat prekernel.asm
nasm -f bin -o bin/prekernel.bin prekernel.asm

# TODO kernel = começa no setor 100 sempre
SECTORS_PREKERNEL=$(stat -c%s "bin/prekernel.bin")
SECTORS_PREKERNEL=$(($SECTORS_PREKERNEL / 512))
if [ $(($SECTORS_PREKERNEL * 512)) -lt $(stat -c%s "bin/prekernel.bin") ]; then
    SECTORS_PREKERNEL=$(($SECTORS_PREKERNEL + 1))
fi

sed -i "s/SECTORS_PREKERNEL/$SECTORS_PREKERNEL/g" bootloader.asm
SECTORS=$(stat -c%s "bin/kernel.bin")
SECTORS=$(($SECTORS / 512))
#SECTORS=$(($SECTORS+1))
#cat bootloader.asm
nasm -f bin -o bin/bootloader.bin bootloader.asm
cat bin/bootloader.bin > bin/final
cat bin/prekernel.bin >> bin/final
#cat bin/kernel.bin >> bin/final
rm bootloader.asm
rm prekernel.asm
mv tmp.bootloader.asm bootloader.asm
mv tmp.prekernel.asm prekernel.asm
dd if=bin/final of=../Build/disk.img bs=512 count=20000 conv=notrunc
dd if=bin/kernel.bin of=../Build/disk.img bs=512 seek=100 conv=notrunc

#dd if=bin/shell.bin of=../Build/disk.img bs=512 seek=500 conv=notrunc

file_path="../Build/disk.img"
block_size=512

file_size=$(stat -c%s "$file_path")
remainder=$(($file_size % $block_size))

if [ $remainder -ne 0 ]; then
  padding=$(($block_size - $remainder))
  dd if=/dev/zero bs=1 count=$padding >> "$file_path"
  echo "Arquivo alinhado com sucesso."
else
  echo "O arquivo já está alinhado."
fi

truncate ../Build/disk.img --size=100M

#rm ../Build/disk.vdi
#VBoxManage convertfromraw --format VDI ../Build/disk.img ../Build/disk.vdi
#VBoxManage internalcommands sethduuid ../Build/disk.vdi

echo "Building user-land stuff"
python3 build_userland.py

rm prekernel.asm
rm bootloader.asm

sleep 1
sh run.sh
