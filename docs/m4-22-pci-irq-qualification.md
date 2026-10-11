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

## Initial status
Specification only. Implementation and CI qualification pending.
