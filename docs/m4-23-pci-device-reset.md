# M4.23 — PCI event device reset and lifecycle

## Goal
Add an explicit, deterministic lifecycle for the synthetic PCI event
device. Reset must clear pending events, interrupt enables, and the
device's contribution to its shared IRQ source without disturbing
another device's asserted interrupt.

## Acceptance criteria

1. Provide an explicit reset API for an initialized event device.
2. Reset clears PENDING and ENABLE and deasserts only that device's INTx.
3. Two devices sharing an IRQ source: resetting one preserves the
   other owner's interrupt.
4. Repeated reset is idempotent.
5. Invalid or unregistered matrix ownership must report failure
   without corrupting device state.
6. Test reconfiguration and event delivery after reset.
7. Keep PPC32 guest ELF tests passing and record CI evidence.

## Scope boundary
This is a synthetic device lifecycle, not a physical PCI reset,
Discovery II hotplug, PCI power management, or guest OS boot.

## Implementation and test evidence

- `ppcvm_pci_event_device_reset()` uses the existing IRQ matrix
  update path and only commits PENDING=0 and ENABLE=0 after successful
  IRQ deassertion.
- `tests/test_pci_event_reset.c` verifies shared-source isolation,
  idempotent reset, cleared register reads, reuse after reset, and
  null-argument rejection.
- A follow-up test verifies that an unregistered IRQ owner causes
  reset failure without changing pending or enable state.

CI evidence:
- Commit `f817607dd`, Actions run 38147394109: **success** for the
  initial reset test and existing regression suite.
- Commit `936ebe3fc`, Actions run 38147472830: **success** for
  unregistered-owner failure assertion.
- Commit `de7f8eb6c`, Actions run 38147486112: **success** for
  the full regression suite after documentation update.

## Status

**M4.23 COMPLETE (synthetic PCI event reset lifecycle).** All reset
acceptance tests pass CI.
No claim of hardware reset semantics or physical Pegasos II
compatibility is made.
