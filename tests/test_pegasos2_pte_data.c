#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
static void map(ppcvm_pegasos2 *m,uint32_t ea,uint32_t frame,uint32_t pp) {
  uint32_t pteg=0;
  assert(ppcvm_mmu_pteg_address(&m->segments,ea,0,&pteg)==PPCVM_MMU_OK);
  uint32_t pte0=UINT32_C(0x80000000)|(UINT32_C(0x123)<<7)|((ea>>22)&63u);
  assert(ppcvm_memory_write32be(&m->ram,pteg,pte0)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m->ram,pteg+4,frame|pp)==PPCVM_MEM_OK);
}
static void setup(ppcvm_pegasos2 *m,uint32_t instruction,uint32_t pp) {
  assert(ppcvm_pegasos2_init(m,131072)==0);
  m->segments.sr[0]=UINT32_C(0x123);
  /* Fetch through real mode; data through PTE. */
  assert(ppcvm_memory_write32be(&m->ram,0,instruction)==PPCVM_MEM_OK);
  map(m,UINT32_C(0x4000),UINT32_C(0xa000),pp);
  m->cpu.pc=0;
  m->cpu.msr=UINT32_C(0x10);
  m->cpu.gpr[4]=UINT32_C(0x4000);
}
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t value=0;
  setup(&m,UINT32_C(0x80640000),2); /* lwz r3,0(r4) */
  assert(ppcvm_memory_write32be(&m.ram,UINT32_C(0xa000),UINT32_C(0x11223344))==PPCVM_MEM_OK);
  assert(ppcvm_pegasos2_step_pte(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==UINT32_C(0x11223344));
  ppcvm_pegasos2_destroy(&m);
  setup(&m,UINT32_C(0x90640000),2); /* stw r3,0(r4) */
  m.cpu.gpr[3]=UINT32_C(0x55667788);
  assert(ppcvm_pegasos2_step_pte(&m)==PPCVM_OK);
  assert(ppcvm_memory_read32be(&m.ram,UINT32_C(0xa000),&value)==PPCVM_MEM_OK);
  assert(value==UINT32_C(0x55667788));
  ppcvm_pegasos2_destroy(&m);
  setup(&m,UINT32_C(0x88640000),2); /* lbz r3,0(r4) */
  assert(ppcvm_memory_write8(&m.ram,UINT32_C(0xa000),UINT8_C(0xab))==PPCVM_MEM_OK);
  assert(ppcvm_pegasos2_step_pte(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==UINT32_C(0xab));
  ppcvm_pegasos2_destroy(&m);
  setup(&m,UINT32_C(0x98640000),2); /* stb r3,0(r4) */
  m.cpu.gpr[3]=UINT32_C(0xcd);
  assert(ppcvm_pegasos2_step_pte(&m)==PPCVM_OK);
  uint8_t byte=0;
  assert(ppcvm_memory_read8(&m.ram,UINT32_C(0xa000),&byte)==PPCVM_MEM_OK);
  assert(byte==UINT8_C(0xcd));
  ppcvm_pegasos2_destroy(&m);
  setup(&m,UINT32_C(0x90640000),1); /* write denied */
  m.cpu.gpr[3]=UINT32_C(0x11223344);
  assert(ppcvm_pegasos2_step_pte(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI);
  assert(m.cpu.dar==UINT32_C(0x4000));
  assert(m.cpu.dsisr==UINT32_C(0x0a000000));
  ppcvm_pegasos2_destroy(&m);
  setup(&m,UINT32_C(0x80640000),2); /* translation miss */
  m.cpu.gpr[4]=UINT32_C(0x8000);
  assert(ppcvm_pegasos2_step_pte(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI);
  assert(m.cpu.dar==UINT32_C(0x8000));
  assert(m.cpu.dsisr==UINT32_C(0x40000000));
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
