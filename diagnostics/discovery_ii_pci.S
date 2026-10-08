/* Experimental Discovery II PCI0 config probe.
 * r3 = MMIO base, r6 = success; PCI 0:2.0 = 1234:5678.
 * CPU-facing config registers are little-endian.
 * Diagnostic halt word is PPCVM-only, not hardware behavior. */
    .section .text
    .globl _start
    .type _start,@function
_start:
    lis 4,0x0010
    ori 4,4,0x0080
    stw 4,0xcf8(3)
    lwz 5,0xcf8(3)
    xor 8,5,4
    cmpwi 8,0
    bne failed
    lwz 5,0xcfc(3)
    lis 7,0x3412
    ori 7,7,0x7856
    xor 8,5,7
    cmpwi 8,0
    bne failed
    li 6,1
    .long 0
failed:
    li 6,0
    .long 0
    .size _start,.-_start
