# M4.26 — Durable PCI event snapshot files

## Goal
Add a safe host-side file I/O layer for the portable 16-byte PEVT
device-state format qualified in M4.25.

## Design
Use the existing snapshot save/encode and decode/restore APIs. File
operations must not serialize C structs directly. Reject truncated,
oversized or malformed files. Loading must validate all bytes before
mutating device or IRQ state.

## Acceptance criteria
1. Write a complete 16-byte PEVT frame to a file with checked I/O errors.
2. Read exactly 16 bytes; reject empty, truncated and oversized files.
3. Reject bad magic, version and event masks without changing live state.
4. Restore IRQ ownership transactionally via existing device API.
5. Test file round-trip and negative cases in a temporary test directory.
6. Preserve all previous PPC32 ELF and PCI event regression tests.
7. Document passing CI before qualification.

## Limitations
Device-level file persistence only. No whole-machine freeze,
atomic multi-device snapshot, guest RAM capture or migration.

## Status
Specification created; implementation pending.
