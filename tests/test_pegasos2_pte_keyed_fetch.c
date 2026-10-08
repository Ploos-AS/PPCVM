#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
static void setup(ppcvm_pegasos2 *m,uint32_t msr) {
  assert(ppcvm_pegasos2_init(m,131072)==0);
  m->segments.sr[0]=UINT32_C(0x20000123); /* Ks=0, Kp=1 */
  uint32_t pteg=0;
  assert(ppcvm_mmu_pteg_address(&m->segments,0,0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&m->ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m->ram,pteg+4,UINT32_C(0xa001))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m->ram,UINT32_C(0xa000),UINT32_C(0x60000000))==PPCVM_MEM_OK); /* ori r0,r0,0 */
  m->cpu.msr=msr;
  m->cpu.pc=0;
}
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t pteg=0,pte1=0;
  setup(&m,UINT32_C(0x4020)); /* problem mode, IR enabled */
  assert(ppcvm_mmu_pteg_address(&m.segments,0,0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==4);
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa101)); /* fetch sets R only */
  ppcvm_pegasos2_destroy(&m);
  setup(&m,UINT32_C(0x20)); /* supervisor */
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==4);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
