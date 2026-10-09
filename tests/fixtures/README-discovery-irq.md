# PPC32 Discovery II IRQ guest fixture

This is a bare-metal **PowerPC32 big-endian ELF** test source, not a MorphOS, AmigaOS 4, AROS, or Open Firmware boot image.

Build with a cross toolchain that provides `powerpc-linux-gnu-gcc` and `powerpc-linux-gnu-readelf`:

```sh
powerpc-linux-gnu-gcc -m32 -mbig-endian -nostdlib -nostartfiles -nodefaultlibs \
  -Wl,-T,tests/fixtures/discovery_irq_guest.ld \
  -o discovery_irq_guest.elf tests/fixtures/discovery_irq_guest.S
powerpc-linux-gnu-readelf -h -l -s discovery_irq_guest.elf
```

Expected entry is `0x1000`, with the interrupt handler located at `0x500`. The guest sets candidate CPU0 IRQ mask bit zero, spins until the host asserts source zero, then masks that source and sets register `r6` to `0x50415353` (`PASS`) before `rfi`.

**Important limitations:** The existing `ppcvm_pegasos2_step_discovery_irq` is opt-in; a harness must arrange the initial MSR[EE], map the controller at `0x20000`, opt into candidate registers, load the ELF and assert the source after the guest writes the mask. The source stays asserted after the handler masks it; this is not a device acknowledgement. This fixture is currently *source-only*: neither an ELF artifact nor automated ELF execution is claimed. Linker/toolchain compatibility and section layout must be checked in CI before considering the acceptance gate complete.
