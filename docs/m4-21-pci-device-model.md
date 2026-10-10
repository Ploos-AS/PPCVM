# M4.21 — PCI device model: implementation contract

## Goal

Move from a diagnostic INTx-only BAR0 endpoint to a documented PCI device model, without claiming that the synthetic device corresponds to any physical Pegasos II peripheral.

## Existing baseline

- PCI configuration space exposes vendor/device IDs, command bits and BARs.
- The synthetic BAR0 endpoint uses CONTROL at offset 0 (bit 0 drives INTx) and read-only STATUS at offset 4.
- The IRQ matrix combines multiple owners on a shared Discovery II source.
- The PPC32 ELF fixture acknowledges the device through BAR0 and tests re-delivery after `rfi`.

## M4.21 acceptance criteria

1. Define a register map with explicit read/write/reset semantics and reserved-bit behavior.
2. Add a separate pending-event register; distinguish event generation, IRQ enable, and IRQ acknowledgement.
3. Drive INTx from `(pending & enable) != 0` rather than directly from a control bit.
4. Test that disabled interrupts retain pending status without asserting INTx, then assert on enable.
5. Test write-one-to-clear acknowledgement and shared-source deassertion.
6. Exercise BAR decoding and PCI Command memory-space enable/disable through the CPU bus.
7. Keep existing diagnostic BAR0 and PPC32 ELF tests passing until an explicit fixture migration.
8. Run CI on all supported host compilers and the PPC32 guest fixture.

## Compatibility boundary

This milestone is about internal correctness and deterministic emulation. Actual Pegasos II Discovery II offsets, firmware interfaces, and specific on-board PCI devices must be validated independently before they can be called hardware-compatible.
