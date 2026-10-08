/* Guest toggles PCI Memory Decode via synthetic config bridge.
 * r3 = config bridge, r4 = BAR0 base, r5 = payload.
 * r6 = 1 if enabled MMIO readback succeeds. Guest then disables decode.
 * Host test verifies disabled BAR access is rejected. */
    .section .text
    .globl _start
    .type _start,@function
_start:
    lis 7,0x8000
    ori 7,7,0x1004
    stw 7,0(3)
    li 8,2
    stw 8,4(3)
    stw 5,4(4)
    lwz 9,4(4)
    xor 10,9,5
    cmpwi 10,0
    bne failed
    li 6,1
    b disable
failed:
    li 6,0
disable:
    li 8,0
    stw 8,4(3)
    .long 0
    .size _start,.-_start
