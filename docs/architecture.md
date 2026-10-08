# Architecture (M0)

- Portable C11 interpreter is the correctness reference. Start with PPC32 big-endian semantics and a precise execution/test harness.
- Split future modules into CPU (including exceptions), MMU, memory/bus, firmware/boot, PCI and peripherals, machine profiles, graphics/input/audio and management.
- First profile: Pegasos II / PowerPC G4 / Marvell Discovery II. Device registers, firmware expectations and real operating-system boot traces must be researched and validated before claiming compatibility.
- Profiles after Pegasos II: Sam460ex and Sam460cr, then other PowerPC hardware. Do not assume a guest OS supports a particular board.
- Authentic mode prioritizes reproducible hardware semantics; fast mode may use JIT once compared with interpreter traces.
- Appliance deployment uses an optional minimal Linux host, unattended start, full-screen direct display, keyboard/mouse/audio, graceful shutdown, and recovery. Host isolation and a separate admin escape hatch are mandatory.
- Never bundle commercial OS images, proprietary firmware or machine ROMs.
