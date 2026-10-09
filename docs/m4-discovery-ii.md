# M4: Pegasos II Discovery II host bridge

Status: design baseline, **not** a hardware-accurate implementation.

## Scope and separation

Pegasos II uses a Marvell Discovery II system controller (MV64361 family).
The existing `ppcvm_pci_config_bridge` is a **synthetic diagnostic interface**
with an arbitrary eight-byte MMIO aperture. It is not the real Discovery II
PCI configuration mechanism. Likewise, the `discovery_scratch` register in
`src/pegasos2.c` is an explicitly synthetic test fixture, not a Marvell
register. Neither should be presented to guest firmware as authentic hardware.

M4 should introduce a separately named controller model and map it into the
Pegasos II address space only after the relevant hardware addresses and
register semantics have been corroborated against technical documentation
and available open-source firmware or OS drivers.

## Implementation order

1. Collect primary documentation for the MV64361 register map and distinguish
   MV64360/MV64361 variants and board-specific wiring.
2. Define a controller context with explicit reset defaults and bounded,
   big-endian 32-bit MMIO accesses; reject unmapped/unsupported offsets rather
   than silently accepting them.
3. Implement address decode and PCI host interfaces behind the existing
   `ppcvm_bus` and `ppcvm_pci_bus` APIs. Do not reuse the synthetic bridge's
   register offsets as if they were hardware offsets.
4. Add deterministic host-side tests for reset, alignment, access permissions,
   address decode and PCI config transactions.
5. Add cross-assembled guest probes that use only documented register offsets,
   then wire them into GitHub Actions.
6. Expand toward interrupts, DMA, timers and firmware-visible discovery in
   separate testable increments.

## Acceptance criteria

- All mapped registers have documented provenance, offset and reset semantics.
- Unsupported accesses produce explicit, deterministic results.
- Synthetic diagnostics continue to work without being confused with the
  hardware-accurate controller.
- CI runs both host-side register tests and PowerPC guest-side diagnostics.
- A boot claim requires a real firmware/OS boot trace, not only synthetic
  diagnostic ELF programs.

## Current limitations

No authentic Discovery II register block, PCI configuration mapping, interrupt
controller, DMA engine or board-specific initialization is implemented by this
document. Hardware register values and addresses must not be guessed.


## M4.16 implementation checkpoint (experimental, CI-tested)

The original design-baseline limitations above describe the initial state, not
the complete current implementation. Since M4.7, opt-in Discovery II PCI config
windows have been implemented at **candidate** offsets PCI0 address/data
`0xCF8/0xCFC` and PCI1 address/data `0xC78/0xC7C`. CPU-facing
register accesses are byte-swapped to the internal PCI configuration model.
These offsets and endian semantics remain subject to hardware corroboration;
passing a synthetic guest diagnostic does **not** establish MV64361 accuracy.

Current test coverage includes:

- Independent PCI0 and PCI1 configuration address latches and device IDs.
- Populated and absent PCI slots (absent devices read as all ones).
- BAR0 4-KiB size probing and restoration through the controller.
- PCI Command memory-decode enable, guest BAR0 MMIO and subsequent disable;
  accesses after disable are rejected by the mapped aperture.
- Warm and cold Pegasos II reset clearing controller config-address latches
  while preserving the opt-in host PCI wiring.
- Host CTest and cross-assembled PowerPC ELF guest diagnostics in GitHub Actions.

### Remaining hardware and boot gaps

| Area | Present | Missing for meaningful boot progress |
| --- | --- | --- |
| PCI configuration | Candidate controller windows, two independent PCI interfaces | Verify real MV64361 offsets, endian modes, configuration cycles and board topology |
| PCI memory | Synthetic BAR mapping, memory-decode gating, callback aperture | Real controller outbound/inbound windows, remapping, PCI bridges and device models |
| Interrupts | No authenticated Discovery II interrupt controller | Interrupt routing, masks, acknowledgement and device IRQ delivery |
| Timers / DMA | Not represented by the tested PCI diagnostic | Documented timer and DMA register behavior where firmware/OS needs it |
| Firmware | Diagnostic ELF loader and synthetic query fixtures | Real Open Firmware / SmartFirmware interface, device tree and boot handoff |
| CPU / MMU | Partial 32-bit PowerPC interpreter | Required G4 instructions, exceptions, translation, BAT/TLB and privilege semantics |
| Board devices | Synthetic PCI test devices | Pegasos II southbridge, storage, console, network and boot-critical peripherals |

### Next acceptance gates

1. **M4.17 — provenance:** confirm each implemented candidate config register
   against primary MV64361 or relevant Pegasos II driver sources; retain
   experimental opt-in until verified.
2. **M4.18 — controller windows:** model and test documented PCI memory windows,
   address translation and enable/reset behavior without conflating the generic
   PCI BAR aperture with a physical Discovery II window.
3. **M4.19 — interrupt baseline:** establish documented mask, cause and
   acknowledgement semantics, then a deterministic PowerPC guest IRQ test.
4. **M5 — firmware-first boot trace:** define a reproducible, legally
   distributable firmware input, record first instructions and first failure,
   and incrementally resolve CPU/MMU, memory-map and device gaps.

**Release gate:** Do not claim Pegasos II firmware, MorphOS, AmigaOS 4,
AROS PPC, Linux or BSD boot support until a reproducible real firmware or
operating-system trace demonstrates it. Diagnostic ELF success is insufficient.

## M4.18 implementation: explicit CPU-to-PCI translation fixture

`ppcvm_pci_map_translated_window` maps a CPU-side aperture to a separately
configured 64-bit PCI address. It is **disabled by default** and still requires
PCI Command Memory Space Enable and a matching BAR/callback on the PCI bus.
The translation is `pci_base + (cpu_address - cpu_base)`; the mapping rejects
invalid alignment, zero-sized ranges and CPU/PCI address overflow. A CTest
regression uses CPU `0xa0000000` to reach PCI BAR0 `0x90000000`, verifies
decode gating, read/write callbacks and disable behavior.

This is a deliberately generic, opt-in fixture. It does **not** implement
MV64361 outbound window registers, window-enable bitfields, board defaults
or hardware reset behavior. It should not be used as evidence of firmware
boot compatibility.

## M4.19 planning: interrupt architecture gate

Before implementing guest-visible IRQ registers, verify the MV64361
interrupt-cause/mask register layout, per-source routing, CPU interrupt
polarity and Pegasos II board wiring from primary sources. A useful
intermediate test fixture may track a pending IRQ and deliver a PowerPC
external exception when MSR[EE] is set, but such a fixture must be clearly
named synthetic and must not be exposed as a physical Discovery II
interrupt-controller model. Cover pending-vs-masked, acknowledge, reset,
exception SRR0/SRR1 and disabled MSR[EE] in deterministic tests.
