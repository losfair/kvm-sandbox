all:
	nasm stage1.asm
	nasm stage2.asm
	nasm stage3.asm

	# stage 1b (long mode init)
	$(CC) -T stage2b.ld -m32 -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall \
		-fomit-frame-pointer -fno-stack-protector -nostartfiles -nostdlib -o stage2b.elf \
		stage2b.c
	objcopy -O binary stage2b.elf stage2b
	cat stage2b >> stage2
	dd if=/dev/null of=stage2 bs=1 seek=2048 count=0

	$(CC) -T stage2c.ld -m64 -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall \
		-mno-sse -mno-sse2 -mno-mmx -mno-80387 -mcmodel=large \
		-fomit-frame-pointer -fno-stack-protector -nostartfiles -nostdlib -o stage2c.elf \
		stage2c.c stage2c_helper.s
	objcopy -O binary stage2c.elf stage2c
	cat stage2c >> stage2

	$(CC) -T x_stage4.ld -m32 -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall \
		-fno-omit-frame-pointer -fno-stack-protector -nostartfiles -o x_stage4.elf \
		x_stage4.c
	objcopy -O binary x_stage4.elf x_stage4

clean:
	rm *.o *.elf stage1 stage2
