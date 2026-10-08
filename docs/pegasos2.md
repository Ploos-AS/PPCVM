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
