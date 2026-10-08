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
ppcvm_bus_result ppcvm_pegasos2_map_high_rom(ppcvm_pegasos2 *m, uint8_t *bytes, uint32_t size) {
  if (!m || !bytes || size < UINT32_C(0x1000) || size > UINT32_C(0x100000)) return PPCVM_BUS_INVALID;
  return ppcvm_bus_map_memory(&m->bus,UINT32_C(0xfff00000),size,bytes,1);
}
void ppcvm_pegasos2_reset(ppcvm_pegasos2 *m) {
  if (!m) return;
  ppcvm_cpu_reset(&m->cpu);
  m->discovery_scratch=0;
  m->discovery_reads=0;
  m->discovery_writes=0;
}
void ppcvm_pegasos2_cold_reset(ppcvm_pegasos2 *m) {
  if (!m) return;
  ppcvm_pegasos2_reset(m);
  if (m->ram.data) memset(m->ram.data,0,m->ram.size);
  memset(&m->bat,0,sizeof(m->bat));
  memset(&m->segments,0,sizeof(m->segments));
}
ppcvm_result ppcvm_pegasos2_boot_high_rom(ppcvm_pegasos2 *m, uint32_t entry) {
  uint32_t instruction=0;
  if (!m || (entry & 3u) || entry < UINT32_C(0xfff00000) ||
      ppcvm_bus_read32be(&m->bus,entry,&instruction)!=PPCVM_BUS_OK)
    return PPCVM_MEMORY_FAULT;
  /* This is an explicit test boot entry, not a hardware reset-vector model. */
  ppcvm_cpu_reset(&m->cpu);
  m->cpu.pc=entry;
  m->cpu.msr=UINT32_C(0x40); /* high exception prefix */
  return PPCVM_OK;
}
ppcvm_result ppcvm_pegasos2_cold_boot_high_rom(ppcvm_pegasos2 *m, uint32_t entry) {
  uint32_t instruction=0;
  if (!m || (entry & 3u) || entry < UINT32_C(0xfff00000) ||
      ppcvm_bus_read32be(&m->bus,entry,&instruction)!=PPCVM_BUS_OK)
    return PPCVM_MEMORY_FAULT;
  ppcvm_pegasos2_cold_reset(m);
  m->cpu.pc=entry;
  m->cpu.msr=UINT32_C(0x40);
  return PPCVM_OK;
}
/* ELF32/PowerPC big-endian ET_EXEC loader; preflight all segments before writing. */
static uint16_t elf16(const uint8_t *p) { return (uint16_t)(((uint16_t)p[0]<<8)|p[1]); }
static uint32_t elf32(const uint8_t *p) {
  return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
ppcvm_result ppcvm_pegasos2_load_elf32(ppcvm_pegasos2 *m, const uint8_t *image,
                                        size_t size, uint32_t *entry) {
  if (!m || !m->ram.data || !image || !entry || size<52u ||
      image[0]!=0x7fu || image[1]!='E' || image[2]!='L' || image[3]!='F' ||
      image[4]!=1u || image[5]!=2u || image[6]!=1u ||
      elf16(image+16)!=2u || elf16(image+18)!=20u ||
      elf32(image+20)!=1u || elf16(image+40)!=52u ||
      elf16(image+42)!=32u) return PPCVM_UNSUPPORTED;
  uint32_t phoff=elf32(image+28), start=elf32(image+24);
  uint16_t count=elf16(image+44);
  if (!count || (size_t)phoff>size || (size_t)count>(size-(size_t)phoff)/32u ||
      (start&3u) || (size_t)start>m->ram.size ||
      m->ram.size-(size_t)start<4u) return PPCVM_MEMORY_FAULT;
  /* The image must not alias writable guest RAM: segment copies can otherwise
     overwrite program headers or later segment payloads during loading. */
  {
    uintptr_t src=(uintptr_t)image, dst=(uintptr_t)m->ram.data;
    if ((src>=dst && src-dst<m->ram.size) ||
        (dst>src && dst-src<size)) return PPCVM_UNSUPPORTED;
  }
  unsigned loaded=0, entry_ok=0;
  for (unsigned i=0;i<count;i++) {
    const uint8_t *ph=image+(size_t)phoff+(size_t)i*32u;
    if (elf32(ph)!=1u) continue;
    uint32_t offset=elf32(ph+4), address=elf32(ph+12);
    uint32_t filesz=elf32(ph+16), memsz=elf32(ph+20);
    uint32_t flags=elf32(ph+24), align=elf32(ph+28);
    if ((flags & ~UINT32_C(7)) || (align && (align & (align-1u))) ||
        (align>1u && ((offset ^ address) & (align-1u)))) return PPCVM_UNSUPPORTED;
    if (filesz>memsz || (size_t)offset>size ||
        (size_t)filesz>size-(size_t)offset ||
        (size_t)address>m->ram.size ||
        (size_t)memsz>m->ram.size-(size_t)address) return PPCVM_MEMORY_FAULT;
    /* Entry must point at file-backed instructions in an executable segment. */
    if ((flags & UINT32_C(1)) && start>=address &&
        (uint64_t)start+4u<=(uint64_t)address+filesz)
      entry_ok=1;
    loaded++;
    for (unsigned j=0;j<i;j++) {
      const uint8_t *prev=image+(size_t)phoff+(size_t)j*32u;
      if (elf32(prev)!=1u) continue;
      uint32_t a=elf32(prev+12), n=elf32(prev+20);
      if (memsz && n && (uint64_t)address<(uint64_t)a+n &&
          (uint64_t)a<(uint64_t)address+memsz) return PPCVM_MEMORY_FAULT;
    }
  }
  if (!loaded || !entry_ok) return PPCVM_MEMORY_FAULT;
  for (unsigned i=0;i<count;i++) {
    const uint8_t *ph=image+(size_t)phoff+(size_t)i*32u;
    if (elf32(ph)!=1u) continue;
    uint32_t offset=elf32(ph+4), address=elf32(ph+12);
    uint32_t filesz=elf32(ph+16), memsz=elf32(ph+20);
    memmove(m->ram.data+address,image+offset,filesz);
    memset(m->ram.data+address+filesz,0,memsz-filesz);
  }
  *entry=start;
  return PPCVM_OK;
}
ppcvm_result ppcvm_pegasos2_boot_elf32_abi(ppcvm_pegasos2 *m,
    const uint8_t *image, size_t size, uint32_t info_address) {
  uint32_t entry=0;
  if (!m || !m->ram.data || (info_address & 3u) ||
      (uint64_t)info_address+PPCVM_PEGASOS2_BOOT_INFO_SIZE>m->ram.size ||
      m->ram.size>UINT32_MAX) return PPCVM_MEMORY_FAULT;
  /* Reject any boot record overlap with a loadable ELF segment before loading. */
  if (!image || size<52u) return PPCVM_UNSUPPORTED;
  if (image[0]!=0x7fu || image[1]!='E' || image[2]!='L' ||
      image[3]!='F' || image[4]!=1u || image[5]!=2u) return PPCVM_UNSUPPORTED;
  {
    uint32_t phoff=elf32(image+28u);
    uint16_t phnum=elf16(image+44u), entsize=elf16(image+42u);
    if (entsize!=32u || (uint64_t)phoff+(uint64_t)phnum*32u>size)
      return PPCVM_MEMORY_FAULT;
    for (uint16_t i=0;i<phnum;i++) {
      const uint8_t *ph=image+phoff+(size_t)i*32u;
      if (elf32(ph)!=1u) continue;
      uint64_t base=elf32(ph+12u), end=base+elf32(ph+20u);
      if (base<(uint64_t)info_address+PPCVM_PEGASOS2_BOOT_INFO_SIZE &&
          end>info_address) return PPCVM_MEMORY_FAULT;
    }
  }
  ppcvm_result result=ppcvm_pegasos2_load_elf32(m,image,size,&entry);
  if (result!=PPCVM_OK) return result;
  ppcvm_cpu_reset(&m->cpu);
  m->cpu.pc=entry;
  m->cpu.gpr[3]=PPCVM_PEGASOS2_BOOT_MAGIC;
  m->cpu.gpr[4]=info_address;
  m->cpu.gpr[5]=(uint32_t)m->ram.size;
  m->cpu.gpr[6]=entry;
  /* A small BE boot record for prototype guests; no Open Firmware claims. */
  ppcvm_memory_write32be(&m->ram,info_address,PPCVM_PEGASOS2_BOOT_MAGIC);
  ppcvm_memory_write32be(&m->ram,info_address+4u,PPCVM_PEGASOS2_BOOT_INFO_SIZE);
  ppcvm_memory_write32be(&m->ram,info_address+8u,(uint32_t)m->ram.size);
  ppcvm_memory_write32be(&m->ram,info_address+12u,entry);
  return PPCVM_OK;
}
ppcvm_result ppcvm_pegasos2_boot_elf32(ppcvm_pegasos2 *m,
                                        const uint8_t *image, size_t size) {
  uint32_t entry=0;
  ppcvm_result result=ppcvm_pegasos2_load_elf32(m,image,size,&entry);
  if (result!=PPCVM_OK) return result;
  /* Loader already verified a file-backed executable entry in RAM. */
  return ppcvm_pegasos2_enter_ram(m,entry);
}
ppcvm_result ppcvm_pegasos2_load_raw(ppcvm_pegasos2 *m, uint32_t address,
                                    const uint8_t *bytes, size_t size) {
  if (!m || !bytes || !size || !m->ram.data || (size_t)address > m->ram.size ||
      size > m->ram.size-(size_t)address) return PPCVM_MEMORY_FAULT;
  /* Reject aliases into guest RAM itself; use memmove to support overlap. */
  memmove(m->ram.data+(size_t)address,bytes,size);
  return PPCVM_OK;
}
ppcvm_result ppcvm_pegasos2_enter_ram(ppcvm_pegasos2 *m, uint32_t entry) {
  uint32_t instruction=0;
  if (!m || (entry&3u) || !m->ram.data || (size_t)entry > m->ram.size ||
      m->ram.size-(size_t)entry < 4u ||
      ppcvm_bus_read32be(&m->bus,entry,&instruction)!=PPCVM_BUS_OK)
    return PPCVM_MEMORY_FAULT;
  m->cpu.pc=entry;
  return PPCVM_OK;
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
static ppcvm_mmu_result translate_step(ppcvm_pegasos2 *m, int use_pte, int keyed,
                                        uint32_t ea, ppcvm_access access,
                                        uint32_t *pa) {
  if (use_pte && keyed) {
    ppcvm_mmu_result result=ppcvm_mmu_translate_bat(&m->bat,m->cpu.msr,ea,access,pa);
    if (result!=PPCVM_MMU_UNSUPPORTED) return result;
    return ppcvm_mmu_lookup_pte_keyed(&m->segments,&m->ram,m->cpu.msr,ea,access,pa);
  }
  if (use_pte)
    return ppcvm_mmu_translate_combined(&m->bat,&m->segments,&m->ram,
                                        m->cpu.msr,ea,access,pa);
  return ppcvm_mmu_translate_bat(&m->bat,m->cpu.msr,ea,access,pa);
}
static ppcvm_mmu_result update_pte_rc(ppcvm_pegasos2 *m, int keyed,
                                        uint32_t ea, ppcvm_access access,
                                        uint32_t *pa) {
  if (keyed) return ppcvm_mmu_lookup_pte_rc_keyed(&m->segments,&m->ram,m->cpu.msr,ea,access,pa);
  return ppcvm_mmu_lookup_pte_rc(&m->segments,&m->ram,ea,access,pa);
}
static ppcvm_result step_translated(ppcvm_pegasos2 *m, int use_pte, int keyed) {
  if (!m || (m->cpu.pc&3u)) return PPCVM_MEMORY_FAULT;
  uint32_t physical=0, instruction=0;
  if (translate_step(m,use_pte,keyed,m->cpu.pc,PPCVM_ACCESS_INSTRUCTION,&physical)!=PPCVM_MMU_OK ||
      ppcvm_bus_read32be(&m->bus,physical,&instruction)!=PPCVM_BUS_OK) {
    ppcvm_cpu_enter_exception(&m->cpu,PPCVM_VECTOR_ISI,m->cpu.pc);
    return PPCVM_OK;
  }
  if (use_pte && (m->cpu.msr&UINT32_C(0x20))) {
    uint32_t bat_pa=0;
    if (ppcvm_mmu_translate_bat(&m->bat,m->cpu.msr,m->cpu.pc,
                                PPCVM_ACCESS_INSTRUCTION,&bat_pa)==PPCVM_MMU_UNSUPPORTED) {
      uint32_t rc_pa=0;
      if (update_pte_rc(m,keyed,m->cpu.pc,
                                  PPCVM_ACCESS_INSTRUCTION,&rc_pa)!=PPCVM_MMU_OK ||
          rc_pa!=physical) return PPCVM_MEMORY_FAULT;
    }
  }
  uint32_t op=instruction>>26;
  if (op==31u) {
    uint32_t xo=(instruction>>1)&1023u;
    if (xo==595u || xo==210u)
      return segment_spr_step(m,instruction);
    uint32_t spr=((instruction>>16)&31u)|(((instruction>>11)&31u)<<5);
    if ((xo==339u || xo==467u) && spr==25u) {
      if (instruction&1u) return PPCVM_UNSUPPORTED;
      if (m->cpu.msr&UINT32_C(0x4000)) {
        ppcvm_cpu_enter_exception(&m->cpu,PPCVM_VECTOR_PROGRAM,m->cpu.pc);
        m->cpu.srr1|=UINT32_C(0x00040000);
        return PPCVM_OK;
      }
      unsigned rt=(instruction>>21)&31u;
      if (xo==339u) m->cpu.gpr[rt]=m->segments.sdr1;
      else m->segments.sdr1=m->cpu.gpr[rt];
      m->cpu.pc+=4;
      return PPCVM_OK;
    }
    if ((xo==339u || xo==467u) && spr>=528u && spr<=543u)
      return bat_spr_step(m,instruction);
  }
  if (op!=32 && op!=34 && op!=36 && op!=38)
    return ppcvm_cpu_step(&m->cpu,instruction);
  uint32_t rt=(instruction>>21)&31u, ra=(instruction>>16)&31u;
  uint32_t ea=(ra?m->cpu.gpr[ra]:0u)+(uint32_t)(int32_t)(int16_t)(instruction&0xffffu);
  if ((op==32 || op==36) && (ea&3u)) return PPCVM_MEMORY_FAULT;
  ppcvm_access access=(op==36 || op==38)?PPCVM_ACCESS_DATA_WRITE:PPCVM_ACCESS_DATA_READ;
  ppcvm_mmu_result translation=translate_step(m,use_pte,keyed,ea,access,&physical);
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
  if (use_pte && (m->cpu.msr&UINT32_C(0x10))) {
    uint32_t bat_pa=0;
    if (ppcvm_mmu_translate_bat(&m->bat,m->cpu.msr,ea,access,&bat_pa)==PPCVM_MMU_UNSUPPORTED) {
      uint32_t rc_pa=0;
      if (update_pte_rc(m,keyed,ea,access,&rc_pa)!=PPCVM_MMU_OK ||
          rc_pa!=physical) return PPCVM_MEMORY_FAULT;
    }
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

ppcvm_result ppcvm_pegasos2_step_bat(ppcvm_pegasos2 *m) {
  return step_translated(m,0,0);
}
ppcvm_result ppcvm_pegasos2_step_pte(ppcvm_pegasos2 *m) {
  return step_translated(m,1,0);
}

ppcvm_result ppcvm_pegasos2_step_pte_keyed(ppcvm_pegasos2 *m) {
  return step_translated(m,1,1);
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
