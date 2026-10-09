OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc)
ENTRY(_start)
SECTIONS
{
  . = 0x500;
  .text.irq : { KEEP(*(.text.irq)) }
  . = 0x1000;
  .text : { KEEP(*(.text.start)) *(.text .text.*) }
  /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
