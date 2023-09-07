OUTPUT_FORMAT("binary")
ENTRY(kentrypoint)
SECTIONS
{
  . = 10485760;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss  : { *(.bss)  }
}

