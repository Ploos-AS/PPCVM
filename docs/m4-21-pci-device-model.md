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

## Implemented synthetic BAR0 event endpoint

`ppcvm_pci_event_device` is separate from the older diagnostic
`ppcvm_pci_irq_device`. Both are synthetic fixtures, not identified
Pegasos II hardware. The new endpoint has a 16-byte memory BAR0.

| Offset | Register | Read | Write |
| --- | --- | --- | --- |
| 0x00 | PENDING | Latched event bits | Rejected |
| 0x04 | ENABLE | IRQ-enable mask | Replace enable mask |
| 0x08 | ACK | Zero | Write-one-to-clear PENDING |
| 0x0c | RAISE | Zero | Write-one-to-set PENDING |

Only bits 0–3 are implemented. Writes with any reserved bit set fail;
unmapped offsets and nonzero BAR indices fail. Reset initialization sets
PENDING=0 and ENABLE=0. INTx is asserted iff `(PENDING & ENABLE) != 0`.
A masked event remains pending until acknowledged. The IRQ matrix owns
shared-line aggregation; the endpoint does not clear other owners.

The PCI Command memory-space bit gates MMIO decoding. The CPU bus maps
BAR0 through a diagnostic fixed aperture; this is not a Discovery II
programmable outbound-window model.

## Test coverage and evidence

- `test_pci_event_device`: pending/enable/ACK semantics and rejected accesses.
- `test_pci_event_device_bar`: PCI BAR0 MMIO dispatch and Command memory decode.
- `test_discovery_irq_elf`: PPC32 guest BAR0 reads, guest ENABLE writes,
  guest ACK, shared-source delivery and `rfi` reentry. Its latest
  revision also tests a masked pending event becoming deliverable after
  the guest unmasks it.

CI evidence:
- Commit `a4c30f10d`: success, Actions run 38120887803 (masked event
  preserved and host-side enable transition).
- Commit `7238289a0`: guest-controlled unmask scenario; CI result must
  be checked before marking it qualified.

## Remaining limitations

- Event generation is synthetic; there is no DMA, PCI device-specific
  firmware interface, or physical Pegasos II device identity.
- MMIO callbacks operate on 32-bit words and assume the existing CPU
  bus endian conversion.
- IRQ and register behavior are modeled deterministically, not timed
  against real PCI hardware.
- Actual MV64361 register addresses and interrupt routing require
  hardware documentation or independent validation.
