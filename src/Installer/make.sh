
cp kernel.asm tmp.kernel.asm
cp bootloader.asm tmp.bootloader.asm

nasm -f bin tmp.kernel.asm -o bin/kernel.bin
SECTORS_KERNEL=$(stat -c%s "bin/kernel.bin")
SECTORS_KERNEL=$(($SECTORS_KERNEL / 512))
if [ $(($SECTORS_KERNEL * 512)) -lt $(stat -c%s "bin/kernel.bin") ]; then
    SECTORS_KERNEL=$(($SECTORS_KERNEL + 1))
fi

sed -i "s/SECTORS_KERNEL/$SECTORS_KERNEL/" tmp.bootloader.asm
nasm -f bin tmp.bootloader.asm -o bin/bootloader.bin

dd if=/dev/zero of=Build/os.img bs=512 count=2880
dd if=bin/bootloader.bin of=Build/os.img bs=512 count=1 conv=notrunc
dd if=bin/kernel.bin of=Build/os.img bs=512 seek=1 conv=notrunc # setor 2

rm tmp.bootloader.asm
rm tmp.kernel.asm

qemu-system-i386 -drive format=raw,file=Build/os.img -d in_asm

