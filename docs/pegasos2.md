# Pegasos II — first machine skeleton

This is an **experimental machine scaffold**, not yet a hardware-compatible Pegasos II.

- CPU: generic PPC32 interpreter; **not yet a 7447/G4 implementation**.
- RAM: guest physical RAM mapped from address zero.
- Discovery II: provisional MMIO window at `0xf1000000`, currently exposing only
  a **synthetic scratch register at offset zero** for integration testing.
  This address and register must **not** be treated as validated hardware documentation.
- Firmware, PCI, interrupt controller, MMU, real Discovery II registers, graphics,
  and boot devices: **not implemented**.

Next: validate the physical address map from primary hardware documentation and
open firmware behavior before adding real chipset registers. Never distribute
proprietary firmware or commercial OS images in the repository.

Classic Amiga PPC accelerator machines remain a separate future integration track
with AmiVM handling Amiga chipset/68k and PPCVM supplying the PPC core.

## Experimental high-ROM entry

`ppcvm_pegasos2_map_high_rom(machine, bytes, size)` maps caller-owned, read-only
bytes at `0xfff00000` (size 4 KiB to 1 MiB). Keep the bytes alive until the
machine is destroyed. `ppcvm_pegasos2_boot_high_rom(machine, entry)` checks
that the aligned entry instruction is mapped, resets CPU registers, selects
MSR[IP], and starts at that address. A failed entry leaves CPU state intact.
This is a synthetic smoke-test entry, **not** a verified Pegasos II reset
sequence or a replacement for Open Firmware. No ROM is bundled.

## Reset semantics (prototype)

`ppcvm_pegasos2_reset()` resets CPU registers and the synthetic Discovery
scratch register/counters. RAM contents, ROM mappings, bus regions, BAT and
segment configuration are retained. This is a **warm diagnostic reset**, not
yet a faithful Pegasos II power-on reset. Use `ppcvm_pegasos2_init()` for a
fresh machine. A later hardware reset model will define device and MMU reset
semantics explicitly.
