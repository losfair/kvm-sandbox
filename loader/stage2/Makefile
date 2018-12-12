all:
	nasm stage2.asm

	$(CC) -T stage2b.ld -m32 -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall \
		-fomit-frame-pointer -fno-stack-protector -nostartfiles -nostdlib -o stage2b.elf \
		stage2b.c
	objcopy -O binary stage2b.elf stage2b
	cat stage2b >> stage2
	dd if=/dev/null of=stage2 bs=1 seek=2048 count=0

	$(CC) -T stage2c.ld -m64 -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall \
		-mno-sse -mno-sse2 -mno-mmx -mno-80387 -mcmodel=large \
		-fomit-frame-pointer -fno-stack-protector -nostartfiles -nostdlib -o stage2c.elf \
		-DLONG_MODE_64 \
		stage2c.c stage2c_helper.s page.c proc.c
	objcopy -O binary stage2c.elf stage2c
	cat stage2c >> stage2

	rm stage2b stage2c

clean:
	rm *.o *.elf stage2 || true
