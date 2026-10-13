# M4.28 — Windows snapshot persistence parity

## Goal
Remove the fixed .tmp staging-file collision on Windows and
provide documented replacement and durability behavior for
PEVT snapshot files.

## Proposed approach
Use Windows-native exclusive creation for unique sibling staging
files, write and flush the 16-byte PEVT frame, then replace the
destination using appropriate Windows filesystem APIs.
Avoid assumptions that POSIX rename semantics apply to Windows.

## Acceptance criteria
1. Concurrent Windows writers never share a staging file.
2. Failed writes do not truncate the existing destination.
3. Successful writes publish a complete PEVT frame.
4. Malformed loads remain transactional with respect to IRQ state.
5. Cleanup removes only staging files owned by the writer.
6. Document replacement and power-loss limitations explicitly.
7. Run Windows-specific tests on a Windows CI runner and retain
   Linux/PPC32 guest regression coverage.

## Scope
Device-level file persistence only; not whole-machine snapshots,
migration or atomic multi-device state capture.

## Status
Specification created; implementation pending.
