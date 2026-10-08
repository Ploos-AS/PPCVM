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
