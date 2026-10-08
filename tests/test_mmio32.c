#include "ppcvm/cpu.h"
#include <assert.h>
#include <stdint.h>
typedef struct { uint32_t reg; unsigned reads; unsigned writes; } device;
static ppcvm_bus_result rd(void *ctx, uint32_t offset, uint32_t *value) {
  device *d=(device *)ctx;
  if (offset != 0) return PPCVM_BUS_INVALID;
  d->reads++; *value=d->reg; return PPCVM_BUS_OK;
}
static ppcvm_bus_result wr(void *ctx, uint32_t offset, uint32_t value) {
  device *d=(device *)ctx;
  if (offset != 0) return PPCVM_BUS_INVALID;
  d->writes++; d->reg=value; return PPCVM_BUS_OK;
}
#define D(op,rt,ra,disp) (((uint32_t)(op)<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(disp)&0xffffu))
int main(void) {
  ppcvm_bus b;
  ppcvm_cpu c;
  device d={0};
  uint32_t word=0;
  ppcvm_bus_init(&b); ppcvm_cpu_reset(&c);
  assert(ppcvm_bus_map_mmio32(&b,0x1000,16,&d,rd,wr)==PPCVM_BUS_OK);
  assert(ppcvm_bus_write32be(&b,0x1000,0x12345678u)==PPCVM_BUS_OK);
  assert(ppcvm_bus_read32be(&b,0x1000,&word)==PPCVM_BUS_OK && word==0x12345678u);
  assert(d.reads==1 && d.writes==1);
  assert(ppcvm_bus_write8(&b,0x1000,1)==PPCVM_BUS_INVALID);
  assert(ppcvm_bus_read32be(&b,0x1004,&word)==PPCVM_BUS_INVALID);
  c.gpr[1]=0x1000; c.gpr[3]=0xaabbccddu;
  assert(ppcvm_cpu_step_bus(&c,&b,D(36,3,1,0))==PPCVM_OK);
  assert(ppcvm_cpu_step_bus(&c,&b,D(32,4,1,0))==PPCVM_OK);
  assert(c.gpr[4]==0xaabbccddu && d.reads==2 && d.writes==2);
  return 0;
}
