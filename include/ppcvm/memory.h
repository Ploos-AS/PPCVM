#ifndef PPCVM_MEMORY_H
#define PPCVM_MEMORY_H
#include <stddef.h>
#include <stdint.h>
typedef struct { uint8_t *data; size_t size; } ppcvm_memory;
typedef enum { PPCVM_MEM_OK=0, PPCVM_MEM_RANGE=1, PPCVM_MEM_ARGUMENT=2 } ppcvm_mem_result;
ppcvm_mem_result ppcvm_memory_init(ppcvm_memory *m, size_t size);
void ppcvm_memory_free(ppcvm_memory *m);
ppcvm_mem_result ppcvm_memory_read8(const ppcvm_memory *m, uint32_t addr, uint8_t *out);
ppcvm_mem_result ppcvm_memory_write8(ppcvm_memory *m, uint32_t addr, uint8_t value);
ppcvm_mem_result ppcvm_memory_read32be(const ppcvm_memory *m, uint32_t addr, uint32_t *out);
ppcvm_mem_result ppcvm_memory_write32be(ppcvm_memory *m, uint32_t addr, uint32_t value);
#endif
