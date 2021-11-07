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

pop ebp
ret

insw:
push ebp
mov ebp, esp

xor eax, eax
mov edx, [ebp+8]
in ax, dx

pop ebp
ret

outsb:
push ebp
mov ebp, esp

mov edx, [ebp+8]
mov eax, [ebp+12]
out dx, al

pop ebp
ret

outsw:
push ebp
mov ebp, esp

mov edx, [ebp+8]
mov eax, [ebp+12]
out dx, ax

pop ebp
ret