ENTRY(_start)
SECTIONS
{
  . = 0x100;
  .text : { *(.text .text.*) }
  /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
