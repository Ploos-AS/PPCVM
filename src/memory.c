#include "ppcvm/memory.h"
#include <stdlib.h>
static int valid(const ppcvm_memory *m, uint32_t addr, size_t len) {
  return m && m->data && (size_t)addr <= m->size && len <= m->size - (size_t)addr;
}
ppcvm_mem_result ppcvm_memory_init(ppcvm_memory *m, size_t size) {
  if (!m || !size) return PPCVM_MEM_ARGUMENT;
  m->data = NULL;
  m->size = 0;
  m->data = calloc(size, 1);
  if (!m->data) return PPCVM_MEM_ARGUMENT;
  m->size = size;
  return PPCVM_MEM_OK;
}
void ppcvm_memory_free(ppcvm_memory *m) {
  if (!m) return;
  free(m->data);
  m->data = NULL;
  m->size = 0;
}
ppcvm_mem_result ppcvm_memory_read8(const ppcvm_memory *m, uint32_t addr, uint8_t *out) {
  if (!out) return PPCVM_MEM_ARGUMENT;
  if (!valid(m, addr, 1)) return PPCVM_MEM_RANGE;
  *out = m->data[addr];
  return PPCVM_MEM_OK;
}
ppcvm_mem_result ppcvm_memory_write8(ppcvm_memory *m, uint32_t addr, uint8_t value) {
  if (!valid(m, addr, 1)) return PPCVM_MEM_RANGE;
  m->data[addr] = value;
  return PPCVM_MEM_OK;
}
ppcvm_mem_result ppcvm_memory_read32be(const ppcvm_memory *m, uint32_t addr, uint32_t *out) {
  if (!out) return PPCVM_MEM_ARGUMENT;
  if (!valid(m, addr, 4)) return PPCVM_MEM_RANGE;
  const uint8_t *p = m->data + addr;
  *out = ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
  return PPCVM_MEM_OK;
}
ppcvm_mem_result ppcvm_memory_write32be(ppcvm_memory *m, uint32_t addr, uint32_t value) {
  if (!valid(m, addr, 4)) return PPCVM_MEM_RANGE;
  uint8_t *p = m->data + addr;
  p[0] = (uint8_t)(value>>24); p[1] = (uint8_t)(value>>16);
  p[2] = (uint8_t)(value>>8); p[3] = (uint8_t)value;
  return PPCVM_MEM_OK;
}
