
section .text

global _start

_start:

; this is our idle process that the scheduler runs when no other processes are available
; also serves as a sentinel node

test:
    jmp test