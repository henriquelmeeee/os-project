cd Kernel

echo "Criando backup..."
mkdir /tmp/backup 2> /dev/null
mkdir /tmp/backup/src 2>/dev/null
cp -r ../src/* /tmp/backup/src/

sudo rm ../Build/disk.img

rm tmp.prekernel.asm
rm tmp.bootloader.asm

cp Prekernel/BIOS/bootloader.asm tmp.bootloader.asm
cp Prekernel/BIOS/prekernel.asm tmp.prekernel.asm

cp Prekernel/BIOS/bootloader.asm bootloader.asm
cp Prekernel/BIOS/prekernel.asm prekernel.asm

KERNEL_LOCATION="10485760"

mkdir bin 2>/dev/null
mkdir bin/tmp 2>/dev/null


GPP_CMDLINE="g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c "

echo "Compilando kernel.cpp & panic.cpp"
$GPP_CMDLINE kernel.cpp -o bin/tmp/kernel.o
$GPP_CMDLINE Core/panic.cpp -o bin/tmp/panic.o
echo "Compilando Memory/*"
$GPP_CMDLINE Memory/Heap/Heap.cpp -o bin/tmp/heap.o
$GPP_CMDLINE Memory/Memory.cpp -o bin/tmp/memory.o

$GPP_CMDLINE Core/Debugging/Symbols.cpp -o bin/tmp/symbols.o

#g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c Filesystem/Filesystem.cpp -o bin/tmp/fs.o


echo "Compilando Drivers"
$GPP_CMDLINE Drivers/VIDEO/Video.cpp -o bin/tmp/video.h
$GPP_CMDLINE Drivers/Disk.cpp -o bin/tmp/disk.o
$GPP_CMDLINE Drivers/Mouse.cpp -o bin/tmp/mouse.o 

echo "Compilando interrupções"
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -mno-sse -mno-mmx -mno-80387 -O0 -c Drivers/Keyboard.cpp -o bin/tmp/driver_kb.o
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -mno-sse -mno-mmx -mno-80387 -O0 -c Interruptions/SpuriousInterrupt.cpp -o bin/tmp/spuriousi.o

nasm -f elf64 Interruptions/ContextSwitchStub.asm -o bin/tmp/contexts_stub.o
g++ -c -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -mno-sse -mno-mmx -mno-80387 -O0 Interruptions/ContextSwitch.cpp -o bin/tmp/contexts.o
g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -O0 -fomit-frame-pointer -mno-sse -mno-mmx -mno-80387 -c Interruptions/FPUErr.cpp -o bin/tmp/fpuerr.o

nasm -f elf64 Syscalls/SyscallStub.asm -o bin/tmp/syscall_stub.o
g++ -c -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -mno-sse -mno-mmx -mno-80387 -O0 Syscalls/Syscall.cpp -o bin/tmp/syscalls.o

echo "Compilando Tasks/*"
g++ -m64 -O0 -fno-builtin -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -O0 -mgeneral-regs-only -c Tasks/Process.cpp -o bin/tmp/process.o
g++ -m64 -O0 -fno-builtin -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -mgeneral-regs-only -c Tasks/KernelTasks/Watchdog.cpp -o bin/tmp/watchdog.o

g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c Utils/Base.cpp -o bin/tmp/base.o

g++ -m64 -fno-PIC -ffreestanding -fno-exceptions -fno-rtti -c Filesystem/Filesystem.cpp -o bin/tmp/fs.o

$GPP_CMDLINE HAL/HAL.cpp -o bin/tmp/hal.o
$GPP_CMDLINE HAL/Devices/APIC.cpp -o bin/tmp/hal_apic.o
$GPP_CMDLINE HAL/Devices/PIC.cpp -o bin/tmp/hal_pic.o
$GPP_CMDLINE HAL/Devices/Processor.cpp -o bin/tmp/hal_processor.o

ld -nostdlib -static -T Core/KernelLinker.ld bin/tmp/*.o -o bin/kernel.bin

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
echo $SECTORS_KERNEL
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
#python3 ../Userland/build_userland.py
sudo umount /dev/loop0 2>/dev/null
sudo rm -rf /tmp/k_tmp
sudo losetup --offset $((1000*512)) /dev/loop0 ../Build/disk.img
sudo mke2fs -b 4096 -t ext2 /dev/loop0
sudo mkdir /mnt/k_tmp 2>/dev/null
sudo mount /dev/loop0 /mnt/k_tmp


#sudo cp ../Userland/teste /mnt/k_tmp
sudo chmod 777 /mnt/k_tmp
sudo cp ../Userland/apps/initd /mnt/k_tmp/initd
sudo echo "w" > /mnt/k_tmp/teste
#sudo mkdir /mnt/k_tmp/diretorio
#sudo mkdir /mnt/k_tmp/outro_dir
#sudo chmod 777 /mnt/k_tmp/diretorio
#sudo echo "abc" > /mnt/k_tmp/diretorio/arquivo
#sudo ls -lah /mnt/k_tmp/diretorio
sudo rm -rf /mnt/k_tmp/lost+found 2>/dev/null

echo "User-land:"
ls -lah /mnt/k_tmp
sudo dumpe2fs /dev/loop0
sync

sudo umount /mnt/k_tmp
sudo losetup -d /dev/loop0
sudo rm -rf /mnt/k_tmp
sudo rm -rf /tmp/k_tmp
rm prekernel.asm
rm bootloader.asm

sh ../run.sh
