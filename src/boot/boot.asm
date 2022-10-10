ORG 0x7c00 ; we expect the BIOS(Implemented by QEMU) to load our bootloader(first 512 bytes) at 0x7c00
BITS 16 ; 16-Bit real mode addressing

; segment selectors once we switch to protected mode
; https://stackoverflow.com/questions/23978486/far-jump-in-gdt-in-bootloader

GDT_CS equ 1000b ; index 1 in GDT, 0 for GDT and 00 for privileged
GDT_DATA equ 10000b ; index 2 in GDT, 0 for GDT and 00 for privileged

; jump over BIOS parameter block, which is obviously not code
jmp short set_cs
nop ; required by https://wiki.osdev.org/FAT#BPB_.28BIOS_Parameter_Block.29

; BIOS parameter block

; FAT16 header
db "MIRROROS"    ; OEMIdentifier, garbage
dw 512           ; BytesPerSector
db 128           ; SectorsPerCluster
dw 200           ; 200 reserved sectors for kernel
db 2             ; number of FATs, 1 orig and 1 backup
dw 64            ; number of directory entries
dw 0             ; total number of sectors ( unused )
db 0xf8          ; MediaDescriptor
dw 256           ; number of sectors per FAT
dw 01            ; number of sectors per track ( unused )
dw 01            ; number of heads ( unused )
dd 0             ; number of hidden sectors
dd 1000000       ; total number of sectors ( used )

db 0x80          ; drive number, 0x80 for hard disks
db 0             ; reserved
db 0x29          ; extended boot signature
dd 0xcafe        ; volume ID (serial number), garbage?
db "MIRROROS   " ; volume label, garbage, padded to 11 bytes with space(0x20)
db "FAT16   "    ; file system type, just for display, garbage, padded to 8 bytes with space(0x20)

; End of BPB

set_cs:
jmp 0:start ; set CS to 0 in case it isn't already, just for safety

start:
cli ; get rid of BIOS interrupts for safety (and we don't need them anymore)
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00 ; clear segment registers and set stack to 0x7c00, grows downwards so it's fine

; load GDT
lgdt[gdt_descriptor]
mov eax, cr0
or al, 1 ; set Protection Enable bit in cr0, enter protected mode
mov cr0, eax
jmp GDT_CS:load_32 ; set CS to correct segment selector

; GDT Segment Descriptors(a bunch of data)
; https://wiki.osdev.org/Global_Descriptor_Table#Segment_Descriptor
; we just overlap all 4gb of memory for both code and data, essentially giving the finger to segmentation model
; since we will use paging instead!

gdt_start:
gdt_null:
dq 0

; code segment descriptor
gdt_code:
dw 0xffff ; first 16 bits of Limit
dw 0 ; first 16 bits of Base
db 0 ; next 8 bits of Base
db 0x9a ; access bytes: Present, Code/Data, Executable, Read access allowed
db 11001111b ; Flags and last 4 bits of Limit. Flags: Byte granularity, 32-Bit segement, Non 64-Bit
db 0; last 8 bits of Base

; data segment desc
gdt_data:
dw 0xffff ; first 16 bits of Limit
dw 0 ; first 16 bits of Base
db 0 ; next 8 bits of Base
db 0x92 ; access bytes: access bytes: Present, Code/Data, Write access allowed
db 11001111b ; Flags and last 4 bits of Limit. Flags: Byte granularity, 32-Bit segement, Non 64-Bit
db 0; last 8 bits of Base
;

gdt_end:

gdt_descriptor:
; first two byte size, next four byte start address of segment descriptor
; https://wiki.osdev.org/Global_Descriptor_Table#GDTR
; the lgdt instruction sees this and loads the GDT properly
dw gdt_end-gdt_start-1
dd gdt_start

; 32-Bit ASM!
BITS 32

; First things, use LBA to read kernel into memory
; https://wiki.osdev.org/ATA_read/write_sectors#Read_in_LBA_mode

;=============================================================================
; ATA read sectors (LBA mode) 
;
; @param EAX Logical Block Address of sector
; @param CL  Number of sectors to read
; @param RDI The address of buffer to put data obtained from disk
;
; @return None
;=============================================================================

load_32:
mov eax, 1 ; start from sector 1 since 0 is boot sector
mov ecx, 200
mov edi, 0x100000
call ata_lba_read
jmp GDT_CS:0x100000 ; jmp to kernel code loaded at 0x100000 after lba read is done

; actual implementation of lba read driver
ata_lba_read:
mov ebx, eax ; backup lba

; send number of sectors to read
mov eax, ecx
mov dx, 0x1f2
out dx, al

; send lowest 8 bits of lba to the disk controller
mov eax, ebx ; restore lba
mov dx, 0x1f3
out dx, al

; send next 8 bits of lba
mov eax, ebx ; restore lba
mov dx, 0x1f4
shr eax, 8
out dx, al

; send next 8 bits of lba
mov eax, ebx ; restore lba
mov dx, 0x1f5
shr eax, 16
out dx, al

; send highest 4 bits of lba and drive option
shr eax, 24
or eax, 0xe0 ; select master drive
mov dx, 0x1f6 ; port to send high 8 bits to
out dx, al

; command port
mov edx, 0x1f7
mov al, 0x20 ; choice of reading with retry
out dx, al

; read all the sectors

next_sector:
push ecx ; save ecx on stack

; check if need to read
do_check:
mov dx, 0x1f7
in al, dx
test al, 8 ; mask to check if disk is still busy
jz do_check ; loop around and check again if it is

; if we are ready to read
mov ecx, 256 ; 256 words==1 sector
mov dx, 0x1f0 ; data port
rep insw ; read a word from port 0x1f0 into ES:DI

pop ecx ; restore ecx
loop next_sector

ret ; finally, done reading

; boot signature for BIOS to boot us :) 
times 510-($ - $$) db 0
dw 0xAA55