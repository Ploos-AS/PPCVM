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

## Status
Specification created; implementation pending.
