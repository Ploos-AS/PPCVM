/* Freely redistributable PPC32 big-endian bare-metal diagnostic guest.
 * Link with discovery_irq_guest.ld; CPU starts at _start (0x1000).
 * Host must map the candidate Discovery II aperture at 0x20000 and
 * assert IRQ source 0 after _start enables its controller mask.
 * This is not a hardware-accurate Pegasos II firmware image. */
    .section .text.start,"ax",@progbits
    .globl _start
_start:
    lis 3,2                /* r3 = 0x20000 controller base */
    lis 4,0x100            /* bus-visible 0x01000000 maps to mask bit 0 */
    stw 4,0x14(3)         /* enable CPU0 IRQ source 0 (candidate) */
1:  b 1b                   /* wait for external exception */

    .section .text.irq,"ax",@progbits
    .globl irq_handler
irq_handler:
    lwz 7,0x04(3)         /* read candidate cause register */
    andis. 8,7,0x0100    /* test bus-visible source 0 bit */
    beq 2f
    lis 10,3               /* diagnostic PCI BAR0 = 0x30000 */
    lwz 11,4(10)           /* read device status before acknowledgement */
    li 4,0
    stw 4,0(10)           /* clear device INTx through BAR0 */
    lwz 12,4(10)           /* read status after acknowledgement */
    lis 13,3               /* second diagnostic PCI BAR0 = 0x31000 */
    ori 13,13,0x1000
    lwz 14,4(13)           /* second owner status before acknowledgement */
    stw 4,0(13)            /* release second owner of shared IRQ source */
    lwz 15,4(13)           /* second owner status after acknowledgement */
    stw 4,0x14(3)         /* mask source after device acknowledgement */
    li 9,1                /* source 0 handled marker */
    b 3f
2:  li 9,0                /* no enabled source 0 in cause */
3:
    lis 6,0x5041
    ori 6,6,0x5353       /* PASS marker in r6 */
    rfi
