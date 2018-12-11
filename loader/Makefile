all:
	nasm stage1.asm
	make -C stage2
	make -C stage3

clean:
	rm *.o *.elf stage1 || true
	make -C stage2 clean
	make -C stage3 clean
