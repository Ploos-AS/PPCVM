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
