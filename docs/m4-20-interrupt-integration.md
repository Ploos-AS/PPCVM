# M4.20 — Pegasos II external interrupt integration

Status: in progress. Synthetic board IRQ delivery and regression coverage pass CI; documented Discovery II IRQ registers and a guest ELF fixture are not yet implemented.

## Scope
Connect the CPU external interrupt acceptance path introduced in M4.19 to a deterministic Pegasos II board-level interrupt source and the Discovery II interrupt controller. Keep the initial test independent of proprietary firmware and guest OS images.

## Acceptance criteria

1. A device raises an interrupt through a documented Discovery II pending/mask path; the CPU receives the external interrupt only when both controller and MSR[EE] permit it.
2. A masked interrupt remains pending without triggering exception entry. Unmasking delivers it exactly once according to the modeled level/edge semantics.
3. Exception entry saves the correct SRR0/SRR1 and transfers to the proper external interrupt vector, including high-vector selection where supported.
4. An exception handler acknowledges the interrupt, executes `rfi`, and resumes the interrupted guest instruction stream without duplicate delivery.
5. Multiple interrupt sources obey explicit arbitration and priority rules; clearing one source does not clear another.
6. A small freely redistributable PPC32 guest ELF exercises the complete path on the Pegasos II profile and emits a deterministic PASS/FAIL signature.
7. CTest registers unit and board-level regression cases; CI builds and executes them on Linux without external ROMs.
8. Negative tests cover disabled EE, controller masking, pending state, repeated assertion, invalid register access, and reset behavior.

## Evidence gates

- Attach CI run and test output to the milestone before marking it complete.
- Document any assumptions about Discovery II behavior against public hardware documentation or traces from owned hardware.
- Do not claim firmware boot, MorphOS, AmigaOS 4, or AROS PPC compatibility from synthetic interrupt tests.

## Suggested implementation sequence

1. Define the interrupt source and controller register contract.
2. Wire Discovery II pending/mask state into the Pegasos II CPU execution loop.
3. Add deterministic board-level interrupt and return-from-interrupt tests.
4. Add the redistributable guest ELF fixture and CI integration.
5. Verify reset, masking, arbitration, and repeated delivery semantics.

## Exit gate

M4.20 is complete only when all acceptance tests pass in CI and the evidence is linked. Firmware startup and OS boot remain separate later gates.

## Verified intermediate evidence (2026-10-09)

- [CI run 37922822632](https://github.com/Ploos-AS/PPCVM/actions/runs/37922822632): successful after the high-vector regression addition.
- [CI run 37922786018](https://github.com/Ploos-AS/PPCVM/actions/runs/37922786018): successful after level-retrigger and independent-source regression additions.
- Synthetic `irq_latch.h` and `irq_bridge.h` remain diagnostic primitives, not a verified MV64361 register map.
- `ppcvm_pegasos2_step_diagnostic_irq` is opt-in. The ordinary Pegasos II step path is unchanged.
- Outstanding gates: validated Discovery II interrupt register offsets and semantics; device-to-controller routing; guest ELF; complete CI acceptance evidence.
