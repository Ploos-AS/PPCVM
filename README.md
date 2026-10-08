# PPCVM

Portable PowerPC virtual machine and machine emulator by Ploos AS.

**Status:** M0 bootstrap. No guest OS boots yet.

## Goals
- **P0:** MorphOS, AmigaOS 4.x, AROS PPC.
- **P1:** Linux PowerPC, OpenBSD/macppc, NetBSD/ofppc and macppc.
- **First machine:** Pegasos II (G4 / Marvell Discovery II), followed by Sam460ex, Sam460cr, compatible Power Mac, and later AmigaOne, Efika, Genesi ODW and Mirari T1042.
- **Operating modes:** desktop, headless/CI, and appliance (host Linux hidden).
- Faithful emulation first; optional JIT acceleration later.

M0 provides a tiny, testable **32-bit PowerPC instruction subset** only; it does not claim machine, firmware, MMU, or operating-system compatibility.

## Build and test
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/ppcvm
```

## Documentation
See [architecture](docs/architecture.md), [roadmap](docs/roadmap.md) and [legal notes](docs/legal.md).
