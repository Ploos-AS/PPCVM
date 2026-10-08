/* Standalone PPCVM PCI diagnostic: no libc, firmware or OS ABI.
 * Host sets r3=PCI BAR0 base, r4=value, and enables PCI memory decode.
 * Exit is the PPCVM-only diagnostic zero-word marker. */
    .section .text
    .globl _start
    .type _start,@function
_start:
    stw 4,4(3)
    lwz 5,4(3)
    ori 6,5,0
    .long 0x00000000
    .size _start,.-_start
