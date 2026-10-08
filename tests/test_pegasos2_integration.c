#include "ppcvm/pegasos2.h"
#include <assert.h>
#include <stdint.h>
#define ADDI(rt,ra,imm) ((14u<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(imm)&0xffffu))
#define X(rt,ra,rb,xo) ((31u<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(rb)<<11)|((uint32_t)(xo)<<1))
#define SPR(op,rt,spr) ((31u<<26)|((uint32_t)(rt)<<21)|(((uint32_t)(spr)&31u)<<16)|(((uint32_t)(spr)>>5)<<11)|((uint32_t)(op)<<1))
#define BC(bo,bi,bd) ((16u<<26)|((uint32_t)(bo)<<21)|((uint32_t)(bi)<<16)|((uint32_t)(bd)&0xfffcu))
#define B(d,lk) ((18u<<26)|((uint32_t)(d)&0x03fffffcu)|(lk))
#define BCLR ((19u<<26)|(20u<<21)|(16u<<1))
#define STW(rs,ra,d) ((36u<<26)|((uint32_t)(rs)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(d)&0xffffu))
#define LWZ(rt,ra,d) ((32u<<26)|((uint32_t)(rt)<<21)|((uint32_t)(ra)<<16)|((uint32_t)(d)&0xffffu))
int main(void) {
  ppcvm_pegasos2 m;
  assert(ppcvm_pegasos2_init(&m,4096)==0);
  /* Main: call function at 0x20, store result, reload it, then stop on sentinel. */
  const uint32_t program[]={
    ADDI(3,0,0),       /* 00: accumulator */
    ADDI(4,0,4),       /* 04: loop counter */
    B(0x18,1),        /* 08: bl 0x20, LR=0x0c */
    STW(3,0,0x100),   /* 0c: write sum */
    LWZ(5,0,0x100),   /* 10: reload */
    UINT32_C(0xffffffff), /* 14: deliberate unsupported sentinel */
    0,0,
    X(3,3,4,266),     /* 20: sum += counter */
    ADDI(4,4,-1),     /* 24: counter-- */
    BC(12,0,8),       /* 28: conditional branch (CR0 LT) not taken */
    BC(16,0,0xfff4),  /* 2c: decrement CTR; loop if nonzero */
    BCLR              /* 30: return */
  };
  /* Initialize CTR=4 and CR0 LT=0; 4 iterations, 4+3+2+1=10. */
  m.cpu.ctr=4;
  for (size_t i=0;i<sizeof(program)/sizeof(program[0]);++i)
    assert(ppcvm_bus_write32be(&m.bus,(uint32_t)(i*4),program[i])==PPCVM_BUS_OK);
  size_t executed=0;
  assert(ppcvm_pegasos2_run(&m,64,&executed)==PPCVM_UNSUPPORTED);
  assert(executed==20);
  assert(m.cpu.pc==0x14 && m.cpu.lr==0x0c);
  assert(m.cpu.gpr[3]==10 && m.cpu.gpr[5]==10);
  assert(m.cpu.ctr==0 && m.cpu.gpr[4]==0);
  uint32_t stored=0;
  assert(ppcvm_bus_read32be(&m.bus,0x100,&stored)==PPCVM_BUS_OK && stored==10);
  ppcvm_pegasos2_destroy(&m);
  return 0;
}
