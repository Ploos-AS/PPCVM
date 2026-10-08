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

## M6+ – Expansion
Sam460cr, appropriate Power Mac profiles (OpenBSD/macppc), AmigaOne, Efika, Genesi ODW, Mirari T1042; JIT and appliance images.

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
