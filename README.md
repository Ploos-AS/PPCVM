# PPCVM

Portable PowerPC virtual machine and machine emulator by Ploos AS.

**Status:** Experimental PowerPC32/Pegasos II emulator. Cross-assembled PowerPC ELF diagnostics execute inside PPCVM in CI; no guest operating system boots yet. Firmware, Linux handoff, MMU, and full G4 compatibility remain incomplete.

## Goals
- **P0:** MorphOS, AmigaOS 4.x, AROS PPC.
- **P1:** Linux PowerPC, OpenBSD/macppc, NetBSD/ofppc and macppc.
- **First machine:** Pegasos II (G4 / Marvell Discovery II), followed by Sam460ex, Sam460cr, compatible Power Mac, and later AmigaOne, Efika, Genesi ODW and Mirari T1042.
- **Operating modes:** desktop, headless/CI, and appliance (host Linux hidden).
- Faithful emulation first; optional JIT acceleration later.

The project now includes a growing **32-bit PowerPC interpreter**, a provisional Pegasos II machine model, PCI/Discovery II diagnostics, and experimental firmware/boot interfaces. These are development scaffolds, not validated Pegasos II hardware or Open Firmware compatibility.\n\n**CI evidence:** `build-test` and `windows-build-test` exercise host builds/tests; `powerpc-guest-execution` cross-builds PowerPC ELF programs and runs guest diagnostics under PPCVM. None of these jobs currently demonstrates a Linux, MorphOS, or AmigaOS 4 boot. See [roadmap](docs/roadmap.md) for the reset-to-firmware and Linux-handoff priorities.

## Build and test
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/ppcvm
```

## Documentation
See [architecture](docs/architecture.md), [roadmap](docs/roadmap.md) and [legal notes](docs/legal.md).
