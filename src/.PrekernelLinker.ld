OUTPUT_FORMAT("binary")
ENTRY(main)
SECTIONS
{
  . = 0x1000;
  .text BLOCK(4K) : ALIGN(4K)
  {
    *(.text)
  }
  .data : { *(.data) }
  .bss  : { *(.bss)  }
}
