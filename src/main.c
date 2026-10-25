#include "ppcvm/pegasos2.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int usage(const char *program) {
  fprintf(stderr, "Usage: %s --elf <powerpc-be-elf32> [--steps N] [--ram MiB] [--abi-v2] [--trace] [--trace-limit N] [--dump-registers] [--break-pc HEX] [--inspect-elf]\n", program);
  fprintf(stderr, "Experimental PVC1 guest runner; not an Open Firmware or Linux boot environment.\n");
  return 2;
}
static void dump_registers(const ppcvm_cpu *cpu) {
  fprintf(stderr, "CPU: pc=%08" PRIx32 " lr=%08" PRIx32
          " ctr=%08" PRIx32 " cr=%08" PRIx32
          " xer=%08" PRIx32 " msr=%08" PRIx32 "\n",
          cpu->pc, cpu->lr, cpu->ctr, cpu->cr, cpu->xer, cpu->msr);
  fprintf(stderr, "Exceptions: srr0=%08" PRIx32 " srr1=%08" PRIx32
          " dar=%08" PRIx32 " dsisr=%08" PRIx32 "\n",
          cpu->srr0, cpu->srr1, cpu->dar, cpu->dsisr);
  for (unsigned i = 0; i < 32u; i += 4u)
    fprintf(stderr, "r%02u=%08" PRIx32 " r%02u=%08" PRIx32
            " r%02u=%08" PRIx32 " r%02u=%08" PRIx32 "\n",
            i, cpu->gpr[i], i+1u, cpu->gpr[i+1u],
            i+2u, cpu->gpr[i+2u], i+3u, cpu->gpr[i+3u]);
}
static uint16_t elf_be16(const uint8_t *p) {
  return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}
static uint32_t elf_be32(const uint8_t *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8) | p[3];
}
static int inspect_elf(const uint8_t *image, size_t length) {
  uint32_t phoff;
  uint16_t phentsize, phnum;
  if (length < 52u || (image[0] != 0x7fu || image[1] != 'E' || image[2] != 'L' || image[3] != 'F') ||
      image[4] != 1u || image[5] != 2u || image[6] != 1u ||
      elf_be16(image+18) != 20u) {
    fprintf(stderr, "Not an ELF32 big-endian PowerPC image\n");
    return 0;
  }
  phoff = elf_be32(image+28);
  phentsize = elf_be16(image+42);
  phnum = elf_be16(image+44);
  if (phentsize < 32u || (size_t)phoff > length ||
      (size_t)phnum > (length-(size_t)phoff)/phentsize) {
    fprintf(stderr, "Invalid ELF program-header table\n");
    return 0;
  }
  printf("ELF32 PPC: type=%u entry=0x%08" PRIx32 " segments=%u\n",
         (unsigned)elf_be16(image+16), elf_be32(image+24), (unsigned)phnum);
  for (uint16_t i=0; i<phnum; ++i) {
    const uint8_t *p=image+(size_t)phoff+(size_t)i*phentsize;
    if (elf_be32(p)==1u)
      printf("LOAD %u: offset=0x%08" PRIx32 " vaddr=0x%08" PRIx32
             " paddr=0x%08" PRIx32 " filesz=%" PRIu32 " memsz=%" PRIu32 "\n",
             (unsigned)i, elf_be32(p+4), elf_be32(p+8),
             elf_be32(p+12), elf_be32(p+16), elf_be32(p+20));
  }
  return 1;
}
static int parse_address(const char *s, uint32_t *out) {
  char *end = NULL;
  unsigned long long value;
  if (!s || !*s || *s == '-') return 0;
  errno = 0;
  value = strtoull(s, &end, 0);
  if (errno || !end || *end || value > UINT32_MAX) return 0;
  *out = (uint32_t)value;
  return 1;
}
static int parse_size(const char *s, size_t *out) {
  char *end = NULL;
  unsigned long long n;
  if (!s || !*s || *s == '-') return 0;
  errno = 0;
  n = strtoull(s, &end, 10);
  if (errno || !end || *end || !n || n > (unsigned long long)SIZE_MAX) return 0;
  *out = (size_t)n;
  return 1;
}
int main(int argc, char **argv) {
  const char *path = NULL;
  size_t steps = 1000, ram_mib = 64, executed = 0, trace_limit = 1000;
  uint32_t break_pc = 0;
  int abi_v2 = 0, trace = 0, dump = 0, have_break = 0, inspect = 0, rc = 1;
  FILE *fp = NULL;
  uint8_t *image = NULL;
  ppcvm_pegasos2 machine;
  int initialized = 0;
  long length;
  ppcvm_result result;
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--elf") && i+1 < argc) path = argv[++i];
    else if (!strcmp(argv[i], "--steps") && i+1 < argc) {
      if (!parse_size(argv[++i], &steps)) return usage(argv[0]);
    } else if (!strcmp(argv[i], "--ram") && i+1 < argc) {
      if (!parse_size(argv[++i], &ram_mib) || ram_mib > SIZE_MAX / (1024u*1024u) ||
          ram_mib > UINT32_MAX / (1024u*1024u)) return usage(argv[0]);
    } else if (!strcmp(argv[i], "--abi-v2")) abi_v2 = 1;
    else if (!strcmp(argv[i], "--trace")) trace = 1;
    else if (!strcmp(argv[i], "--inspect-elf")) inspect = 1;
    else if (!strcmp(argv[i], "--dump-registers")) dump = 1;
    else if (!strcmp(argv[i], "--break-pc") && i+1 < argc) {
      if (!parse_address(argv[++i], &break_pc) || (break_pc & 3u)) return usage(argv[0]);
      have_break = 1;
    }
    else if (!strcmp(argv[i], "--trace-limit") && i+1 < argc) {
      if (!parse_size(argv[++i], &trace_limit)) return usage(argv[0]);
    }
    else return usage(argv[0]);
  }
  if (!path) return usage(argv[0]);
  fp = fopen(path, "rb");
  if (!fp) { perror(path); goto done; }
  if (fseek(fp, 0, SEEK_END) || (length = ftell(fp)) <= 0 ||
      fseek(fp, 0, SEEK_SET)) {
    fprintf(stderr, "Cannot determine ELF file size\n"); goto done;
  }
  image = malloc((size_t)length);
  if (!image) { fprintf(stderr, "Out of memory\n"); goto done; }
  if (fread(image, 1, (size_t)length, fp) != (size_t)length) {
    fprintf(stderr, "Failed reading ELF image\n"); goto done;
  }
  fclose(fp); fp = NULL;
  if (inspect) { rc = inspect_elf(image, (size_t)length) ? 0 : 1; goto done; }
  if (!inspect_elf(image, (size_t)length)) goto done;
  if (elf_be16(image+16) != 2u) {
    fprintf(stderr, "Guest ELF must be ET_EXEC (type 2), not relocatable or shared\n");
    goto done;
  }
  {
    const uint32_t phoff = elf_be32(image+28);
    const uint16_t phsize = elf_be16(image+42), count = elf_be16(image+44);
    const uint64_t ram_bytes = (uint64_t)ram_mib * 1024u * 1024u;
    for (uint16_t i = 0; i < count; ++i) {
      const uint8_t *p = image + (size_t)phoff + (size_t)i * phsize;
      if (elf_be32(p) != 1u) continue;
      const uint32_t offset = elf_be32(p+4), addr = elf_be32(p+8);
      const uint32_t file_bytes = elf_be32(p+16), mem_bytes = elf_be32(p+20);
      if (file_bytes > mem_bytes ||
          (uint64_t)offset + file_bytes > (uint64_t)(size_t)length ||
          (uint64_t)addr + mem_bytes > ram_bytes) {
        fprintf(stderr, "LOAD %u does not fit file/RAM: vaddr=0x%08" PRIx32
                " filesz=%" PRIu32 " memsz=%" PRIu32 " RAM=%zu MiB\n",
                (unsigned)i, addr, file_bytes, mem_bytes, ram_mib);
        goto done;
      }
    }
  }
  if (ppcvm_pegasos2_init(&machine, ram_mib * 1024u * 1024u)) {
    fprintf(stderr, "Machine initialization failed\n"); goto done;
  }
  initialized = 1;
  result = abi_v2
      ? ppcvm_pegasos2_boot_elf32_abi_v2(&machine, image, (size_t)length, 0x1000u)
      : ppcvm_pegasos2_boot_elf32(&machine, image, (size_t)length);
  if (result != PPCVM_OK) {
    fprintf(stderr, "ELF load/boot failed: result=%d\n", (int)result); goto done;
  }
  printf("ELF loaded: entry=0x%08" PRIx32 ", RAM=%zu MiB, ABI=%s\n",
         machine.cpu.pc, ram_mib, abi_v2 ? "PVC1-v2" : "raw");
  if (trace || have_break) {
    result = PPCVM_OK;
    for (size_t i = 0; i < steps; ++i) {
      uint32_t pc = machine.cpu.pc, opcode = 0;
      if (have_break && pc == break_pc) {
        fprintf(stderr, "breakpoint: step=%zu pc=0x%08" PRIx32 "\n", i, pc);
        dump_registers(&machine.cpu);
        break;
      }
      ppcvm_mem_result fetch = ppcvm_memory_read32be(&machine.ram, pc, &opcode);
      if (trace && i < trace_limit)
        fprintf(stderr, "step=%zu pc=0x%08" PRIx32 " opcode=%s0x%08" PRIx32 "\n",
                i, pc, fetch == PPCVM_MEM_OK ? "" : "unmapped:", opcode);
      result = ppcvm_pegasos2_step(&machine);
      if (result != PPCVM_OK) {
        fprintf(stderr, "stop: step=%zu pc=0x%08" PRIx32 " result=%d\n",
                i, pc, (int)result);
        break;
      }
      ++executed;
    }
  } else {
    result = ppcvm_pegasos2_run(&machine, steps, &executed);
  }
  printf("Guest: result=%d, executed=%zu, pc=0x%08" PRIx32 "\n",
         (int)result, executed, machine.cpu.pc);
  if (dump || result != PPCVM_OK) dump_registers(&machine.cpu);
  rc = result == PPCVM_OK ? 0 : 1;
done:
  if (initialized) ppcvm_pegasos2_destroy(&machine);
  free(image);
  if (fp) fclose(fp);
  return rc;
}
