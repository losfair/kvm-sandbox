all:
	nasm stage1.asm
	nasm stage2.asm
	nasm stage3.asm
	#$(CC) -Tstage2.ld -m32 -O2 -ffreestanding -nostdlib --no-pie -N -o stage2.elf stage2.c
	#objcopy -O binary --set-start 0x100000 stage2.elf stage2

clean:
	rm *.o *.elf stage1 stage2