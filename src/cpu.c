#include "ppcvm/cpu.h"
#include <string.h>
void ppcvm_cpu_reset(ppcvm_cpu *cpu) { memset(cpu, 0, sizeof(*cpu)); }
ppcvm_result ppcvm_cpu_step(ppcvm_cpu *cpu, uint32_t insn) {
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
    case 24: /* ori: RS is in RT field */
      cpu->gpr[ra] = cpu->gpr[rt] | imm;
      break;
    case 25: /* oris */
      cpu->gpr[ra] = cpu->gpr[rt] | (imm << 16);
      break;
    case 26: /* xori */
      cpu->gpr[ra] = cpu->gpr[rt] ^ imm;
      break;
    case 28: /* andi.: record CR0 */
      cpu->gpr[ra] = cpu->gpr[rt] & imm;
      {
        int32_t signed_result = (int32_t)cpu->gpr[ra];
        uint32_t cr0 = signed_result < 0 ? 8u : (signed_result > 0 ? 4u : 2u);
        cr0 |= (cpu->xer >> 31) & 1u;
        cpu->cr = (cpu->cr & 0x0fffffffu) | (cr0 << 28);
      }
      break;
    case 18: { /* b: displacement is sign extended from 26 bits */
      uint32_t disp = insn & 0x03fffffcu;
      int32_t offset = (int32_t)((disp ^ 0x02000000u) - 0x02000000u);
      uint32_t target = (insn & 2u) ? (uint32_t)offset : cpu->pc + (uint32_t)offset;
      if (insn & 1u) cpu->lr = next_pc;
      next_pc = target;
      break;
    }
    default:
      return PPCVM_UNSUPPORTED;
  }
  cpu->pc = next_pc;
  return PPCVM_OK;
}
