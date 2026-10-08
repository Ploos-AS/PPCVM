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
