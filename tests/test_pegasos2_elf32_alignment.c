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
  elf[0]=0x7f;elf[1]='E';elf[2]='L';elf[3]='F';
  elf[4]=1;elf[5]=2;elf[6]=1;
  w16(elf+16,2);w16(elf+18,20);w32(elf+20,1);
  w32(elf+24,0x100);w32(elf+28,52);
  w16(elf+40,52);w16(elf+42,32);w16(elf+44,1);
  w32(elf+52,1);w32(elf+56,96);w32(elf+64,0x100);
  w32(elf+68,4);w32(elf+72,8);w32(elf+76,5);
  w32(elf+96,0x3860002a);
  uint32_t entry=0;
  assert(ppcvm_pegasos2_load_elf32(&m,elf,sizeof(elf),&entry)==PPCVM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x100,0x11223344)==PPCVM_MEM_OK);
  w32(elf+80,3); /* non-power-of-two alignment */
  assert(ppcvm_pegasos2_load_elf32(&m,elf,sizeof(elf),&entry)==PPCVM_UNSUPPORTED);
  w32(elf+80,16); /* offset 96 and address 0x100 are congruent */
  assert(ppcvm_pegasos2_load_elf32(&m,elf,sizeof(elf),&entry)==PPCVM_OK);
  w32(elf+80,32);
  assert(ppcvm_pegasos2_load_elf32(&m,elf,sizeof(elf),&entry)==PPCVM_OK);
  w32(elf+80,64); /* offset 96 vs address 0x100: not congruent */
  assert(ppcvm_pegasos2_load_elf32(&m,elf,sizeof(elf),&entry)==PPCVM_UNSUPPORTED);
  w32(elf+80,0);w32(elf+76,8); /* unsupported flags */
  assert(ppcvm_pegasos2_load_elf32(&m,elf,sizeof(elf),&entry)==PPCVM_UNSUPPORTED);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
