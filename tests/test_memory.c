#include "ppcvm/memory.h"
#include <assert.h>
int main(void) {
  ppcvm_memory m = {0};
  uint32_t word = 0;
  uint8_t byte = 0;
  assert(ppcvm_memory_init(&m, 16) == PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m, 4, 0x12345678u) == PPCVM_MEM_OK);
  assert(ppcvm_memory_read32be(&m, 4, &word) == PPCVM_MEM_OK && word == 0x12345678u);
  assert(ppcvm_memory_read8(&m, 4, &byte) == PPCVM_MEM_OK && byte == 0x12u);
  assert(ppcvm_memory_write8(&m, 7, 0xabu) == PPCVM_MEM_OK);
  assert(ppcvm_memory_read32be(&m, 4, &word) == PPCVM_MEM_OK && word == 0x123456abu);
  assert(ppcvm_memory_write32be(&m, 13, 1u) == PPCVM_MEM_RANGE);
  assert(ppcvm_memory_read32be(&m, 0xffffffffu, &word) == PPCVM_MEM_RANGE);
  ppcvm_memory_free(&m);
  assert(m.data == 0 && m.size == 0);
  return 0;
}
