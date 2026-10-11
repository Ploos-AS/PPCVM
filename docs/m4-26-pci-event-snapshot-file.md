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

## Implementation and tests

`ppcvm_pci_event_device_save_file()` encodes a 16-byte PEVT frame
and writes it to a sibling `.tmp` path, then attempts to rename it
over the destination. A failed write, close or rename reports an error.
`ppcvm_pci_event_device_load_file()` reads exactly one frame and
rejects truncated, oversized or malformed data before restoring
the device's IRQ state.

`tests/test_pci_event_snapshot_file.c` covers successful file
round-trip, exact frame length, corrupt inputs, unchanged IRQ state
after failed load, replacement of a previously published snapshot,
and failure to stage a write.

## CI qualification

- `9c858e141`, Actions run 38161556259: **success** — file I/O API.
- `5c0c29d1f`, Actions run 38167238244: **success** — file regression.
- `8ba8231dd`, Actions run 38167357860: **success** — staged writes.
- `880f1a074`, Actions run 38167403545: **success** — replacement
  and staging-failure regression, including prior tests.

## Remaining limitations

The staging path is a fixed `.tmp` sibling and is **not safe for
concurrent writers**. This is not guaranteed crash-durable: there
is no file or directory fsync. Rename replacement behavior may
vary by host OS. Failed writes may leave a partial temporary file,
which the implementation attempts to remove. The file API is not
suitable for untrusted multi-user directories without additional
hardening.

## Status

**M4.26 COMPLETE for single-writer, best-effort device-level file
persistence**, qualified by CI. Crash durability, concurrency-safe
staging and whole-machine snapshots remain future work.
