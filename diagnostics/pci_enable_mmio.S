/* Guest enables PCI Memory Decode and performs BAR0 MMIO.
 * r3 = synthetic config bridge base, r4 = BAR0 base,
 * r5 = write value, r6 = 1 on matching readback else 0.
 * The zero word is PPCVM diagnostic HALT, not real PPC ISA. */
    .section .text
    .globl _start
    .type _start,@function
_start:
    lis 7,0x8000
    ori 7,7,0x1004
    stw 7,0(3)
    li 8,2
    stw 8,4(3)
    lwz 9,4(3)
    xor 10,9,8
    cmpwi 10,0
    bne failed
    stw 5,4(4)
    lwz 9,4(4)
    xor 10,9,5
    cmpwi 10,0
    bne failed
    li 6,1
    .long 0
failed:
    li 6,0
    .long 0
    .size _start,.-_start
