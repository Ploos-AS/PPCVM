#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
static void w16(uint8_t *p,unsigned x){p[0]=(uint8_t)(x>>8);p[1]=(uint8_t)x;}
static void w32(uint8_t *p,uint32_t x){p[0]=(uint8_t)(x>>24);p[1]=(uint8_t)(x>>16);p[2]=(uint8_t)(x>>8);p[3]=(uint8_t)x;}
int main(void){
  ppcvm_pegasos2 m;
  uint8_t elf[128]={0};
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  elf[0]=0x7f;elf[1]='E';elf[2]='L';elf[3]='F';elf[4]=1;elf[5]=2;elf[6]=1;
  w16(elf+16,2);w16(elf+18,20);w32(elf+20,1);
  w32(elf+24,0x100);w32(elf+28,52);
  w16(elf+40,52);w16(elf+42,32);w16(elf+44,1);
  w32(elf+52,1);w32(elf+56,96);w32(elf+64,0x100);
  w32(elf+68,24);w32(elf+72,24);w32(elf+76,5);
  /* ELF guest reads the actual ABI v2 record using r4 from the loader. */
  w32(elf+96,0x80e40008);  /* lwz r7,8(r4): RAM bytes */
  w32(elf+100,0x8104000c); /* lwz r8,12(r4): ELF entry */
  w32(elf+104,0x81240010); /* lwz r9,16(r4): ABI version */
  w32(elf+108,0x2c090002); /* cmpwi r9,2 */
  w32(elf+112,0x40820008); /* bne +8: skip compatible marker */
  w32(elf+116,0x39400001); /* addi r10,r0,1: compatible */
  assert(ppcvm_pegasos2_boot_elf32_abi_v2(&m,elf,sizeof(elf),0x200)==PPCVM_OK);
  uint32_t word=0;
  assert(ppcvm_memory_read32be(&m.ram,0x200,&word)==PPCVM_MEM_OK && word==PPCVM_PEGASOS2_BOOT_MAGIC);
  assert(ppcvm_memory_read32be(&m.ram,0x204,&word)==PPCVM_MEM_OK && word==32);
  assert(ppcvm_memory_read32be(&m.ram,0x210,&word)==PPCVM_MEM_OK && word==2);
  assert(ppcvm_memory_read32be(&m.ram,0x218,&word)==PPCVM_MEM_OK && word==0);
  assert(m.cpu.pc==UINT32_C(0x100));
  assert(m.cpu.gpr[4]==UINT32_C(0x200));
  assert(ppcvm_pegasos2_run(&m,6u,0)==PPCVM_OK);
  assert(m.cpu.gpr[7]==UINT32_C(65536));
  assert(m.cpu.gpr[8]==UINT32_C(0x100));
  assert(m.cpu.gpr[9]==2u && m.cpu.gpr[10]==1u);
  assert(m.cpu.pc==UINT32_C(0x118));
  /* Re-run the same ELF with a deliberately incompatible ABI version.
   * Guest code must observe the changed record, not a host-side constant. */
  assert(ppcvm_memory_write32be(&m.ram,
      UINT32_C(0x200)+PPCVM_PEGASOS2_BOOT_INFO_V2_VERSION_OFFSET,
      3u)==PPCVM_MEM_OK);
  m.cpu.pc=UINT32_C(0x100);
  assert(ppcvm_pegasos2_run(&m,4u,0)==PPCVM_OK);
  assert(m.cpu.gpr[9]==3u && m.cpu.gpr[10]==4u);
  m.cpu.pc=0x44;
  /* v2's extra 16 bytes must not overlap the executable's BSS. */
  w32(elf+64,0x210);
  assert(ppcvm_pegasos2_boot_elf32_abi_v2(&m,elf,sizeof(elf),0x200)==PPCVM_MEMORY_FAULT);
  assert(m.cpu.pc==0x44);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
