#include "ppcvm/cpu.h"
#include <string.h>
void ppcvm_cpu_reset(ppcvm_cpu *cpu) { memset(cpu, 0, sizeof(*cpu)); }
ppcvm_result ppcvm_cpu_step_memory(ppcvm_cpu *cpu, ppcvm_memory *memory, uint32_t insn) {
  uint32_t opcode = insn >> 26;
  uint32_t rt = (insn >> 21) & 31u;
  uint32_t ra = (insn >> 16) & 31u;
  uint32_t imm = insn & 0xffffu;
  uint32_t next_pc = cpu->pc + 4u;
  switch (opcode) {
    case 14: /* addi */
      cpu->gpr[rt] = (ra ? cpu->gpr[ra] : 0u) + (uint32_t)(int32_t)(int16_t)imm;
      break;
    case 15: /* addis */
      cpu->gpr[rt] = (ra ? cpu->gpr[ra] : 0u) + ((uint32_t)((int32_t)(int16_t)imm * 65536));
      break;
    case 24: /* ori */
      cpu->gpr[ra] = cpu->gpr[rt] | imm;
      break;
    case 25: /* oris */
      cpu->gpr[ra] = cpu->gpr[rt] | (imm << 16);
      break;
    case 26: /* xori */
      cpu->gpr[ra] = cpu->gpr[rt] ^ imm;
      break;
    case 28: { /* andi. */
      cpu->gpr[ra] = cpu->gpr[rt] & imm;
      int32_t value = (int32_t)cpu->gpr[ra];
      uint32_t cr0 = value < 0 ? 8u : (value > 0 ? 4u : 2u);
      cr0 |= (cpu->xer >> 31) & 1u;
      cpu->cr = (cpu->cr & 0x0fffffffu) | (cr0 << 28);
      break;
    }
    case 18: { /* b */
      uint32_t disp = insn & 0x03fffffcu;
      int32_t offset = (int32_t)((disp ^ 0x02000000u) - 0x02000000u);
      uint32_t target = (insn & 2u) ? (uint32_t)offset : cpu->pc + (uint32_t)offset;
      if (insn & 1u) cpu->lr = next_pc;
      next_pc = target;
      break;
    }
    case 32: /* lwz */
    case 34: /* lbz */
    case 36: /* stw */
    case 38: { /* stb */
      uint32_t ea = (ra ? cpu->gpr[ra] : 0u) + (uint32_t)(int32_t)(int16_t)imm;
      if (!memory) return PPCVM_MEMORY_FAULT;
      ppcvm_mem_result status;
      uint32_t word;
      uint8_t byte;
      if ((opcode == 32 || opcode == 36) && (ea & 3u)) return PPCVM_MEMORY_FAULT;
      if (opcode == 32) {
        status = ppcvm_memory_read32be(memory, ea, &word);
        if (status != PPCVM_MEM_OK) return PPCVM_MEMORY_FAULT;
        cpu->gpr[rt] = word;
      } else if (opcode == 34) {
        status = ppcvm_memory_read8(memory, ea, &byte);
        if (status != PPCVM_MEM_OK) return PPCVM_MEMORY_FAULT;
        cpu->gpr[rt] = byte;
      } else if (opcode == 36) {
        status = ppcvm_memory_write32be(memory, ea, cpu->gpr[rt]);
        if (status != PPCVM_MEM_OK) return PPCVM_MEMORY_FAULT;
      } else {
        status = ppcvm_memory_write8(memory, ea, (uint8_t)cpu->gpr[rt]);
        if (status != PPCVM_MEM_OK) return PPCVM_MEMORY_FAULT;
      }
      break;
    }
    default:
      return PPCVM_UNSUPPORTED;
  }
  cpu->pc = next_pc;
  return PPCVM_OK;
}
ppcvm_result ppcvm_cpu_step(ppcvm_cpu *cpu, uint32_t insn) {
  return ppcvm_cpu_step_memory(cpu, NULL, insn);
}
