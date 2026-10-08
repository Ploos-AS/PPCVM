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
  w32(elf+68,4);w32(elf+72,8);w32(elf+76,5);w32(elf+96,0x3860002a);
  m.cpu.pc=0x44;m.cpu.gpr[3]=123;
  assert(ppcvm_memory_write32be(&m.ram,0x200,0x11223344)==PPCVM_MEM_OK);
  assert(ppcvm_memory_write32be(&m.ram,0x100,0xaabbccdd)==PPCVM_MEM_OK);
  /* Invalid entry must not alter CPU, loaded image, or boot record. */
  w32(elf+24,0x104);
  assert(ppcvm_pegasos2_boot_elf32_abi(&m,elf,sizeof(elf),0x200)==PPCVM_MEMORY_FAULT);
  uint32_t value=0;
  assert(m.cpu.pc==0x44 && m.cpu.gpr[3]==123);
  assert(ppcvm_memory_read32be(&m.ram,0x100,&value)==PPCVM_MEM_OK && value==0xaabbccdd);
  assert(ppcvm_memory_read32be(&m.ram,0x200,&value)==PPCVM_MEM_OK && value==0x11223344);
  /* Boot info inside segment BSS is also forbidden. */
  w32(elf+24,0x100);w32(elf+72,0x120);
  assert(ppcvm_pegasos2_boot_elf32_abi(&m,elf,sizeof(elf),0x200)==PPCVM_MEMORY_FAULT);
  assert(m.cpu.pc==0x44);
  assert(ppcvm_memory_read32be(&m.ram,0x100,&value)==PPCVM_MEM_OK && value==0xaabbccdd);
  assert(ppcvm_memory_read32be(&m.ram,0x200,&value)==PPCVM_MEM_OK && value==0x11223344);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
