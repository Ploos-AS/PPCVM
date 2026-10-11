# M4.24 — PCI event state snapshot and restore

## Goal
Support deterministic save/restore of the synthetic PCI event device
without losing pending events or corrupting shared IRQ ownership.

## Proposed contract
A snapshot contains PENDING and ENABLE, with an explicit version field.
Do not serialize host pointers, callback addresses, or IRQ matrix
internals. Restore validates version and reserved bits before mutation,
then updates the IRQ matrix through the same transactional state path
used by normal register writes.

## Acceptance criteria

1. Save and restore PENDING and ENABLE for a registered event device.
2. Restore recomputes INTx from `(pending & enable) != 0`.
3. Shared-source isolation: restoring one device does not clear another
   owner's asserted IRQ.
4. Reject unsupported versions and reserved bits without state changes.
5. Verify round-trip restore of masked pending events, active IRQ,
   and zero state.
6. Test deterministic guest-visible behavior after restore.
7. Preserve existing PPC32 ELF and PCI event regression tests and
   document passing CI.

## Non-goals
This is not whole-machine snapshotting, migration, DMA serialization,
or a claim of physical Pegasos II hardware compatibility.

## Implemented API

`ppcvm_pci_event_device_save()` copies a versioned, pointer-free
`ppcvm_pci_event_snapshot` containing PENDING and ENABLE. The current
version is 1. This is an in-memory C structure, not a portable
on-disk serialization format: byte order, padding, and file format
are not specified.

`ppcvm_pci_event_device_restore()` rejects null arguments, unsupported
versions, and reserved bits (only bits 0–3 are valid). It updates the
IRQ matrix before committing register state. An unregistered IRQ owner
causes restore failure without changing the device's register values.

## Regression coverage

`tests/test_pci_event_snapshot.c` covers active IRQ round-trip,
masked pending events, shared IRQ ownership, invalid versions,
reserved bits, null arguments, and unregistered owner failure.
The test is registered as `pci_event_snapshot` in CTest.

Existing PPC32 guest tests remain in the overall CI suite. A dedicated
PPC32 guest-visible restore test is **not yet implemented**, so
acceptance criterion 6 is still outstanding.

## CI evidence

- Commit `3565299f3`, Actions run 38147764855: **success**
  for the initial API implementation and preexisting test suite.
- Commit `18fa40d6d`: snapshot regression test registration;
  CI result pending at documentation time.

## Status

**IN PROGRESS** pending CI qualification of the new snapshot tests
and dedicated guest-visible restore coverage.
