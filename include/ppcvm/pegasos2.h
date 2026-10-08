#ifndef PPCVM_PEGASOS2_H
#define PPCVM_PEGASOS2_H
#include "ppcvm/bus.h"
#include "ppcvm/cpu.h"
#include "ppcvm/memory.h"
#include "ppcvm/mmu.h"
#include <stddef.h>
#include <stdint.h>
/* Provisional discovery window for testing, not a validated Pegasos II map. */
#define PPCVM_PEGASOS2_DISCOVERY_BASE UINT32_C(0xf1000000)
#define PPCVM_PEGASOS2_DISCOVERY_SIZE UINT32_C(0x1000)
typedef struct {
  ppcvm_cpu cpu;
  ppcvm_memory ram;
  ppcvm_bus bus;
  ppcvm_bat_state bat;
  ppcvm_segment_state segments;
  uint32_t discovery_scratch;
  uint32_t discovery_reads;
  uint32_t discovery_writes;
} ppcvm_pegasos2;
/* Host-side diagnostic firmware information service; no guest trap ABI yet. */
#define PPCVM_PEGASOS2_FW_QUERY_VERSION UINT32_C(1)
#define PPCVM_PEGASOS2_FW_QUERY_RAM_BYTES UINT32_C(2)
#define PPCVM_PEGASOS2_FW_QUERY_BOOT_MAGIC UINT32_C(3)
ppcvm_result ppcvm_pegasos2_firmware_query(const ppcvm_pegasos2 *machine,
    uint32_t selector, uint32_t *value);
/* Diagnostic mailbox: four BE words at address: selector, argument,
   result, status. Host processes one request on explicit invocation.
   status 0=OK, 1=unsupported, 2=invalid. No automatic MMIO trap. */
#define PPCVM_PEGASOS2_FW_MAILBOX_SIZE UINT32_C(16)
ppcvm_result ppcvm_pegasos2_firmware_mailbox(ppcvm_pegasos2 *machine,
    uint32_t address);
/* Map caller-owned, read-only diagnostic/firmware bytes at the high vector prefix. */
ppcvm_bus_result ppcvm_pegasos2_map_high_rom(ppcvm_pegasos2 *machine, uint8_t *bytes, uint32_t size);
/* Reset CPU and synthetic device state without clearing RAM or ROM mappings. */
void ppcvm_pegasos2_reset(ppcvm_pegasos2 *machine);
/* Diagnostic cold reset: clear RAM and MMU configuration, preserve bus/ROM maps. */
void ppcvm_pegasos2_cold_reset(ppcvm_pegasos2 *machine);
/* Start at a mapped high-ROM instruction address; reset CPU only. */
ppcvm_result ppcvm_pegasos2_boot_high_rom(ppcvm_pegasos2 *machine, uint32_t entry);
/* Validate ROM entry, then cold-reset machine and begin execution there. */
ppcvm_result ppcvm_pegasos2_cold_boot_high_rom(ppcvm_pegasos2 *machine, uint32_t entry);
/* Diagnostic boot ABI v1: registers r3=magic, r4=info address, r5=RAM bytes,
   r6=entry; all other GPRs and privileged state reset. Not an OF ABI. */
#define PPCVM_PEGASOS2_BOOT_MAGIC UINT32_C(0x50564331)
#define PPCVM_PEGASOS2_BOOT_INFO_SIZE UINT32_C(16)
/* Extended PVC1 record, opt-in; first 16 bytes retain v1 layout. */
#define PPCVM_PEGASOS2_BOOT_INFO_V2_SIZE UINT32_C(32)
ppcvm_result ppcvm_pegasos2_boot_elf32_abi_v2(ppcvm_pegasos2 *machine,
    const uint8_t *image, size_t size, uint32_t info_address);
ppcvm_result ppcvm_pegasos2_boot_elf32_abi(ppcvm_pegasos2 *machine,
    const uint8_t *image, size_t size, uint32_t info_address);
/* Load an ELF32 guest and transfer CPU PC to its validated entry. */
ppcvm_result ppcvm_pegasos2_boot_elf32(ppcvm_pegasos2 *machine, const uint8_t *image, size_t size);
/* Load ELF32 big-endian PowerPC PT_LOAD segments into guest RAM atomically. */
ppcvm_result ppcvm_pegasos2_load_elf32(ppcvm_pegasos2 *machine, const uint8_t *image, size_t size, uint32_t *entry);
/* Load a raw big-endian PPC image into RAM; caller supplies load address. */
ppcvm_result ppcvm_pegasos2_load_raw(ppcvm_pegasos2 *machine, uint32_t address, const uint8_t *bytes, size_t size);
/* Set CPU entry after checking a mapped, aligned RAM instruction address. */
ppcvm_result ppcvm_pegasos2_enter_ram(ppcvm_pegasos2 *machine, uint32_t entry);
int ppcvm_pegasos2_init(ppcvm_pegasos2 *machine, size_t ram_size);
/* Fetches one big-endian instruction from mapped memory and executes it. */
ppcvm_result ppcvm_pegasos2_step(ppcvm_pegasos2 *machine);
/* Opt-in synthetic trap: guest executes sc with r3=PVC1 magic and
   r4=mailbox address. Only this step function intercepts the call. */
ppcvm_result ppcvm_pegasos2_step_firmware(ppcvm_pegasos2 *machine);
/* Opt-in prototype: instruction fetch faults enter ISI vector 0x400. */
ppcvm_result ppcvm_pegasos2_step_isi(ppcvm_pegasos2 *machine);
/* Opt-in combined ISI/DSI stepping; alignment remains an explicit fault. */
ppcvm_result ppcvm_pegasos2_step_exceptions(ppcvm_pegasos2 *machine);
/* Opt-in BAT instruction fetch; data accesses still use physical bus addresses. */
ppcvm_result ppcvm_pegasos2_step_bat_fetch(ppcvm_pegasos2 *machine);
/* Opt-in IBAT+DBAT translation for basic lwz/lbz/stw/stb. */
ppcvm_result ppcvm_pegasos2_step_bat(ppcvm_pegasos2 *machine);
/* Opt-in BAT+PTE instruction/data translation, still prototype-only. */
ppcvm_result ppcvm_pegasos2_step_pte(ppcvm_pegasos2 *machine);
/* Opt-in PTE translation with SR protection keys and R/C tracking. */
ppcvm_result ppcvm_pegasos2_step_pte_keyed(ppcvm_pegasos2 *machine);
/* Runs up to limit instructions, stopping at the first fault.
   executed is set to the number of successful instructions. */
ppcvm_result ppcvm_pegasos2_run(ppcvm_pegasos2 *machine, size_t limit, size_t *executed);
void ppcvm_pegasos2_destroy(ppcvm_pegasos2 *machine);
#endif
