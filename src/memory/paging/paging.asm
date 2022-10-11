BITS 32

section .asm ; stuff these assembly into the .asm section to not clutter compiler generated code

global paging_load_pagedir
global paging_enable_paging

; https://wiki.osdev.org/Paging#Enabling
paging_load_pagedir:
push ebp
mov ebp, esp
mov eax, [ebp+8] ; page dir base address
mov cr3, eax
leave
ret

paging_enable_paging:
push ebp
mov ebp, esp
mov eax, cr0
or eax, 0x80000001 ; set the Paging and Protection bit, to enable paging
mov cr0, eax
leave
ret