BINS = ./bin/boot.bin ./bin/kernel.bin
LINKS = ./build/kernel.asm.o ./build/kernel.o ./build/idt/idt.o ./build/memory/memory.o ./build/io/io.asm.o ./build/interrupts.asm.o ./build/memory/heap/heap.o ./build/memory/heap/kheap.o ./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o ./build/disk/disk.o ./build/fs/path_parser.o ./build/string/string.o
INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: $(BINS)
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=200 >> ./bin/os.bin

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
	i686-elf-ld -relocatable $(LINKS) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) ./build/kernelfull.o -T ./src/linker.ld -ffreestanding -O0 -nostdlib -o ./bin/kernel.bin

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

./build/disk/disk.o: ./src/disk/disk.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/disk/disk.c -c -o ./build/disk/disk.o

./build/fs/path_parser.o: ./src/fs/path_parser.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/fs/path_parser.c -c -o ./build/fs/path_parser.o

./build/string/string.o: ./src/string/string.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) ./src/string/string.c -c -o ./build/string/string.o

clean:
	rm -rf $(BINS) ./bin/os.bin
	rm -rf $(LINKS) ./build/kernelfull.o
