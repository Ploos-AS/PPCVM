# M4.20 — Synthetic PCI INTx / Discovery II IRQ qualification

## Scope and limitations

This milestone validates a **synthetic** PCI interrupt path in the PPCVM
Pegasos II diagnostic harness. The Discovery II register offsets, byte ordering,
and IRQ routing remain **candidate mappings**; no real Pegasos II board trace
or MorphOS/AmigaOS 4 boot is claimed. These tests do not establish hardware
compatibility.

## Test matrix

| Scenario | Test | Expected result |
| --- | --- | --- |
| Shared PCI INTx source, two owners | `test_pci_irq_matrix` | Wired-OR remains asserted until both owners release |
| BAR0-backed synthetic IRQ device | `test_pci_irq_device`, `test_pci_irq_device_bar` | MMIO controls device assertion and status |
| Guest reads Discovery II cause | `test_discovery_irq_elf` | PPC32 big-endian guest sees source 0 and source 1 |
| Guest acknowledges both shared owners | `test_discovery_irq_elf` | BAR0 status transitions 1 to 0 for each device |
| Independent source remains asserted | `test_discovery_irq_elf` | Source 1 persists when shared source 0 clears |
| Guest-controlled IRQ mask re-arm | `test_discovery_irq_elf` | CPU0 mask remains enabled across handler return |
| Repeated IRQ delivery | `test_discovery_irq_elf` | Guest handler counter increments for each invocation |
| INTx asserted during handler | `test_discovery_irq_elf` | No nested external exception while MSR[EE] is cleared |
| INTx after acknowledgement, before rfi | `test_discovery_irq_elf` | Pending level is delivered only after MSR[EE] is restored |

## Reproduction

Build with CMake, enable testing, and run `ctest --output-on-failure`.
The `test_discovery_irq_elf` case requires the cross-assembled
`discovery_irq_guest.elf` fixture; refer to the repository's
`powerpc-assembly` GitHub Actions job for the exact toolchain and commands.

## Evidence and exit criteria

As of 2026-10-10, GitHub Actions run
[38050565791](https://github.com/Ploos-AS/PPCVM/actions/runs/38050565791)
passed for commit `2f1653ff6`, covering three guest IRQ cycles.
The later, stricter pre-`rfi` checks in commit `ecad513e1` are **not yet
CI-verified** at the time this document was written.

M4.20 is ready to close only when the latest head's CTest and PPC32
assembly jobs pass and the candidate register map is clearly documented.
Hardware-accurate Discovery II mapping and guest OS boot remain separate
future milestones.

## Follow-up

1. Confirm the latest GitHub Actions run passes on the final test revision.
2. Add precise step-by-step IRQ timing assertions for all handler transitions.
3. Replace synthetic device registers with a documented, realistic PCI device.
4. Compare candidate Discovery II offsets, endian behavior and routing with
   hardware documentation or independently captured traces.
