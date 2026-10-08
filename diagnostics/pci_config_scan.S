/* Synthetic PCI configuration scan from guest PowerPC.
 * r3 = bridge base. Probe slots 0..3 on bus 0, function 0.
 * r6 = number of matching devices (expected 1 at slot 2).
 * r9 = matched BDF address; halt is PPCVM diagnostic-only.
 * Only the synthetic bridge layout is assumed. */
    .section .text
    .globl _start
    .type _start,@function
_start:
    li 6,0
    li 9,0
    lis 4,0x8000
    li 10,4
loop:
    stw 4,0(3)
    lwz 5,4(3)
    lis 7,0x5678
    ori 7,7,0x1234
    xor 8,5,7
    cmpwi 8,0
    bne next
    addi 6,6,1
    or 9,4,4
next:
    addi 4,4,0x800
    addi 10,10,-1
    cmpwi 10,0
    bne loop
    .long 0
    .size _start,.-_start
