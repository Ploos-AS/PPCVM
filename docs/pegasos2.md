# Pegasos II — first machine skeleton

This is an **experimental machine scaffold**, not yet a hardware-compatible Pegasos II.

- CPU: generic PPC32 interpreter; **not yet a 7447/G4 implementation**.
- RAM: guest physical RAM mapped from address zero.
- Discovery II: provisional MMIO window at `0xf1000000`, currently exposing only
  a **synthetic scratch register at offset zero** for integration testing.
  This address and register must **not** be treated as validated hardware documentation.
- Firmware, PCI, interrupt controller, MMU, real Discovery II registers, graphics,
  and boot devices: **not implemented**.

Next: validate the physical address map from primary hardware documentation and
open firmware behavior before adding real chipset registers. Never distribute
proprietary firmware or commercial OS images in the repository.

Classic Amiga PPC accelerator machines remain a separate future integration track
with AmiVM handling Amiga chipset/68k and PPCVM supplying the PPC core.

## Experimental high-ROM entry

`ppcvm_pegasos2_map_high_rom(machine, bytes, size)` maps caller-owned, read-only
bytes at `0xfff00000` (size 4 KiB to 1 MiB). Keep the bytes alive until the
machine is destroyed. `ppcvm_pegasos2_boot_high_rom(machine, entry)` checks
that the aligned entry instruction is mapped, resets CPU registers, selects
MSR[IP], and starts at that address. A failed entry leaves CPU state intact.
This is a synthetic smoke-test entry, **not** a verified Pegasos II reset
sequence or a replacement for Open Firmware. No ROM is bundled.

## Reset semantics (prototype)

`ppcvm_pegasos2_reset()` resets CPU registers and the synthetic Discovery
scratch register/counters. RAM contents, ROM mappings, bus regions, BAT and
segment configuration are retained. This is a **warm diagnostic reset**, not
yet a faithful Pegasos II power-on reset. Use `ppcvm_pegasos2_init()` for a
fresh machine. A later hardware reset model will define device and MMU reset
semantics explicitly.

`ppcvm_pegasos2_cold_reset()` is a separate **diagnostic** reset: it also
zeros guest RAM and the BAT/segment/SDR1 translation state, while preserving
mapped ROM bytes and bus regions. Unlike a hardware-faithful power cycle,
this intentionally erases RAM and does not model firmware or device reset
sequencing. Neither reset operation frees caller-owned ROM buffers.

`ppcvm_pegasos2_cold_boot_high_rom(machine, entry)` validates an aligned,
mapped high-ROM instruction address **before** clearing RAM and reset state.
If validation fails, the machine is unchanged. If it succeeds, the diagnostic
cold reset runs and the CPU begins at `entry` with MSR[IP] selected. This is
not a real Pegasos II reset vector or Open Firmware boot protocol.

## M2 synthetic firmware-to-RAM handoff

`test_pegasos2_firmware_handoff` cold-boots a caller-supplied diagnostic ROM.
The ROM constructs a PowerPC instruction in registers, writes it to guest RAM,
sets CTR to RAM address zero, and branches via `bctr`. The CPU then fetches and
executes that instruction from RAM. This verifies a minimal firmware-to-guest
execution handoff using existing C11 CPU and bus components; it does **not**
implement Open Firmware, actual Pegasos II firmware, or OS boot.

## M2 raw program loader

`ppcvm_pegasos2_load_raw(machine, address, bytes, size)` copies a nonempty
raw PowerPC big-endian instruction/data image into mapped guest RAM with
bounds checks. `ppcvm_pegasos2_enter_ram(machine, entry)` validates a 4-byte
aligned RAM entry and changes the CPU PC without resetting registers or MSR.
A CTest loads and executes two instructions. This is **not** an ELF loader,
firmware ABI, or executable format parser; callers must know the image's load
and entry addresses and prepare CPU state explicitly.

## M2 experimental ELF32 loader

`ppcvm_pegasos2_load_elf32(machine, image, size, &entry)` accepts a bounded,
big-endian ELF32 PowerPC `ET_EXEC` image with 32-byte program headers. It
validates all `PT_LOAD` file/RAM bounds and segment overlap before writing,
copies file bytes, zero-fills BSS, and returns the entry address. It rejects
entries outside loaded segments. It does not interpret relocations, dynamic
linking, section headers, MMU mappings, or firmware ABIs. The caller uses
`ppcvm_pegasos2_enter_ram()` to transfer execution.

The ELF32 preflight also checks `PT_LOAD` flags (only PF_R/PF_W/PF_X)
and `p_align` (zero/one or power of two, with congruent file offset and
physical load address). These checks reject malformed segments before
modifying guest RAM; they do not yet enforce runtime memory permissions.

ELF entry validation requires a complete 4-byte instruction inside the
file-backed portion of an executable (`PF_X`) `PT_LOAD` segment. An entry in
BSS or a data-only segment is rejected before RAM is modified. Runtime
execution permissions are not yet enforced by the bus or MMU.

## M2 ELF32 guest entry helper

`ppcvm_pegasos2_boot_elf32(machine, image, size)` validates and loads an
ELF32 executable, then sets CPU PC to its file-backed executable entry.
It leaves CPU registers and MSR unchanged, does not perform cold reset, and
is **not** a firmware boot protocol. Rejected ELF files leave the CPU PC
and RAM unchanged; successful loading intentionally replaces RAM segments.

## Experimental boot ABI v1

`ppcvm_pegasos2_boot_elf32_abi(machine, image, size, info_address)` loads an
ELF32 guest, resets CPU state, and enters the ELF entry with `r3=0x50564331`
(`PVC1`), `r4=info_address`, `r5=RAM size`, `r6=entry`. The 16-byte
big-endian record at `info_address` contains magic, record length, RAM size,
and entry. The record must be word-aligned, fit in RAM and not overlap any
ELF `PT_LOAD` memory region. This is a PPCVM-only diagnostic convention,
**not** Open Firmware, CHRP, or a MorphOS/AmigaOS boot ABI.

### Extended diagnostic record (v2)

`ppcvm_pegasos2_boot_elf32_abi_v2` preserves the first four big-endian
words of PVC1 and extends the record to 32 bytes. Word offsets `+16`,
`+20`, `+24`, `+28` contain version `2`, flags `0`, reserved service
pointer `0`, and reserved `0`. The full 32-byte record is protected
against overlaps with ELF segments. These fields are reserved for future
work; no firmware services are implemented by this record.

### Host-side firmware information queries

`ppcvm_pegasos2_firmware_query` supports three read-only selectors:
`1` (PVC1 ABI version, currently 2), `2` (RAM bytes), and `3` (PVC1
magic). Unsupported selectors leave the output untouched. This is a
host API only: **guest PowerPC code cannot invoke these services yet**.
No Open Firmware client interface is claimed.
