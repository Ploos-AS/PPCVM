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
  w32(elf+68,8);w32(elf+72,12);w32(elf+76,5);
  w32(elf+96,0x3860002a);w32(elf+100,0x38830001);
  m.cpu.pc=0x40;
  assert(ppcvm_pegasos2_boot_elf32(&m,elf,sizeof(elf))==PPCVM_OK);
  assert(m.cpu.pc==0x100);
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==42 && m.cpu.gpr[4]==43 && m.cpu.pc==0x108);
  m.cpu.pc=0x44;
  w32(elf+24,0x104); /* Entry is valid but points at second instruction. */
  assert(ppcvm_pegasos2_boot_elf32(&m,elf,sizeof(elf))==PPCVM_OK && m.cpu.pc==0x104);
  m.cpu.pc=0x44;
  w32(elf+24,0x108); /* Entry is in BSS and must be rejected. */
  assert(ppcvm_pegasos2_boot_elf32(&m,elf,sizeof(elf))==PPCVM_MEMORY_FAULT);
  assert(m.cpu.pc==0x44);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
