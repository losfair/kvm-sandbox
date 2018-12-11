all:
	$(CC) -T stage3.ld -m64 -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall \
		-fomit-frame-pointer -fno-stack-protector -nostartfiles -o stage3.elf \
		stage3.c stage3_helper.s
	objcopy -O binary stage3.elf stage3

clean:
	rm *.o *.elf stage3 || true
