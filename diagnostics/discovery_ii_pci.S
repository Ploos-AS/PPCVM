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
    /* PCI1 is independent and contains a different device. */
    stw 4,0xc78(3)
    lwz 5,0xc78(3)
    xor 8,5,4
    cmpwi 8,0
    bne failed
    lwz 5,0xc7c(3)
    lis 7,0xcdab
    ori 7,7,0x01ef
    xor 8,5,7
    cmpwi 8,0
    bne failed
    /* PCI0 latch remains unchanged after PCI1 access. */
    lwz 5,0xcf8(3)
    xor 8,5,4
    cmpwi 8,0
    bne failed
    /* Scan absent slot 3 on PCI0: config data must be all ones. */
    lis 9,0x0018
    ori 9,9,0x0080
    stw 9,0xcf8(3)
    lwz 5,0xcfc(3)
    li 7,-1
    xor 8,5,7
    cmpwi 8,0
    bne failed
    /* Scan absent slot 3 on PCI1, independently. */
    stw 9,0xc78(3)
    lwz 5,0xc7c(3)
    xor 8,5,7
    cmpwi 8,0
    bne failed
    /* Restore and rediscover both populated slot 2 devices. */
    stw 4,0xcf8(3)
    lwz 5,0xcfc(3)
    lis 7,0x3412
    ori 7,7,0x7856
    xor 8,5,7
    cmpwi 8,0
    bne failed
    stw 4,0xc78(3)
    lwz 5,0xc7c(3)
    lis 7,0xcdab
    ori 7,7,0x01ef
    xor 8,5,7
    cmpwi 8,0
    bne failed
    /* BAR0 on PCI0: config register 0x10; guest-visible LE word. */
    lis 9,0x1010
    ori 9,9,0x0080
    stw 9,0xcf8(3)
    lwz 5,0xcfc(3)
    lis 7,0x0000
    ori 7,7,0x0090
    xor 8,5,7
    cmpwi 8,0
    bne failed
    /* Probe BAR0 size (4 KiB), then restore original base. */
    li 10,-1
    stw 10,0xcfc(3)
    lwz 5,0xcfc(3)
    lis 7,0x00f0
    ori 7,7,0xffff
    xor 8,5,7
    cmpwi 8,0
    bne restore_failed
    lis 10,0x0000
    ori 10,10,0x0090
    stw 10,0xcfc(3)
    /* Enable PCI Memory Decode: command register at config offset 4. */
    lis 9,0x0410
    ori 9,9,0x0080
    stw 9,0xcf8(3)
    lis 10,0x0200
    stw 10,0xcfc(3)
    lwz 5,0xcfc(3)
    xor 8,5,10
    cmpwi 8,0
    bne failed
    /* BAR0 aperture is supplied in r11; test payload in r12. */
    stw 12,4(11)
    lwz 5,4(11)
    xor 8,5,12
    cmpwi 8,0
    bne failed
    /* Disable memory decoding after the successful BAR access. */
    li 10,0
    stw 10,0xcfc(3)
    lwz 5,0xcfc(3)
    cmpwi 5,0
    bne failed
    li 6,1
    .long 0
restore_failed:
    lis 10,0x0000
    ori 10,10,0x0090
    stw 10,0xcfc(3)
    b failed
    .long 0
failed:
    li 6,0
    .long 0
    .size _start,.-_start
