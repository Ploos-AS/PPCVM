# M4.3: Discovery II register provenance (research checkpoint)

This document records register **offsets** observed in historical Linux
`include/linux/mv643xx.h`, which names the family as MV64340. The offsets
are corroborated across multiple kernel trees but have **not yet been
independently validated for the Pegasos II MV64361 variant**. They are
candidates for implementation, not proof of board-level wiring or reset values.

Source: https://android.googlesource.com/kernel/common/+/234d53d2bb60a9436a6d4d55bef1712613b79014/include/linux/mv643xx.h

| Candidate offset | Historical kernel macro | Purpose |
| --- | --- | --- |
| 0x068 | MV64340_INTERNAL_SPACE_BASE_ADDR | Internal register-space base |
| 0x278 | MV64340_BASE_ADDR_ENABLE | Decode window enables |
| 0xcf8 | MV64340_PCI_0_CONFIG_ADDR | PCI interface 0 config address |
| 0xcfc | MV64340_PCI_0_CONFIG_DATA_VIRTUAL_REG | PCI interface 0 config data |
| 0xc78 | MV64340_PCI_1_CONFIG_ADDR | PCI interface 1 config address |
| 0xc7c | MV64340_PCI_1_CONFIG_DATA_VIRTUAL_REG | PCI interface 1 config data |

The older Linux PowerPC controller implementation provides useful context
for hostbridge configuration and should be cross-checked:
https://android.googlesource.com/kernel/msm/+/1da177e4c3f41524e886b7f1b8a0c1fc7321cac2/arch/ppc/syslib/mv64x60.c

## Validation required before enabling guest-visible registers

- Obtain MV64361-specific documentation and establish whether the above
  MV64340 offsets and access semantics match this exact silicon.
- Confirm CPU-side register endianness and PCI configuration address
  encoding from primary documentation or traced working software.
- Determine the Pegasos II board's register base, reset configuration,
  PCI bus wiring and address windows. Do not adopt example board addresses.
- Specify per-register writable bits, reset values and unsupported cycles.
- Add explicit test cases for missing BDF, disabled configuration cycle,
  BAR probing and reset once behavior is verified.

The current M4 controller intentionally rejects all offsets. This
checkpoint does not change its guest-visible behavior.

## M4.5: Pegasos II-specific cross-check and endianness hazard

QEMU's Pegasos II machine documentation explicitly identifies the
**MV64361 Discovery II** northbridge and **VT8231** southbridge:
https://qemu.googlesource.com/qemu/+/c5b4afd4d56e9c2251e6674d8c9ae530a923ecb9/docs/system/ppc/amigang.rst

The Linux device-tree binding documents the MV64360 PCI host-bridge
configuration aperture as `reg = <0xcf8 0x8>`:
https://android.googlesource.com/kernel/msm/+/android-wear-5.1.1_r0.6/Documentation/devicetree/bindings/marvell.txt

A Pegasos II-specific community code example selects the PCI configuration
apertures at `MV64361_BASE + 0xc78` for the bus with memory window
`0x80000000` and `MV64361_BASE + 0xcf8` for the bus with memory
window `0xc0000000`. It uses **little-endian 32-bit** accesses for
the configuration address and data registers:
https://www.amigans.net/modules/newbb/viewtopic.php?post_id=147837

**Implementation blocker:** `ppcvm_bus_read32be` and
`ppcvm_bus_write32be` expose big-endian word semantics for 32-bit MMIO.
Directly attaching a PCI configuration bridge to these callbacks would
risk incorrectly modelling the little-endian CPU-facing register aperture.
Before enabling these registers, introduce an explicitly tested byte-order
adapter or a distinct little-endian MMIO API. Test byte-level and word-level
guest behavior, not only host calls.

These sources increase confidence in the offsets and PCI interface mapping,
but do **not** establish all MV64361 reset defaults, register writable masks,
or exact Pegasos II internal-register base. Do not guess these values.


## M4.17: evidence ledger and implementation status

This is a **source-based provenance review**, not a claim that physical
MV64361 register behavior has been measured. The references above are
historical source links; no board trace or MV64361 silicon manual has yet
been supplied to this project.

| Item | Evidence available | PPCVM behavior | Confidence / blocker |
| --- | --- | --- | --- |
| PCI0 address/data 0xCF8/0xCFC | Historical MV64340 header, Pegasos II community example | Opt-in address latch and PCI config transactions | Medium for offset; MV64361-specific register masks/reset unverified |
| PCI1 address/data 0xC78/0xC7C | Historical MV64340 header, Pegasos II community example | Separate opt-in PCI interface | Medium for offset; physical bus wiring requires confirmation |
| Little-endian CPU-visible PCI config | Pegasos II community example | Explicit swap32 for 32-bit guest accesses | Medium; byte/halfword semantics and real firmware traces unverified |
| PCI BAR and command register | Standard PCI config model, not Discovery II-specific evidence | BAR probing, Memory Decode and synthetic MMIO aperture | Valid as diagnostic model, not authentic controller outbound mapping |
| Internal base and decode enables 0x068/0x278 | Historical MV64340 header only | Not exposed by controller | Variant-specific semantics and reset values unverified |
| Pegasos II controller base | Not established here by primary board evidence | Explicit caller-provided base in tests | No default physical address may be asserted |
| IRQ, DMA, timers and PCI outbound windows | No validated register-level model | Unsupported | Requires primary manual or firmware/driver trace |

### Decision

Keep the four PCI configuration offsets behind
`ppcvm_discovery_ii_enable_pci_config`. Do **not** promote them to
unconditionally enabled, hardware-accurate MV64361 registers. In
particular, the current 4-KiB controller mapping and the independent PCI
MMIO aperture are diagnostic arrangements, not a discovered Pegasos II
memory map.

### Evidence required for promotion

1. MV64361-specific datasheet/reference manual, or a trace from genuine
   Pegasos II firmware/OS that establishes exact register base and access
   widths/endian conversion.
2. Compare configuration address encoding, read/write masks and reset
   defaults with the current emulation and record any differences.
3. Validate which controller PCI interface connects to each actual board
   device and its outbound memory window.
4. Add failing regression tests for each documented discrepancy *before*
   changing behavior; preserve synthetic compatibility separately.

**Next engineering step (M4.18):** a separately configurable, disabled-by-
default controller PCI memory-window model with explicit bounds and
translation, after its real register programming contract is established.
