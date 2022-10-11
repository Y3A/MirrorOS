section .asm

global insb
global insw

global outsb
global outsw

insb:
push ebp
mov ebp, esp

xor eax, eax
mov edx, [ebp+8]
in al, dx

leave
ret

insw:
push ebp
mov ebp, esp

xor eax, eax
mov edx, [ebp+8]
in ax, dx

leave
ret

outsb:
push ebp
mov ebp, esp

mov edx, [ebp+8]
mov eax, [ebp+12]
out dx, al

leave
ret

outsw:
push ebp
mov ebp, esp

mov edx, [ebp+8]
mov eax, [ebp+12]
out dx, ax

leave
ret