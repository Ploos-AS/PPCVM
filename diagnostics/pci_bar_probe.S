/* Guest PCI BAR0 sizing and restore using synthetic config bridge.
 * r3 bridge base; r6 = 1 on success, 0 on mismatch.
 * Expected PCI device 0:2.0, BAR0 = 0x90000000, size 4096.
 * HALT word is PPCVM diagnostic-only. */
    .section .text
    .globl _start
    .type _start,@function
_start:
    lis 4,0x8000
    ori 4,4,0x1010
    stw 4,0(3)
    lwz 5,4(3)
    lis 7,0x9000
    xor 8,5,7
    cmpwi 8,0
    bne failed
    li 9,-1
    stw 9,4(3)
    lwz 5,4(3)
    lis 7,0xffff
    ori 7,7,0xf000
    xor 8,5,7
    cmpwi 8,0
    bne restore_failed
    lis 9,0x9000
    stw 9,4(3)
    li 6,1
    .long 0
restore_failed:
    lis 9,0x9000
    stw 9,4(3)
failed:
    li 6,0
    .long 0
    .size _start,.-_start
