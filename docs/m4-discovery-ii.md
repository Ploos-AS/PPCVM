# M4: Pegasos II Discovery II host bridge

Status: design baseline, **not** a hardware-accurate implementation.

## Scope and separation

Pegasos II uses a Marvell Discovery II system controller (MV64361 family).
The existing `ppcvm_pci_config_bridge` is a **synthetic diagnostic interface**
with an arbitrary eight-byte MMIO aperture. It is not the real Discovery II
PCI configuration mechanism. Likewise, the `discovery_scratch` register in
`src/pegasos2.c` is an explicitly synthetic test fixture, not a Marvell
register. Neither should be presented to guest firmware as authentic hardware.

M4 should introduce a separately named controller model and map it into the
Pegasos II address space only after the relevant hardware addresses and
register semantics have been corroborated against technical documentation
and available open-source firmware or OS drivers.

## Implementation order

1. Collect primary documentation for the MV64361 register map and distinguish
   MV64360/MV64361 variants and board-specific wiring.
2. Define a controller context with explicit reset defaults and bounded,
   big-endian 32-bit MMIO accesses; reject unmapped/unsupported offsets rather
   than silently accepting them.
3. Implement address decode and PCI host interfaces behind the existing
   `ppcvm_bus` and `ppcvm_pci_bus` APIs. Do not reuse the synthetic bridge's
   register offsets as if they were hardware offsets.
4. Add deterministic host-side tests for reset, alignment, access permissions,
   address decode and PCI config transactions.
5. Add cross-assembled guest probes that use only documented register offsets,
   then wire them into GitHub Actions.
6. Expand toward interrupts, DMA, timers and firmware-visible discovery in
   separate testable increments.

## Acceptance criteria

- All mapped registers have documented provenance, offset and reset semantics.
- Unsupported accesses produce explicit, deterministic results.
- Synthetic diagnostics continue to work without being confused with the
  hardware-accurate controller.
- CI runs both host-side register tests and PowerPC guest-side diagnostics.
- A boot claim requires a real firmware/OS boot trace, not only synthetic
  diagnostic ELF programs.

## Current limitations

No authentic Discovery II register block, PCI configuration mapping, interrupt
controller, DMA engine or board-specific initialization is implemented by this
document. Hardware register values and addresses must not be guessed.
