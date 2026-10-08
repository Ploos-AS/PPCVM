#include "ppcvm/cpu.h"
#include <string.h>
static uint32_t ppcvm_rotl32(uint32_t value, unsigned shift) {
  shift &= 31u;
  return shift ? (value << shift) | (value >> (32u-shift)) : value;
}
static uint32_t ppcvm_mask32(unsigned mb, unsigned me) {
  uint32_t mask=0;
  for (unsigned bit=0; bit<32u; ++bit)
    if ((mb<=me && bit>=mb && bit<=me) ||
        (mb>me && (bit>=mb || bit<=me)))
      mask |= UINT32_C(0x80000000) >> bit;
  return mask;
}
/* Minimal synchronous system-call exception: low vectors only, no MMU. */
static void ppcvm_enter_syscall(ppcvm_cpu *cpu, uint32_t next_pc) {
  cpu->srr0=next_pc;
  cpu->srr1=cpu->msr;
  cpu->msr &= ~UINT32_C(0x0000c030); /* PR, EE, IR, DR */
  cpu->pc=UINT32_C(0x00000c00);
}
void ppcvm_cpu_reset(ppcvm_cpu *cpu) { memset(cpu, 0, sizeof(*cpu)); }
ppcvm_result ppcvm_cpu_step_memory(ppcvm_cpu *cpu, ppcvm_memory *memory, uint32_t insn) {
  uint32_t opcode = insn >> 26;
  uint32_t rt = (insn >> 21) & 31u;
  uint32_t ra = (insn >> 16) & 31u;
  uint32_t imm = insn & 0xffffu;
  uint32_t next_pc = cpu->pc + 4u;
  switch (opcode) {
    case 17: /* sc: only architecturally defined basic encoding */
      if (insn!=UINT32_C(0x44000002)) return PPCVM_UNSUPPORTED;
      ppcvm_enter_syscall(cpu,next_pc);
      return PPCVM_OK;
    case 14: /* addi */
      cpu->gpr[rt] = (ra ? cpu->gpr[ra] : 0u) + (uint32_t)(int32_t)(int16_t)imm;
      break;
    case 15: /* addis */
      cpu->gpr[rt] = (ra ? cpu->gpr[ra] : 0u) + ((uint32_t)((int32_t)(int16_t)imm * 65536));
      break;
    case 21: { /* rlwinm: rotate left word immediate then mask */
      unsigned sh=(insn>>11)&31u;
      unsigned mb=(insn>>6)&31u;
      unsigned me=(insn>>1)&31u;
      uint32_t value=ppcvm_rotl32(cpu->gpr[rt],sh) & ppcvm_mask32(mb,me);
      cpu->gpr[ra]=value;
      if (insn&1u) {
        uint32_t bits=(int32_t)value<0 ? 8u : (value ? 4u : 2u);
        bits|=(cpu->xer>>31)&1u;
        cpu->cr=(cpu->cr&UINT32_C(0x0fffffff))|(bits<<28);
      }
      break;
    }
    case 24: /* ori */
      cpu->gpr[ra] = cpu->gpr[rt] | imm;
      break;
    case 25: /* oris */
      cpu->gpr[ra] = cpu->gpr[rt] | (imm << 16);
      break;
    case 26: /* xori */
      cpu->gpr[ra] = cpu->gpr[rt] ^ imm;
      break;
    case 27: /* xoris */
      cpu->gpr[ra] = cpu->gpr[rt] ^ (imm << 16);
      break;
    case 28: /* andi. */
    case 29: { /* andis. */
      cpu->gpr[ra] = cpu->gpr[rt] & (opcode==29 ? (imm << 16) : imm);
      int32_t value = (int32_t)cpu->gpr[ra];
      uint32_t cr0 = value < 0 ? 8u : (value > 0 ? 4u : 2u);
      cr0 |= (cpu->xer >> 31) & 1u;
      cpu->cr = (cpu->cr & 0x0fffffffu) | (cr0 << 28);
      break;
    }
    case 11: /* cmpi: signed compare */
    case 10: { /* cmpli: unsigned compare */
      uint32_t bf=(insn>>23)&7u;
      uint32_t field=(insn>>21)&3u;
      if (field != 0u) return PPCVM_UNSUPPORTED; /* 32-bit only, reserved bit clear */
      uint32_t a=cpu->gpr[ra];
      uint32_t b=opcode==11 ? (uint32_t)(int32_t)(int16_t)imm : imm;
      uint32_t bits;
      if (opcode==11) {
        int32_t sa=(int32_t)a, sb=(int32_t)b;
        bits=sa<sb ? 8u : (sa>sb ? 4u : 2u);
      } else {
        bits=a<b ? 8u : (a>b ? 4u : 2u);
      }
      bits|=(cpu->xer>>31)&1u;
      uint32_t shift=28u-4u*bf;
      cpu->cr=(cpu->cr & ~(15u<<shift)) | (bits<<shift);
      break;
    }
    case 16: { /* bc: conditional branch */
      uint32_t bo=(insn>>21)&31u;
      uint32_t bi=(insn>>16)&31u;
      uint32_t bd=insn&0xfffcu;
      int32_t offset=(int32_t)((bd^0x8000u)-0x8000u);
      if ((bo & 4u)==0u) cpu->ctr--;
      int ctr_ok=(bo&4u)!=0u || ((cpu->ctr!=0u) != ((bo&2u)!=0u));
      int cr_bit=(int)((cpu->cr>>(31u-bi))&1u);
      int cond_ok=(bo&16u)!=0u || (cr_bit==((bo&8u)!=0u));
      if (insn&1u) cpu->lr=next_pc;
      if (ctr_ok && cond_ok) next_pc=(insn&2u) ? (uint32_t)offset : cpu->pc+(uint32_t)offset;
      break;
    }
    case 19: { /* bclr / bcctr: indirect branches */
      uint32_t xo=(insn>>1)&1023u;
      uint32_t bo=(insn>>21)&31u;
      uint32_t bi=(insn>>16)&31u;
      if (xo==50u) { /* rfi: privileged exception return, minimal 32-bit model */
        if (insn!=UINT32_C(0x4c000064) || (cpu->msr&UINT32_C(0x4000))) return PPCVM_UNSUPPORTED;
        cpu->msr=cpu->srr1;
        next_pc=cpu->srr0 & ~3u;
        break;
      }
      if ((xo!=16u && xo!=528u) || (insn&0x0000e000u)!=0u) return PPCVM_UNSUPPORTED;
      if (xo==528u && (bo&4u)==0u) return PPCVM_UNSUPPORTED; /* bcctr must not decrement CTR */
      uint32_t target=(xo==16u ? cpu->lr : cpu->ctr) & ~3u;
      if (xo==16u && (bo&4u)==0u) cpu->ctr--;
      int ctr_ok=xo==528u || (bo&4u)!=0u || ((cpu->ctr!=0u) != ((bo&2u)!=0u));
      int cr_bit=(int)((cpu->cr>>(31u-bi))&1u);
      int cond_ok=(bo&16u)!=0u || (cr_bit==((bo&8u)!=0u));
      if (insn&1u) cpu->lr=next_pc;
      if (ctr_ok && cond_ok) next_pc=target;
      break;
    }
    case 31: { /* selected XFX-form special register moves */
      uint32_t xo=(insn>>1)&1023u;
      uint32_t spr=((insn>>16)&31u)|(((insn>>11)&31u)<<5);
      if (xo==266u || xo==40u || xo==444u || xo==316u || xo==28u) {
        /* X-form arithmetic and logic. These variants do not set XER overflow. */
        uint32_t rb=(insn>>11)&31u;
        uint32_t value;
        if ((insn&0x400u)!=0u) return PPCVM_UNSUPPORTED; /* OE must be zero */
        if (xo==266u) value=cpu->gpr[ra]+cpu->gpr[rb]; /* add */
        else if (xo==40u) value=cpu->gpr[rb]-cpu->gpr[ra]; /* subf */
        else if (xo==444u) value=cpu->gpr[rt]|cpu->gpr[rb]; /* or */
        else if (xo==316u) value=cpu->gpr[rt]^cpu->gpr[rb]; /* xor */
        else value=cpu->gpr[rt]&cpu->gpr[rb]; /* and */
        if (xo==266u || xo==40u) cpu->gpr[rt]=value;
        else cpu->gpr[ra]=value;
        if (insn&1u) {
          uint32_t bits=(int32_t)value<0 ? 8u : (value ? 4u : 2u);
          bits|=(cpu->xer>>31)&1u;
          cpu->cr=(cpu->cr&UINT32_C(0x0fffffff))|(bits<<28);
        }
        break;
      }
      if (xo!=339u && xo!=467u) return PPCVM_UNSUPPORTED;
      if (spr!=8u && spr!=9u && spr!=26u && spr!=27u) return PPCVM_UNSUPPORTED; /* LR CTR SRR0 SRR1 */
      if (xo==339u) {
        cpu->gpr[rt]=spr==8u ? cpu->lr : (spr==9u ? cpu->ctr : (spr==26u ? cpu->srr0 : cpu->srr1));
      } else if (spr==8u) cpu->lr=cpu->gpr[rt];
      else if (spr==9u) cpu->ctr=cpu->gpr[rt];
      else if (spr==26u) cpu->srr0=cpu->gpr[rt];
      else cpu->srr1=cpu->gpr[rt];
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
