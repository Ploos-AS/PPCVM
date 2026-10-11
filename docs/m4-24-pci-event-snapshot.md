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

## Status
Specification created; implementation and qualification pending.
