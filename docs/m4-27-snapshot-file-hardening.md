# M4.27 — Snapshot file persistence hardening

## Goal
Harden the PEVT device snapshot file writer beyond M4.26's
single-writer, best-effort sibling .tmp implementation.

## Proposed work
- Use unique, exclusive-create staging files in the destination
  directory to avoid collisions and accidental overwrite.
- Ensure error paths close and remove only the staging file owned
  by the current operation.
- On POSIX hosts, sync the staged file and containing directory
  where supported; document platform-specific limitations.
- Specify how replacement works on Windows and POSIX.
- Keep the validated 16-byte PEVT wire format unchanged.

## Acceptance criteria
1. Existing destination survives failed staging and validation.
2. Concurrent writers never share the same temporary file.
3. Test cleanup and failure cases without leaving stale staging files.
4. Test successful replacement and restore on supported CI hosts.
5. Document durability guarantees per platform rather than claiming
   universal crash safety.
6. Preserve all existing PPC32 ELF and PCI event tests.
7. Record passing CI before qualification.

## Non-goals
Whole-machine snapshots, migration and atomic multi-device restore.

## Implemented

On POSIX, the writer now creates a unique staging file in the same
directory using `mkstemp()`, writes the PEVT frame, flushes stdio,
calls `fsync()` on the staged file, closes it, renames it over the
destination, then opens and `fsync()`s the containing directory.
Failures are reported; a directory-sync failure can occur **after**
the new destination has already been published.

`tests/test_pci_event_snapshot_concurrent.c` exercises two processes
writing repeatedly to the same destination and verifies that the final
file is a complete, decodable frame. Existing file corruption,
replacement, IRQ restore and PPC32 ELF regressions remain enabled.

## CI evidence

- `bad394d8d`, run 38167743345: **success** — unique POSIX staging
  and staged-file sync.
- `8fc0cad1e`, run 38168303455: **success** — concurrent writer test.
- `3a7457cea`, run 38214933238: **failure** — directory-path
  terminator regression.
- `86e207078`, run 38215006681: **success** — terminator corrected;
  build and regression suite pass.

## Platform and durability limits

- **POSIX:** unique staging avoids writers sharing the same temporary
  file. Atomic destination replacement depends on the host filesystem
  and OS. Successful file and directory `fsync()` improve durability,
  but cannot guarantee survival across every storage stack or power
  failure. Concurrent writers remain last-rename-wins; no conflict
  detection or coordinated VM state capture is provided.
- **Windows:** still uses the fixed `.tmp` sibling from M4.26 and
  lacks equivalent durable replacement and unique staging. It is
  not concurrency-safe.
- Untrusted directories and adversarial filesystem races require
  additional hardening. Failure after rename may leave a valid new
  snapshot even when the API returns an error.

## Status

**M4.27 COMPLETE for POSIX staging and best-effort durability**, with
passing CI. Cross-platform parity and stronger persistence guarantees
remain future work.
