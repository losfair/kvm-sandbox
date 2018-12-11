all:
	nasm stage1.asm
	make -C stage2

clean:
	rm *.o *.elf stage1 || true
	make -C stage2 clean
