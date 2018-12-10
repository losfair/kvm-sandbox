all:
	nasm stage1.asm
	nasm stage2.asm
	nasm stage3.asm
	$(CC) -T x.ld -m32 -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall \
		-fno-omit-frame-pointer -fno-stack-protector -nostartfiles -o x_stage4.elf \
		x_stage4.c
	objcopy -O binary x_stage4.elf x_stage4

clean:
	rm *.o *.elf stage1 stage2
