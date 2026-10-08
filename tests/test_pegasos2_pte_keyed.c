#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
static void setup(ppcvm_pegasos2 *m,uint32_t msr) {
  assert(ppcvm_pegasos2_init(m,131072)==0);
  m->segments.sr[0]=UINT32_C(0x20000123); /* Ks=0, Kp=1 */
  assert(ppcvm_memory_write32be(&m->ram,0,UINT32_C(0x90640000))==PPCVM_MEM_OK); /* stw r3,0(r4) */
  uint32_t pteg=0,ea=UINT32_C(0x4000);
  assert(ppcvm_mmu_pteg_address(&m->segments,ea,0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&m->ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m->ram,pteg+4,UINT32_C(0xa001))==PPCVM_MEM_OK);
  m->cpu.msr=msr;
  m->cpu.gpr[4]=ea;
  m->cpu.gpr[3]=UINT32_C(0x11223344);
}
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t value=0,pteg=0,pte1=0;
  setup(&m,UINT32_C(0x4010)); /* problem mode, DR enabled */
  assert(ppcvm_mmu_pteg_address(&m.segments,UINT32_C(0x4000),0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI);
  assert(m.cpu.dar==UINT32_C(0x4000));
  assert(m.cpu.dsisr==UINT32_C(0x0a000000));
  assert(ppcvm_memory_read32be(&m.ram,UINT32_C(0xa000),&value)==PPCVM_MEM_OK);
  assert(value==0);
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa001));
  ppcvm_pegasos2_destroy(&m);
  setup(&m,UINT32_C(0x10)); /* supervisor mode, DR enabled */
  assert(ppcvm_mmu_pteg_address(&m.segments,UINT32_C(0x4000),0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==4);
  assert(ppcvm_memory_read32be(&m.ram,UINT32_C(0xa000),&value)==PPCVM_MEM_OK);
  assert(value==UINT32_C(0x11223344));
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa181));
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
