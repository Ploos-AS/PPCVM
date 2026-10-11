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

## Status
Specification only. Implementation and CI qualification pending.
