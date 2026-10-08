#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
static void setup(ppcvm_pegasos2 *m,uint32_t instruction) {
  assert(ppcvm_pegasos2_init(m,131072)==0);
  m->segments.sr[0]=UINT32_C(0x20000123); /* problem key=1 */
  assert(ppcvm_memory_write32be(&m->ram,0,instruction)==PPCVM_MEM_OK);
  uint32_t pteg=0;
  assert(ppcvm_mmu_pteg_address(&m->segments,UINT32_C(0x4000),0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_memory_write32be(&m->ram,pteg,UINT32_C(0x80009180))==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m->ram,pteg+4,UINT32_C(0xa001))==PPCVM_MEM_OK);
  m->cpu.msr=UINT32_C(0x4010);
  m->cpu.gpr[4]=UINT32_C(0x4000);
}
int main(void) {
  ppcvm_pegasos2 m;
  uint32_t pteg=0,pte1=0,value=0;
  setup(&m,UINT32_C(0x90640000)); /* stw r3,0(r4) */
  m.cpu.gpr[3]=UINT32_C(0x11223344);
  m.bat.dbatu[0]=UINT32_C(0x00000001); /* user-valid BAT */
  m.bat.dbatl[0]=UINT32_C(0x00000002); /* RW BAT maps EA directly */
  assert(ppcvm_mmu_pteg_address(&m.segments,UINT32_C(0x4000),0,&pteg)==PPCVM_MMU_OK);
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==4);
  assert(ppcvm_memory_read32be(&m.ram,UINT32_C(0x4000),&value)==PPCVM_MEM_OK);
  assert(value==UINT32_C(0x11223344));
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa001)); /* no PTE R/C changes on BAT hit */
  ppcvm_pegasos2_destroy(&m);
  setup(&m,UINT32_C(0x90640000));
  m.bat.dbatu[0]=UINT32_C(0x00000001);
  m.bat.dbatl[0]=0; /* BAT protection must not fall back to PTE */
  assert(ppcvm_pegasos2_step_pte_keyed(&m)==PPCVM_OK);
  assert(m.cpu.pc==PPCVM_VECTOR_DSI);
  assert(m.cpu.dsisr==UINT32_C(0x0a000000));
  assert(ppcvm_memory_read32be(&m.ram,pteg+4,&pte1)==PPCVM_MEM_OK);
  assert(pte1==UINT32_C(0xa001));
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
