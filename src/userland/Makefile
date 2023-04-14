
PREFIX = ../..
BINPREFIX = $(PREFIX)/bin/userland
LINKPREFIX = $(PREFIX)/build/userland

BINS =     $(BINPREFIX)/idle/idle.bin

LINKS =    

INCLUDES = -I.

FLAGS =    -g -ffreestanding -falign-jumps -falign-functions -falign-labels
FLAGS +=   -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions
FLAGS +=   -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp
FLAGS +=   -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

$(BINPREFIX)/idle/idle.bin: ./idle/idle.asm
	nasm -f bin ./idle/idle.asm -o $(BINPREFIX)/idle/idle.bin

all: $(BINS)
	# mount filesystem and copy files
	sudo mkdir -p /mnt/MirrorOS
	sudo mount -t ext2 $(PREFIX)/bin/ext2fs /mnt/MirrorOS
	mkdir -p /mnt/MirrorOS/sys/
	cp $(BINPREFIX)/idle/idle.bin /mnt/MirrorOS/sys/idle.bin
	sudo umount /mnt/MirrorOS

clean:
	rm -rf $(BINS)
	rm -rf $(LINKS)