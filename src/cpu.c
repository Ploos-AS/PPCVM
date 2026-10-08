#include "ppcvm/cpu.h"
#include <string.h>
void ppcvm_cpu_reset(ppcvm_cpu *cpu) { memset(cpu, 0, sizeof(*cpu)); }
ppcvm_result ppcvm_cpu_step(ppcvm_cpu *cpu, uint32_t insn) {
  uint32_t opcode = insn >> 26;
  uint32_t rt = (insn >> 21) & 31u;
  uint32_t ra = (insn >> 16) & 31u;
  uint32_t imm = insn & 0xffffu;
  uint32_t result;
  switch (opcode) {
    case 14: /* addi: RA=0 denotes literal zero */
      result = (ra ? cpu->gpr[ra] : 0u) + (uint32_t)(int32_t)(int16_t)imm;
      cpu->gpr[rt] = result;
      break;
    case 24: /* ori */
      cpu->gpr[ra] = cpu->gpr[rt] | imm;
      break;
    default:
      return PPCVM_UNSUPPORTED;
  }
  cpu->pc += 4;
  return PPCVM_OK;
}
