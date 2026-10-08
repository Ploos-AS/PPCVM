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

### Diagnostic guest RAM mailbox

`ppcvm_pegasos2_firmware_mailbox(machine, address)` explicitly processes a
16-byte, word-aligned mailbox in guest RAM. Big-endian words: `+0` selector,
`+4` argument (reserved), `+8` result, `+12` status (`0` success,
`1` unsupported, `2` invalid). A failed selector clears the result and
sets status. The request words are preserved. The caller must explicitly
invoke this host function after guest code writes the request; **there is
no automatic guest trap, interrupt or MMIO trigger yet**. This mailbox is
not an Open Firmware interface.

### Opt-in guest-triggered diagnostic call

`ppcvm_pegasos2_step_firmware()` recognizes instruction `sc`
(`0x44000002`) **only when** guest `r3` equals the PVC1 magic and
`r4` points to the aligned 16-byte mailbox. It processes one mailbox
request, sets `r3=0` on success or `r3=1` for unsupported selectors,
and advances PC by four. Invalid mailbox addresses return a memory fault
without advancing PC. Other instructions use the normal bus CPU step.
This opt-in synthetic trap is **not** Open Firmware and is not enabled
by `ppcvm_pegasos2_step()`.

### PVC1 diagnostic ABI compatibility contract

The PVC1 magic (`0x50564331`) and the first 16 bytes of the boot
record remain compatible between the original ABI and opt-in v2.
The v2 record is 32 bytes; offset `+16` is the version number `2`.
All mailbox fields are 32-bit **big-endian** words. Selector IDs `1`,
`2`, and `3` and status codes `0` (success), `1` (unsupported), `2`
(invalid) are reserved with their current meanings. Unknown selectors
must not mutate request fields, and their response value is zero.
Guest trap activation requires the exact `sc` encoding and PVC1 magic
in `r3` while using the opt-in firmware stepping API; normal `sc`
behavior is preserved in standard stepping. New fields and services
must be added without reinterpreting these existing IDs or offsets.
This is a **prototype diagnostic ABI**, not an Open Firmware ABI or a
claim of OS boot compatibility.

### Generic PCI configuration groundwork (M3)

`include/ppcvm/pci.h` and `src/pci.c` provide a standalone 256-byte
PCI configuration header model. Config dword accesses use **PCI little-
endian** byte order, independent of the big-endian PowerPC CPU. Vendor,
device, class and revision identification are immutable through this
prototype write API; other aligned dwords are currently plain storage.
No BAR sizing, command-register masks, PCI bus enumeration, interrupts,
chipset address decoding or verified Pegasos II device IDs are modeled.
The generic PCI model is **not wired into the Pegasos II bus yet**.

### Standalone PCI BDF registry

`ppcvm_pci_bus` holds up to 16 test devices, addressed by PCI bus
(0–255), device (0–31), and function (0–7). Reads from an unpopulated
BDF return `0xffffffff`, and writes are ignored. Duplicate registrations
and invalid device/function numbers are rejected. This remains a generic
software model: no Pegasos II host bridge, configuration mechanism, PCI
BAR probing or interrupt routing is connected yet.

### PCI 32-bit memory BAR prototype

`ppcvm_pci_set_mem_bar32` configures a non-prefetchable 32-bit memory
BAR with a power-of-two size of at least 16 bytes and an aligned base.
Writing `0xffffffff` to a configured BAR enables PCI size probing:
subsequent reads return its address mask. Writing a new address ends
probing and aligns the base to the configured BAR size. Each BAR has
independent state. Unconfigured BARs remain generic storage. I/O BARs,
64-bit BAR pairs, BAR-to-MMIO decoding and real Pegasos II chipset
integration are not yet supported.

### PCI I/O BAR prototype

`ppcvm_pci_set_io_bar32` adds 32-bit I/O-space BARs (power-of-two
size >=4 bytes). Probing returns the I/O address mask with bit 0 set;
normal writes retain bit 0 and align the address. Memory and I/O BARs
can coexist on a device. This is configuration-space emulation only:
there is no CPU port-I/O mapping or Pegasos II host-bridge decoding.

### PCI 64-bit memory BAR prototype

`ppcvm_pci_set_mem_bar64` reserves two consecutive BAR registers,
starting at indices 0–4. Both low and high 32-bit halves support
independent sizing probes; the low half reports memory type `10b`.
Sizes are power-of-two, at least 16 bytes, and can exceed 4 GiB.
Configuration writes are masked to the configured size. This is still
an isolated PCI configuration model without real MMIO decoding.

### Standalone PCI memory address decoding

`ppcvm_pci_bus_decode_memory` resolves an address against configured
32-bit and 64-bit memory BARs and returns BDF, BAR index and byte offset.
I/O BARs are excluded. Unmapped addresses return 1; overlapping BARs
return -1 rather than silently choosing a device. This is a host-side
lookup only; it does **not** attach PCI regions to PowerPC memory access,
implement PCI command-register decode enable bits, or establish real
Pegasos II bridge windows.

### PCI Command decode enable

Memory BAR lookup requires PCI Command bit 1 (Memory Space Enable).
Devices start with memory decoding disabled; guest configuration writes
to offset `0x04` can enable or disable it. The prototype currently
models only writable Command bits 0–2 and treats the Status halfword as
read-only. I/O Space Enable and Bus Master Enable are stored but not
connected to I/O or DMA engines. This remains a standalone model.

### PCI I/O-space address decoding

`ppcvm_pci_bus_decode_io` resolves 32-bit I/O BAR addresses only when
PCI Command bit 0 (I/O Space Enable) is set. Memory BARs are ignored,
and overlaps return an error rather than selecting an arbitrary device.
This lookup is host-side only: no PowerPC I/O access instruction,
PCI bridge routing or port-I/O device callbacks are implemented yet.

### PCI MMIO callback dispatch

A registered PCI BDF may expose host-side `read32` and `write32`
callbacks. `ppcvm_pci_bus_mmio_read32/write32` decode the address,
require Memory Space Enable, and forward the BAR index and byte offset.
Only aligned 32-bit operations are supported. Missing callbacks,
ambiguous ranges and unmapped addresses fail explicitly. This API is
not yet connected to the PowerPC CPU bus, and callback values are
host-native `uint32_t`; device-specific endian behavior is not modeled.

### Optional CPU bus PCI MMIO aperture

`ppcvm_pci_map_mmio_aperture` maps an explicitly chosen, fixed 32-bit
aperture on `ppcvm_bus`. Aligned `read32be`/`write32be` accesses are
forwarded to PCI memory BAR decoding and device callbacks. PCI Command
Memory Space Enable is enforced. The aperture and PCI registry must
outlive the CPU bus mapping. BAR relocation is visible only within the
fixed aperture; it does not dynamically resize or move the mapping.
The caller must choose an address range that does not overlap RAM/ROM
or other MMIO. This is an opt-in prototype, not a validated Pegasos II
PCI host-bridge aperture. Byte accesses and 64-bit CPU addresses are
not supported by this adapter.

### PowerPC instruction-to-PCI regression

`test_pci_cpu_instructions` executes encoded PowerPC `stw` and `lwz`
through `ppcvm_cpu_step_bus`, the 32-bit CPU MMIO aperture, PCI BAR
address decode, and a registered virtual device callback. It checks
that PCI Command Memory Space Enable gates both operations and that
register values round-trip. Instructions are supplied by the host test;
this is not yet a guest firmware or OS boot, and the adapter does not
implement a real Pegasos II PCI host bridge.

### RAM-fetched PowerPC PCI diagnostic

`test_pci_ram_program` stores three big-endian PowerPC instructions in
emulated RAM, fetches them via the CPU bus, and executes `stw`, `lwz`
and `ori` through the reference interpreter. The store/load pair reaches
a PCI MMIO callback and the final register copy verifies the value.
The host test explicitly drives the fetch/step loop; this is not a
firmware-driven guest boot or complete Pegasos II PCI implementation.

### Reusable CPU bus fetch/execute step

`ppcvm_cpu_step_bus_fetch(cpu,bus)` fetches one aligned big-endian
PowerPC instruction from the CPU bus at `pc`, then executes it using
`ppcvm_cpu_step_bus`. The PCI RAM-program regression now uses this API.
Unmapped or unaligned instruction fetch returns a memory fault without
advancing PC. This helper does not perform MMU instruction translation
or deliver an ISI exception; it is intended for direct-mapped diagnostics.

### Bounded CPU run loop

`ppcvm_cpu_run_bus(cpu,bus,max_steps)` repeatedly fetches and executes
instructions with a deterministic maximum step count. It reports the
number of successfully executed instructions, final PC, and whether it
stopped at the limit, an unsupported instruction, a memory fault, or
invalid arguments. A zero limit performs no instructions. The helper
uses direct bus addressing without instruction MMU translation and
is intended for diagnostics, not a complete machine scheduler.

### Opt-in diagnostic halt

`ppcvm_cpu_run_bus_diagnostic` recognizes the synthetic zero word
`PPCVM_DIAGNOSTIC_HALT` as a stop marker. It reports `PPCVM_RUN_HALT`
without executing or counting the marker, leaving PC at its address.
The ordinary `ppcvm_cpu_run_bus` and CPU interpreter do not intercept
this word. **This is not a PowerPC HALT instruction**, not firmware,
and not enabled for normal guest execution.

The `test_pci_ram_program` integration diagnostic now places the halt
marker after its three PowerPC instructions and runs with
`ppcvm_cpu_run_bus_diagnostic`. It asserts that the run ends with
`PPCVM_RUN_HALT`, three executed instructions, PC at the marker,
and exactly one PCI MMIO read and write. This remains a synthetic
host-configured PCI test, not a booted guest operating system.

### ELF32 PowerPC PCI diagnostic

`test_pci_elf_diagnostic` constructs an ELF32 big-endian PowerPC ET_EXEC
image with an executable PT_LOAD segment, boots it through the existing
Pegasos II ELF loader, and runs its `stw`/`lwz`/`ori`/diagnostic-halt
sequence through the bounded diagnostic runner. It checks PCI callback
counts and register round-trip. The test builds ELF bytes on the host;
it does not invoke an external PowerPC cross-compiler or boot real
firmware. PCI registration, BAR configuration and guest register inputs
are host-provided; this is not an OS boot.

### Cross-assembled PCI diagnostic source

`diagnostics/pci_mmio.S` is a freestanding PowerPC assembly source with
`stw`, `lwz`, `ori` and the PPCVM-only diagnostic halt marker. Its linker
script places the entry at guest address `0x100`. The separate CI job
`powerpc-assembly` builds an ELF32 image using GNU PowerPC cross-binutils
and prints ELF metadata and disassembly. The existing C integration test
still constructs its own ELF bytes and does not yet load the CI-built
artifact. The host supplies PCI BAR configuration and initial registers.
