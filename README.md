# PPCVM

Portable PowerPC virtual machine and machine emulator by Ploos AS.

**Status:** Experimental PowerPC32/Pegasos II emulator. Cross-assembled PowerPC ELF diagnostics execute inside PPCVM in CI; no guest operating system boots yet. Firmware, Linux handoff, MMU, and full G4 compatibility remain incomplete.

## Goals
- **P0:** MorphOS, AmigaOS 4.x, AROS PPC.
- **P1:** Linux PowerPC, OpenBSD/macppc, NetBSD/ofppc and macppc.
- **First machine:** Pegasos II (G4 / Marvell Discovery II), followed by Sam460ex, Sam460cr, compatible Power Mac, and later AmigaOne, Efika, Genesi ODW and Mirari T1042.
- **Operating modes:** desktop, headless/CI, and appliance (host Linux hidden).
- Faithful emulation first; optional JIT acceleration later.

The project now includes a growing **32-bit PowerPC interpreter**, a provisional Pegasos II machine model, PCI/Discovery II diagnostics, and experimental firmware/boot interfaces. These are development scaffolds, not validated Pegasos II hardware or Open Firmware compatibility.

**CI evidence:** `build-test` and `windows-build-test` exercise host builds/tests; `powerpc-guest-execution` cross-builds PowerPC ELF programs and runs guest diagnostics under PPCVM. None of these jobs currently demonstrates a Linux, MorphOS, or AmigaOS 4 boot. See [roadmap](docs/roadmap.md) for the reset-to-firmware and Linux-handoff priorities.

## Build and test
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/ppcvm --elf path/to/powerpc-be-elf32.elf --ram 128 --steps 10000 --trace --trace-limit 200 --dump-registers
```

## First Linux/PPC boot investigation

PPCVM currently accepts **ELF32 big-endian PowerPC** executables, not arbitrary
compressed `vmlinuz`, `zImage`, `uImage`, or a complete Linux installation.
Use `readelf -h -l kernel.elf` to check the file before loading it. The
interpreter must not be mistaken for a real Pegasos II firmware environment.

A first diagnostic attempt with a suitable **uncompressed, ELF32 PowerPC**
kernel image is:

```sh
readelf -h -l kernel.elf
./build/ppcvm --elf kernel.elf --ram 256 --steps 100000 \
  --trace --trace-limit 200 --dump-registers
```

A successful ELF load is **not** evidence that Linux booted. Linux/PPC
expects a board-specific firmware handoff (including a device tree or
Open Firmware services, depending on kernel and platform), correct physical
memory layout, exception behavior, interrupt controller and devices.
PPCVM's `--abi-v2` uses its private PVC1 diagnostic record and must **not**
be passed as a Linux boot ABI. The initial goal is to record the first
unsupported instruction, memory fault or missing firmware dependency and
implement that prerequisite, not to declare boot success after a step limit.

## Documentation
See [architecture](docs/architecture.md), [roadmap](docs/roadmap.md) and [legal notes](docs/legal.md).
