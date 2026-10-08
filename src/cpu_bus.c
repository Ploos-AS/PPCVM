#include "ppcvm/cpu.h"
/* Bus-backed data operations; non-memory opcodes share the reference interpreter.
   Instruction fetch, MMU translation and exception vectors are not implemented yet. */
ppcvm_result ppcvm_cpu_step_bus(ppcvm_cpu *cpu, ppcvm_bus *bus, uint32_t insn) {
  if (!cpu) return PPCVM_MEMORY_FAULT;
  uint32_t op = insn >> 26;
  if (op != 32 && op != 34 && op != 36 && op != 38)
    return ppcvm_cpu_step(cpu, insn);
  if (!bus) return PPCVM_MEMORY_FAULT;
  uint32_t rt = (insn >> 21) & 31u;
  uint32_t ra = (insn >> 16) & 31u;
  uint32_t ea = (ra ? cpu->gpr[ra] : 0u) + (uint32_t)(int32_t)(int16_t)(insn & 0xffffu);
  if ((op == 32 || op == 36) && (ea & 3u)) return PPCVM_MEMORY_FAULT;
  ppcvm_bus_result status;
  uint32_t word = 0;
  uint8_t byte = 0;
  switch (op) {
    case 32:
      status = ppcvm_bus_read32be(bus, ea, &word);
      if (status != PPCVM_BUS_OK) return PPCVM_MEMORY_FAULT;
      cpu->gpr[rt] = word;
      break;
    case 34:
      status = ppcvm_bus_read8(bus, ea, &byte);
      if (status != PPCVM_BUS_OK) return PPCVM_MEMORY_FAULT;
      cpu->gpr[rt] = byte;
      break;
    case 36:
      status = ppcvm_bus_write32be(bus, ea, cpu->gpr[rt]);
      if (status != PPCVM_BUS_OK) return PPCVM_MEMORY_FAULT;
      break;
    default:
      status = ppcvm_bus_write8(bus, ea, (uint8_t)cpu->gpr[rt]);
      if (status != PPCVM_BUS_OK) return PPCVM_MEMORY_FAULT;
      break;
  }
  cpu->pc += 4;
  return PPCVM_OK;
}
