ENTRY(_start)
SECTIONS
{
  . = 0x80000;
  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : { *(.bss*) }
  . = ALIGN(16);
  _end = .;
  _stack_top = 0x200000;
}
