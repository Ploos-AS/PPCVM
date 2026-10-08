/* Synthetic PCI config bridge guest probe.
 * r3 = bridge base, r6 = 1 on success, 0 on failure.
 * PCI device 0:2.0 has vendor 1234, device 5678.
 * Halt word is PPCVM diagnostic-only. */
    .section .text
    .globl _start
    .type _start,@function
_start:
    lis 4,0x8000
    ori 4,4,0x1000
    stw 4,0(3)
    lwz 5,4(3)
    lis 7,0x5678
    ori 7,7,0x1234
    xor 8,5,7
    cmpwi 8,0
    bne failed
    li 6,1
    .long 0
failed:
    li 6,0
    .long 0
    .size _start,.-_start
