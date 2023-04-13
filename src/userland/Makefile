all:
	nasm -f elf -g ./first_prog.asm -o ../../build/userland/first_prog.asm.o
	i686-elf-gcc -g -T ./linker.ld -o ../../build/userland/first_prog.bin -ffreestanding -O0 -nostdlib -fpic -g ../../build/userland/first_prog.asm.o

clean:
	rm -rf ../../build/userland/first_prog.asm.o
	rm -rf ../../build/userland/first_prog.bin