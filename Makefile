BINS = ./bin/boot.bin ./bin/kernel.bin
LINKS = ./build/kernel.asm.o ./build/kernel.o ./build/idt/idt.o ./build/memory/memory.o ./build/io/io.asm.o ./build/interrupts.asm.o ./build/memory/heap/heap.o ./build/memory/heap/kheap.o ./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o ./build/string/string.o ./build/drivers/vga.o ./build/drivers/ata.o ./build/fs/ext2fs.o ./build/fs/vfs.o ./build/fs/path.o ./build/gdt/gdt.o ./build/gdt/gdt.asm.o ./build/task/tss.o ./build/task/tss.asm.o
INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: $(BINS)
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin # boot sector goes first, then the kernel which is loaded by the former.
	dd if=/dev/zero bs=512 count=200 >> ./bin/os.bin # pad some bytes for the bootloader to load first, so we don't have to change when our kernel increases in size

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

./build/kernel.o : ./src/kernel.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/kernel.c -c -o ./build/kernel.o 

./build/idt/idt.o : ./src/idt/idt.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/idt/idt.c -c -o ./build/idt/idt.o

./build/memory/memory.o : ./src/memory/memory.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/memory/memory.c -c -o ./build/memory/memory.o

./bin/kernel.bin: $(LINKS)
	i686-elf-ld -relocatable $(LINKS) -o ./build/kernelfull.o # mash object files together, produce output as another object file.
	i686-elf-gcc $(FLAGS) ./build/kernelfull.o -T ./src/linker.ld -ffreestanding -O0 -nostdlib -o ./bin/kernel.bin # compile with our predefined linker settings

./build/io/io.asm.o: ./src/io/io.asm
	nasm -f elf -g ./src/io/io.asm -o ./build/io/io.asm.o

./build/interrupts.asm.o: ./src/idt/interrupts.asm
	nasm -f elf -g ./src/idt/interrupts.asm -o ./build/interrupts.asm.o

./build/memory/heap/heap.o : ./src/memory/heap/heap.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/memory/heap/heap.c -c -o ./build/memory/heap/heap.o

./build/memory/heap/kheap.o : ./src/memory/heap/kheap.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/memory/heap/kheap.c -c -o ./build/memory/heap/kheap.o

./build/memory/paging/paging.o : ./src/memory/paging/paging.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/memory/paging/paging.c -c -o ./build/memory/paging/paging.o

./build/memory/paging/paging.asm.o: ./src/memory/paging/paging.asm
	nasm -f elf -g ./src/memory/paging/paging.asm -o ./build/memory/paging/paging.asm.o

./build/string/string.o: ./src/string/string.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/string/string.c -c -o ./build/string/string.o

./build/drivers/vga.o: ./src/drivers/vga.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/drivers/vga.c -c -o ./build/drivers/vga.o

./build/drivers/ata.o: ./src/drivers/ata.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/drivers/ata.c -c -o ./build/drivers/ata.o

./build/fs/ext2fs.o: ./src/fs/ext2fs.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/fs/ext2fs.c -c -o ./build/fs/ext2fs.o

./build/fs/vfs.o: ./src/fs/vfs.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/fs/vfs.c -c -o ./build/fs/vfs.o

./build/fs/path.o: ./src/fs/path.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/fs/path.c -c -o ./build/fs/path.o

./build/gdt/gdt.o: ./src/gdt/gdt.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/gdt/gdt.c -c -o ./build/gdt/gdt.o

./build/task/tss.o: ./src/task/tss.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/task/tss.c -c -o ./build/task/tss.o

./build/gdt/gdt.asm.o: ./src/gdt/gdt.asm
	nasm -f elf -g ./src/gdt/gdt.asm -o ./build/gdt/gdt.asm.o

./build/task/tss.asm.o: ./src/task/tss.asm
	nasm -f elf -g ./src/task/tss.asm -o ./build/task/tss.asm.o

clean:
	rm -rf $(BINS) ./bin/os.bin
	rm -rf $(LINKS) ./build/kernelfull.o
