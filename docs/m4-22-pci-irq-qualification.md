# M4.22 — PCI IRQ qualification and fault boundaries

## Goal
Harden the synthetic PCI event device before modeling real Pegasos II
peripherals. Preserve deterministic behavior, explicit failure modes,
and guest-driven interrupt acknowledgement.

## Acceptance criteria

1. Verify two independent event devices sharing one IRQ source; W1C on
   one device must not release the other owner's asserted level.
2. Verify event accumulation while disabled, and later guest-enabled
   delivery without loss.
3. Verify event acknowledgement while another bit remains enabled and
   pending; INTx must remain asserted.
4. Verify invalid BAR offsets, reserved event bits and PCI Command
   memory decode rejection do not mutate event state.
5. Verify IRQ delivery after `rfi` without nested delivery while
   MSR[EE] is cleared.
6. Record CI evidence for each acceptance criterion and preserve the
   existing PPC32 ELF fixture.

## Non-goals
No claim of physical Pegasos II register compatibility, firmware boot,
MorphOS boot, or PCI DMA support.

## Implementation and qualification evidence

- `tests/test_pci_event_shared.c` covers two PCI event devices sharing
  one Discovery II source. Acknowledging one device or one pending bit
  must not release another asserted owner. Disabled pending events are
  retained and delivered when enabled. Invalid writes must not change
  event state.
- `tests/test_pci_event_device_bar.c` additionally covers rejected
  writes to read-only registers, reserved event bits, and MMIO accesses
  while PCI Command memory decode is disabled. State and IRQ level
  must remain unchanged after rejected operations.
- `tests/test_discovery_irq_elf.c` preserves the PPC32 guest
  acknowledgement, masked-pending unmask, and post-`rfi` delivery
  regression checks from M4.21.

CI evidence:
- Commit `448a6ce67`, Actions run 38121183489: **success** for
  shared-source test registration and existing regression suite.
- Commit `dcb6eef05`, Actions run 38121228605: **success** for
  BAR0 fault-boundary assertions and existing regression suite.

## Qualification status

**M4.22 COMPLETE (synthetic PCI IRQ fault boundaries).** Shared-source
and BAR0 fault-boundary tests passed CI. The existing PPC32 ELF fixture
also remains in the passing regression suite. Hardware compatibility
remains outside this synthetic-model milestone.
