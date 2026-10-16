# M4.29 — Snapshot file fault and path hardening

## Goal
Verify failure paths and publication behavior for device-level PEVT
snapshots on POSIX and Windows before extending snapshot scope.

## Work packages
1. Test a failed destination/staging path without changing the
   in-memory device state.
2. Test that failed writes preserve an existing valid destination
   where failure happens before publication.
3. Assert that concurrent writers leave no owned staging files.
4. Exercise Windows writers in separate processes, not just threads.
5. Audit Windows UTF-8/Unicode paths and replace ANSI APIs with
   wide-character variants where appropriate.
6. Distinguish failure before and after publication in API docs.
7. Retain Linux, Windows and PowerPC guest CI coverage.

## Acceptance criteria
- No truncated or malformed published PEVT frame in tested races.
- A failed load does not mutate device or IRQ state.
- A failed pre-publication save does not replace the old snapshot.
- Temporary-file cleanup is verified, not merely assumed.
- All supported CI platforms pass before qualification.

## Non-goals
Whole-machine checkpoints, multi-device transaction boundaries,
and guaranteed durability across all filesystems and hardware.

## Baseline
M4.28 qualified by GitHub Actions run 38331310135;
documentation qualification commit cb65a1668 passed run 38331407434.

## Status
Implementation in progress. CI run 38966746005 passed Linux, Windows
and PowerPC on commit 18013c80a. Windows concurrent writers are
currently limited to 8 saves each for diagnosis: the 32-save variant
timed out at 90 seconds in run 38966268091. The successful 8-save
run is a baseline, not proof that higher-load concurrency is fixed.

Covered: invalid/missing input, staging path failure, snapshot
integrity, temporary-file cleanup, concurrent POSIX processes,
concurrent Windows threads and independent Windows processes.

Open: diagnose Windows contention at higher iteration counts;
audit and implement Unicode-safe Windows paths; distinguish
pre-publication from post-publication failures in API documentation.
Do not mark M4.29 qualified until these are resolved and CI passes.
