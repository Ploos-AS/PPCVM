#include "ppcvm/pegasos2.h"
#include <string.h>
/* Scratch register is a synthetic diagnostic placeholder, not a Discovery II register. */
static ppcvm_bus_result discovery_read(void *context, uint32_t offset, uint32_t *value) {
  ppcvm_pegasos2 *m=(ppcvm_pegasos2 *)context;
  if (offset != 0 || !value) return PPCVM_BUS_INVALID;
  m->discovery_reads++;
  *value=m->discovery_scratch;
  return PPCVM_BUS_OK;
}
static ppcvm_bus_result discovery_write(void *context, uint32_t offset, uint32_t value) {
  ppcvm_pegasos2 *m=(ppcvm_pegasos2 *)context;
  if (offset != 0) return PPCVM_BUS_INVALID;
  m->discovery_writes++;
  m->discovery_scratch=value;
  return PPCVM_BUS_OK;
}
int ppcvm_pegasos2_init(ppcvm_pegasos2 *m, size_t ram_size) {
  if (!m || !ram_size || ram_size > UINT32_C(0xf0000000)) return -1;
  memset(m,0,sizeof(*m));
  ppcvm_cpu_reset(&m->cpu);
  ppcvm_bus_init(&m->bus);
  if (ppcvm_memory_init(&m->ram,ram_size) != PPCVM_MEM_OK) return -1;
  if (ppcvm_bus_map_memory(&m->bus,0,(uint32_t)ram_size,m->ram.data,0) != PPCVM_BUS_OK ||
      ppcvm_bus_map_mmio32(&m->bus,PPCVM_PEGASOS2_DISCOVERY_BASE,
                           PPCVM_PEGASOS2_DISCOVERY_SIZE,m,discovery_read,discovery_write) != PPCVM_BUS_OK) {
    ppcvm_pegasos2_destroy(m);
    return -1;
  }
  return 0;
}
void ppcvm_pegasos2_destroy(ppcvm_pegasos2 *m) {
  if (!m) return;
  ppcvm_memory_free(&m->ram);
  ppcvm_bus_init(&m->bus);
}

ppcvm_result ppcvm_pegasos2_step(ppcvm_pegasos2 *m) {
  uint32_t instruction=0;
  if (!m || (m->cpu.pc & 3u)) return PPCVM_MEMORY_FAULT;
  if (ppcvm_bus_read32be(&m->bus,m->cpu.pc,&instruction) != PPCVM_BUS_OK)
    return PPCVM_MEMORY_FAULT;
  return ppcvm_cpu_step_bus(&m->cpu,&m->bus,instruction);
}

ppcvm_result ppcvm_pegasos2_step_isi(ppcvm_pegasos2 *m) {
  if (!m) return PPCVM_MEMORY_FAULT;
  uint32_t instruction=0;
  if ((m->cpu.pc&3u)!=0u) return PPCVM_MEMORY_FAULT; /* alignment is not ISI */
  if (ppcvm_bus_read32be(&m->bus,m->cpu.pc,&instruction)!=PPCVM_BUS_OK) {
    ppcvm_cpu_enter_exception(&m->cpu,PPCVM_VECTOR_ISI,m->cpu.pc);
    return PPCVM_OK;
  }
  return ppcvm_cpu_step_bus(&m->cpu,&m->bus,instruction);
}

ppcvm_result ppcvm_pegasos2_step_exceptions(ppcvm_pegasos2 *m) {
  if (!m) return PPCVM_MEMORY_FAULT;
  uint32_t instruction=0;
  if ((m->cpu.pc&3u)!=0u) return PPCVM_MEMORY_FAULT;
  if (ppcvm_bus_read32be(&m->bus,m->cpu.pc,&instruction)!=PPCVM_BUS_OK) {
    ppcvm_cpu_enter_exception(&m->cpu,PPCVM_VECTOR_ISI,m->cpu.pc);
    return PPCVM_OK;
  }
  return ppcvm_cpu_step_bus_dsi(&m->cpu,&m->bus,instruction);
}

ppcvm_result ppcvm_pegasos2_step_bat_fetch(ppcvm_pegasos2 *m) {
  if (!m) return PPCVM_MEMORY_FAULT;
  if ((m->cpu.pc&3u)!=0u) return PPCVM_MEMORY_FAULT;
  uint32_t physical=0, instruction=0;
  if (ppcvm_mmu_translate_bat(&m->bat,m->cpu.msr,m->cpu.pc,
                              PPCVM_ACCESS_INSTRUCTION,&physical)!=PPCVM_MMU_OK ||
      ppcvm_bus_read32be(&m->bus,physical,&instruction)!=PPCVM_BUS_OK) {
    ppcvm_cpu_enter_exception(&m->cpu,PPCVM_VECTOR_ISI,m->cpu.pc);
    return PPCVM_OK;
  }
  /* Data address translation will be integrated separately. */
  return ppcvm_cpu_step_bus_dsi(&m->cpu,&m->bus,instruction);
}

/* BAT SPR numbers: IBAT0U/L..IBAT3U/L = 528..535,
   DBAT0U/L..DBAT3U/L = 536..543. Supervisor-only. */
static ppcvm_result bat_spr_step(ppcvm_pegasos2 *m, uint32_t insn) {
  if ((insn>>26)!=31u) return PPCVM_UNSUPPORTED;
  uint32_t xo=(insn>>1)&1023u;
  if (xo!=339u && xo!=467u) return PPCVM_UNSUPPORTED;
  uint32_t spr=((insn>>16)&31u)|(((insn>>11)&31u)<<5);
  if (spr<528u || spr>543u) return PPCVM_UNSUPPORTED;
  if (insn&1u) return PPCVM_UNSUPPORTED;
  if (m->cpu.msr&UINT32_C(0x4000)) {
    ppcvm_cpu_enter_exception(&m->cpu,PPCVM_VECTOR_PROGRAM,m->cpu.pc);
    m->cpu.srr1|=UINT32_C(0x00040000); /* program exception: privileged instruction */
    return PPCVM_OK;
  }
  unsigned idx=(spr-528u)/2u;
  uint32_t *reg;
  if (idx<4u) reg=(spr&1u)?&m->bat.ibatl[idx]:&m->bat.ibatu[idx];
  else {
    idx-=4u;
    reg=(spr&1u)?&m->bat.dbatl[idx]:&m->bat.dbatu[idx];
  }
  unsigned rt=(insn>>21)&31u;
  if (xo==339u) m->cpu.gpr[rt]=*reg;
  else *reg=m->cpu.gpr[rt];
  m->cpu.pc+=4;
  return PPCVM_OK;
}
/* mfsr/mtsr operate on SR[0..15]; both are privileged. */
static ppcvm_result segment_spr_step(ppcvm_pegasos2 *m, uint32_t insn) {
  uint32_t xo=(insn>>1)&1023u;
  if (xo!=595u && xo!=210u) return PPCVM_UNSUPPORTED;
  if ((insn&1u) || ((insn>>20)&1u)) return PPCVM_UNSUPPORTED;
  if (m->cpu.msr&UINT32_C(0x4000)) {
    ppcvm_cpu_enter_exception(&m->cpu,PPCVM_VECTOR_PROGRAM,m->cpu.pc);
    m->cpu.srr1|=UINT32_C(0x00040000);
    return PPCVM_OK;
  }
  unsigned sr=(insn>>16)&15u;
  unsigned rt=(insn>>21)&31u;
  if (xo==595u) m->cpu.gpr[rt]=m->segments.sr[sr];
  else m->segments.sr[sr]=m->cpu.gpr[rt];
  m->cpu.pc+=4;
  return PPCVM_OK;
}
ppcvm_result ppcvm_pegasos2_step_bat(ppcvm_pegasos2 *m) {
  if (!m || (m->cpu.pc&3u)) return PPCVM_MEMORY_FAULT;
  uint32_t physical=0, instruction=0;
  if (ppcvm_mmu_translate_bat(&m->bat,m->cpu.msr,m->cpu.pc,
                              PPCVM_ACCESS_INSTRUCTION,&physical)!=PPCVM_MMU_OK ||
      ppcvm_bus_read32be(&m->bus,physical,&instruction)!=PPCVM_BUS_OK) {
    ppcvm_cpu_enter_exception(&m->cpu,PPCVM_VECTOR_ISI,m->cpu.pc);
    return PPCVM_OK;
  }
  uint32_t op=instruction>>26;
  if (op==31u) {
    uint32_t xo=(instruction>>1)&1023u;
    if (xo==595u || xo==210u)
      return segment_spr_step(m,instruction);
    uint32_t spr=((instruction>>16)&31u)|(((instruction>>11)&31u)<<5);
    if ((xo==339u || xo==467u) && spr>=528u && spr<=543u)
      return bat_spr_step(m,instruction);
  }
  if (op!=32 && op!=34 && op!=36 && op!=38)
    return ppcvm_cpu_step(&m->cpu,instruction);
  uint32_t rt=(instruction>>21)&31u, ra=(instruction>>16)&31u;
  uint32_t ea=(ra?m->cpu.gpr[ra]:0u)+(uint32_t)(int32_t)(int16_t)(instruction&0xffffu);
  if ((op==32 || op==36) && (ea&3u)) return PPCVM_MEMORY_FAULT;
  ppcvm_access access=(op==36 || op==38)?PPCVM_ACCESS_DATA_WRITE:PPCVM_ACCESS_DATA_READ;
  ppcvm_mmu_result translation=ppcvm_mmu_translate_bat(&m->bat,m->cpu.msr,ea,access,&physical);
  if (translation==PPCVM_MMU_PROTECTION) goto protection_fault;
  if (translation!=PPCVM_MMU_OK) goto fault;
  ppcvm_bus_result status;
  uint32_t word=0;
  uint8_t byte=0;
  if (op==32) {
    status=ppcvm_bus_read32be(&m->bus,physical,&word);
    if (status!=PPCVM_BUS_OK) goto fault;
    m->cpu.gpr[rt]=word;
  } else if (op==34) {
    status=ppcvm_bus_read8(&m->bus,physical,&byte);
    if (status!=PPCVM_BUS_OK) goto fault;
    m->cpu.gpr[rt]=byte;
  } else if (op==36) {
    status=ppcvm_bus_write32be(&m->bus,physical,m->cpu.gpr[rt]);
    if (status!=PPCVM_BUS_OK) goto fault;
  } else {
    status=ppcvm_bus_write8(&m->bus,physical,(uint8_t)m->cpu.gpr[rt]);
    if (status!=PPCVM_BUS_OK) goto fault;
  }
  m->cpu.pc+=4;
  return PPCVM_OK;
protection_fault:
  m->cpu.dar=ea;
  /* Protection violation: DSISR bit 27, plus store indicator bit 25. */
  m->cpu.dsisr=(access==PPCVM_ACCESS_DATA_WRITE)?UINT32_C(0x0a000000):UINT32_C(0x08000000);
  ppcvm_cpu_enter_exception(&m->cpu,PPCVM_VECTOR_DSI,m->cpu.pc);
  return PPCVM_OK;
fault:
  m->cpu.dar=ea;
  m->cpu.dsisr=(access==PPCVM_ACCESS_DATA_WRITE)?UINT32_C(0x42000000):UINT32_C(0x40000000);
  ppcvm_cpu_enter_exception(&m->cpu,PPCVM_VECTOR_DSI,m->cpu.pc);
  return PPCVM_OK;
}

ppcvm_result ppcvm_pegasos2_run(ppcvm_pegasos2 *m, size_t limit, size_t *executed) {
  if (executed) *executed=0;
  if (!m) return PPCVM_MEMORY_FAULT;
  for (size_t i=0; i<limit; ++i) {
    ppcvm_result result=ppcvm_pegasos2_step(m);
    if (result != PPCVM_OK) return result;
    if (executed) *executed=i+1;
  }
  return PPCVM_OK;
}
