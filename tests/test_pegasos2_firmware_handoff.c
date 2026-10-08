#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#define ADDI(rt,ra,imm) ((14u<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint16_t)(imm)))
#define ADDIS(rt,ra,imm) ((15u<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint16_t)(imm)))
#define ORI(rs,ra,imm) ((24u<<26)|((uint32_t)(rs)<<21)|((uint32_t)(ra)<<16)|((uint16_t)(imm)))
#define STW(rs,ra,imm) ((36u<<26)|((uint32_t)(rs)<<21)|((uint32_t)(ra)<<16)|((uint16_t)(imm)))
#define MTSPR(rs,spr) ((31u<<26)|((uint32_t)(rs)<<21)|(((uint32_t)(spr)&31u)<<16)|(((uint32_t)(spr)>>5)<<11)|(467u<<1))
#define BCTR ((19u<<26)|(20u<<21)|(528u<<1))
static void put32(uint8_t *rom, unsigned off, uint32_t w) {
  rom[off]=(uint8_t)(w>>24); rom[off+1]=(uint8_t)(w>>16);
  rom[off+2]=(uint8_t)(w>>8); rom[off+3]=(uint8_t)w;
}
int main(void) {
  ppcvm_pegasos2 m;
  uint8_t rom[4096];
  memset(rom,0,sizeof(rom));
  /* Synthetic firmware writes one guest instruction to RAM, then jumps there. */
  const uint32_t guest=ADDI(5,3,1);
  put32(rom,0x100,ADDIS(4,0,(guest>>16)));
  put32(rom,0x104,ORI(4,4,guest&0xffffu));
  put32(rom,0x108,STW(4,0,0));
  put32(rom,0x10c,ADDI(3,0,42));
  put32(rom,0x110,ADDI(6,0,0));
  put32(rom,0x114,MTSPR(6,9));
  put32(rom,0x118,BCTR);
  assert(ppcvm_pegasos2_init(&m,65536)==0);
  assert(ppcvm_pegasos2_map_high_rom(&m,rom,sizeof(rom))==PPCVM_BUS_OK);
  assert(ppcvm_pegasos2_cold_boot_high_rom(&m,UINT32_C(0xfff00100))==PPCVM_OK);
  for (unsigned i=0;i<7;i++) assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.pc==0);
  uint32_t word=0;
  assert(ppcvm_memory_read32be(&m.ram,0,&word)==PPCVM_MEM_OK && word==guest);
  assert(ppcvm_pegasos2_step(&m)==PPCVM_OK);
  assert(m.cpu.gpr[3]==42 && m.cpu.gpr[5]==43 && m.cpu.pc==4);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
