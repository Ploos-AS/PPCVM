# Roadmap

## M0 – Bootstrap
- [x] C11 project and testable PPC32 instruction scaffold
- [x] CMake/CTest and GitHub Actions
- [x] Architecture, legal policy and priority OS matrix
- [ ] Full CPU instruction semantics, exception model, MMU, bus and real machine profile (later milestones)

## M1 – Pegasos II fundamentals
CPU G4 model; exceptions/MMU; machine memory map; Discovery II registers; UART and deterministic tests.

## M2 – Firmware and MorphOS
Document compatible firmware acquisition/implementation; obtain first boot traces; validate MorphOS startup.

## M3 – AmigaOS 4 and Unix guests
AmigaOS 4.x, Linux PPC and NetBSD/ofppc on Pegasos II, where guest release and firmware permit.

## M4 – I/O and stability
PCI, storage, display, networking, sound, input, snapshots and repeatable CI qualification.

## M5 – AROS PPC hardware profile
Sam460ex or verified supported board/port; prove boot and program execution. AROS PPC is **P0** despite using a second machine.

## M6 – Sam family (after Pegasos II qualification)
- [ ] Shared PPC 440EP core and peripherals for Sam440ep and Sam440ep-Flex.
- [ ] Separate Sam440ep and Sam440ep-Flex board profiles.
- [ ] PPC 460EX core and Sam460ex board profile.
- [ ] Confirm OS and firmware support separately for each board; never infer boot compatibility from CPU family alone.
- [ ] Keep Pegasos II as the only active first-machine implementation until its boot/runtime gates pass.

## M7 – Additional bplan/Genesi machines
- [ ] Pegasos I: separate Articia S board/chipset model; do not reuse Pegasos II Discovery II mappings.
- [ ] Efika: separate MPC5200B SoC, board peripherals and firmware model.
- [ ] Validate each board's Open Firmware behavior and guest compatibility independently.

## M8 – ACube Sam460 variants
- [ ] Sam460ex: PPC460EX SoC and board peripherals (planned in M6).
- [ ] Sam460cr: independent board configuration and peripheral differences; do not assume Sam460ex firmware interchangeability.
- [ ] Boot and runtime tests for each supported guest and board combination.

## M9 – High-end and newer PPC machines
- [ ] A-EON X5000: distinct NXP/Freescale P5020/P5040 e5500 CPU variants and Cyrus board hardware, firmware and PCIe.
- [ ] Mirari T1042: distinct QorIQ T1042/e5500 platform profile; establish public hardware and firmware specifications before implementation.
- [ ] Document CPU endian modes, device trees, interrupt controllers and MMU differences per machine.
- [ ] Do not imply compatibility with proprietary firmware or operating systems without boot evidence.

## M10+ – Further expansion
Appropriate Power Mac profiles (OpenBSD/macppc), AmigaOne, Genesi ODW, JIT and appliance images.

## Machine profile priority
| Priority | Board | Status |
| --- | --- | --- |
| P0 | bplan Pegasos II | Active skeleton; not boot-capable |
| P1 | bplan Pegasos I | Planned |
| P1 | bplan Efika | Planned |
| P1 | ACube Sam440ep / Sam440ep-Flex | Planned |
| P1 | ACube Sam460ex | Planned |
| P1 | ACube Sam460cr | Planned |
| P2 | A-EON X5000 | Planned |
| P2 | Mirari T1042 | Planned |

Machine priority controls implementation order, not guaranteed guest OS support. Each profile requires verified board documentation, firmware strategy, CPU/MMU/interrupt behavior, peripheral mapping, and repeatable boot tests.

## Guest priority and qualification
| Priority | Guest | Initial target |
| --- | --- | --- |
| P0 | MorphOS | Pegasos II |
| P0 | AmigaOS 4.x | Pegasos II |
| P0 | AROS PPC | verified supported Sam-family machine/port |
| P1 | Linux PPC | Pegasos II |
| P1 | NetBSD/ofppc | Pegasos II |
| P1 | OpenBSD/macppc | compatible Power Mac profile |
| P1 | NetBSD/macppc | compatible Power Mac profile |

No OS qualification is asserted until its boot and runtime tests pass.
