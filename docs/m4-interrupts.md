# M4.19 — External interrupt implementation gate

Status: design baseline only. No MV64361 IRQ registers are modeled.

## Separation of responsibilities

- Peripheral asserts or deasserts a level-triggered IRQ source.
- Discovery II controller masks/routes sources using **verified** MV64361
  cause/mask registers (not yet implemented).
- CPU accepts an external interrupt only with MSR[EE] enabled, saves
  interrupted PC in SRR0 and previous MSR in SRR1, and enters vector
  0x500 (high prefix when MSR[IP] is set).
- Guest acknowledges the source; CPU exception entry must not implicitly
  clear a level-triggered peripheral cause.

## Required regression cases

1. Pending IRQ with MSR[EE] clear does not enter the vector.
2. Pending IRQ with MSR[EE] set enters vector 0x500.
3. SRR0/SRR1 and low/high exception prefix are correct.
4. A persistent level source reasserts after rfi until acknowledged.
5. Two independent sources do not lose state when one is cleared.
6. Controller reset semantics match verified MV64361 reset defaults.
7. Unsupported controller register offsets fail closed.

## Evidence and release gates

Do not invent MV64361 cause, mask, routing, acknowledgement or reset
register behavior. Obtain MV64361-specific primary documentation or
working Pegasos II firmware/driver traces before exposing guest-visible
registers. The CPU external-interrupt exception can be implemented and
tested separately without claiming controller accuracy.

No firmware or guest OS boot support is established by synthetic ELF
diagnostics.
