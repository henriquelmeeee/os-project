qemu-system-x86_64 -drive file=../Build/disk.img,format=raw -vga std -d cpu_reset,int -m 2048 -serial stdio #-enable-kvm 
#qemu-system-x86_64 -drive file=../Build/disk.img,format=raw -vga std -d cpu_reset,int -m 2048 -serial stdio -icount shift=10,align=on,sleep=on
#qemu-system-x86_64 -s -S -drive file=../Build/disk.img,format=raw -vga std -m 2048 -serial stdio #-enable-kvm
