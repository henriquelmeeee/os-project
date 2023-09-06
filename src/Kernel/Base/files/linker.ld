ENTRY(_start)

SECTIONS
{
  . = 0x0;
  .header : { LONG(_start); LONG(_data) }

  .data :
  {
    _data = .;
    *(.data)
  }

  .text :
  {
    _start = .;
    *(.text)
  }
}

