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

## Implementation

The Windows writer now uses `CreateFileA(..., CREATE_NEW)` with a
process-ID and interlocked sequence suffix for exclusive sibling
staging, `WriteFile`, `FlushFileBuffers`, `CloseHandle`, and
`MoveFileExA(MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)`.
Failure cleanup targets only the staging pathname owned by that
operation. The PEVT wire format remains unchanged.

The existing concurrent-writer CTest now exercises two Windows
threads writing distinct snapshots to one destination. On POSIX,
the test continues to use separate processes. Windows CI is
enabled in `.github/workflows/ci.yml`.

## Limitations

- The Windows regression uses two threads in one process, not two
  independent processes. The exclusive-create scheme uses a
  per-process interlocked counter and process ID.
- A successful Windows replacement is not a universal power-loss
  guarantee. Filesystem, hardware and API limitations still apply.
- Concurrent writes are last-successful-replacement-wins. No
  inter-writer coordination or full-machine consistency is implied.
- Windows pathname handling currently uses ANSI APIs; Unicode
  filenames and long-path handling need separate coverage.

## Status

**M4.28 QUALIFIED** against commit `9d4878ab4`:
GitHub Actions run [38331310135](https://github.com/Ploos-AS/PPCVM/actions/runs/38331310135)
completed successfully with Linux build/CTest, Windows build/CTest,
and PowerPC assembly diagnostics. The subsequent documentation-only
commit `87888196f` was still in progress when this status was recorded.

Remaining enhancements: independent Windows process stress test,
Unicode/long path support, and stronger platform-specific durability
documentation and fault injection.
