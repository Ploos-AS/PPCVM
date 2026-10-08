#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
static void w16(uint8_t *p,unsigned x){p[0]=(uint8_t)(x>>8);p[1]=(uint8_t)x;}
static void w32(uint8_t *p,uint32_t x){p[0]=(uint8_t)(x>>24);p[1]=(uint8_t)(x>>16);p[2]=(uint8_t)(x>>8);p[3]=(uint8_t)x;}
int main(void){
  ppcvm_pegasos2 m;
  uint8_t elf[160]={0};
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  elf[0]=0x7f;elf[1]='E';elf[2]='L';elf[3]='F';
  elf[4]=1;elf[5]=2;elf[6]=1;
  w16(elf+16,2);w16(elf+18,20);w32(elf+20,1);
  w32(elf+24,0x100);w32(elf+28,52);
  w16(elf+40,52);w16(elf+42,32);w16(elf+44,2);
  w32(elf+52,1);w32(elf+56,120);w32(elf+64,0x100);
  w32(elf+68,4);w32(elf+72,8);
  w32(elf+84,1);w32(elf+88,124);w32(elf+96,0x200);
  w32(elf+100,4);w32(elf+104,12);
  w32(elf+120,0x3860002a);w32(elf+124,0x3880002b);
  uint32_t entry=0;
  assert(ppcvm_pegasos2_load_elf32(&m,elf,sizeof(elf),&entry)==PPCVM_OK && entry==0x100);
  uint32_t value=0;
  assert(ppcvm_memory_read32be(&m.ram,0x200,&value)==PPCVM_MEM_OK && value==0x3880002b);
  assert(ppcvm_memory_read32be(&m.ram,0x204,&value)==PPCVM_MEM_OK && value==0);
  /* Overlap second segment with first, reject before modifying either. */
  assert(ppcvm_memory_write32be(&m.ram,0x100,0x11223344)==PPCVM_MEM_OK);
  w32(elf+96,0x104);
  assert(ppcvm_pegasos2_load_elf32(&m,elf,sizeof(elf),&entry)==PPCVM_MEMORY_FAULT);
  assert(ppcvm_memory_read32be(&m.ram,0x100,&value)==PPCVM_MEM_OK && value==0x11223344);
  w32(elf+96,0x200);
  /* Reject file-size larger than memory-size, with no writes. */
  w32(elf+104,3);
  assert(ppcvm_pegasos2_load_elf32(&m,elf,sizeof(elf),&entry)==PPCVM_MEMORY_FAULT);
  assert(ppcvm_memory_read32be(&m.ram,0x100,&value)==PPCVM_MEM_OK && value==0x11223344);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
