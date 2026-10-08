/* Self-checking PCI diagnostic. r3=BAR0 base, r4=write value.
 * r6=1 on matching readback, r6=0 on mismatch.
 * HALT word is PPCVM-specific, not a PowerPC ISA instruction. */
    .section .text
    .globl _start
    .type _start,@function
_start:
    stw 4,4(3)
    lwz 5,4(3)
    xor 7,5,4
    cmpwi 7,0
    bne failed
    li 6,1
    .long 0
failed:
    li 6,0
    .long 0
    .size _start,.-_start
