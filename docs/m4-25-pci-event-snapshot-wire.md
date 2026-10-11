# M4.25 — Portable PCI event snapshot serialization

## Goal
Introduce a portable, explicitly versioned byte representation of the
synthetic PCI event device's snapshot. Keep the in-memory API from M4.24
and add encode/decode routines that do not depend on host struct layout.

## Proposed wire format (16 bytes)
- Bytes 0–3: ASCII magic `PEVT`
- Bytes 4–7: format version, unsigned 32-bit big-endian (1)
- Bytes 8–11: PENDING, unsigned 32-bit big-endian
- Bytes 12–15: ENABLE, unsigned 32-bit big-endian

Only event bits 0–3 are valid. No pointers, host-endian integers,
compiler padding or matrix state may be serialized.

## Acceptance criteria
1. Encode a valid snapshot into exactly 16 deterministic bytes.
2. Decode only correct magic, version and event-bit masks.
3. Reject short buffers and malformed input without partial output.
4. Round-trip snapshots on supported host platforms.
5. Restore decoded snapshots through the existing transactional API.
6. Verify IRQ behavior after decode and restore, including shared
   ownership and masked pending events.
7. Keep PPC32 ELF and all prior PCI event regression tests passing.
8. Record successful CI evidence before marking complete.

## Scope boundary
This is a device-state interchange format, not a whole-VM snapshot,
guest memory dump, PCI DMA migration or hardware compatibility claim.

## Implementation

The public codec API is declared in `include/ppcvm/pci_event_device.h`
and implemented in `src/pci_event_device.c`:

- `ppcvm_pci_event_snapshot_encode()` writes a deterministic 16-byte
  frame after validating version and event-bit masks.
- `ppcvm_pci_event_snapshot_decode()` requires exactly 16 bytes and
  validates magic, version and event-bit masks before committing output.
- The codec is independent of C struct padding and host byte order.

`tests/test_pci_event_snapshot_wire.c` verifies exact wire bytes,
round-trip, malformed input, short and oversized decode buffers,
and rejection without partial output mutation.

`tests/test_pci_event_snapshot.c` now exercises the complete device
save → encode → decode → restore chain and checks restored IRQ state.
The existing PPC32 ELF regression also passes.

## CI qualification

- `e58dc268d` — Actions run 38160573287: **success** (codec implementation).
- `cc1e12c65` — Actions run 38161414128: **success** (codec test in CTest).
- `520977ede` — Actions run 38161448272: **success** (end-to-end
  snapshot serialization and IRQ restore, existing suite).

## Status

**M4.25 COMPLETE.** Portable device-level snapshot wire format and
integration tests are qualified by CI. Whole-machine snapshotting,
durable storage I/O, migration and physical Pegasos II hardware
compatibility remain outside this milestone.
