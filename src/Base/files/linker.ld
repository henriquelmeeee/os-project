SECTIONS
{
    . = 0x100000;

    .text :
    {
        *(main.o(.text)) /* ensure main is first */
        *(.text)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
        . = ALIGN(4);
    }

    /DISCARD/ :
    {
        *(.eh_frame)
    }
}
