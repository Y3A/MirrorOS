ENTRY(_start)
OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0x400000; /* User code starts at 0x400000 */
    
    .text ALIGN(4096) :
    {
        *(.text) /* Only create .text in output if it exists in input */
    }
 
    .rodata ALIGN(4096) :
    {
        *(.rodata)
    }

    .data ALIGN(4096) :
    {
        *(.data)
    }

    .bss ALIGN(4096) :
    {
        *(COMMON) /* Place all uninitialised data here */
        *(.bss)
    }
    
    .asm ALIGN(4096) :
    {
        *(.asm)
    }
}