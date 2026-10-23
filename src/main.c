#include "ppcvm/pegasos2.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int usage(const char *program) {
  fprintf(stderr, "Usage: %s --elf <powerpc-be-elf32> [--steps N] [--ram MiB] [--abi-v2] [--trace]\n", program);
  fprintf(stderr, "Experimental PVC1 guest runner; not an Open Firmware or Linux boot environment.\n");
  return 2;
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
  size_t steps = 1000, ram_mib = 64, executed = 0;
  int abi_v2 = 0, trace = 0, rc = 1;
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
  if (trace) {
    result = PPCVM_OK;
    for (size_t i = 0; i < steps; ++i) {
      uint32_t pc = machine.cpu.pc, opcode = 0;
      ppcvm_mem_result fetch = ppcvm_memory_read32be(&machine.ram, pc, &opcode);
      fprintf(stderr, "step=%zu pc=0x%08" PRIx32 " opcode=%s0x%08" PRIx32 "\\n",
              i, pc, fetch == PPCVM_MEM_OK ? "" : "unmapped:", opcode);
      result = ppcvm_pegasos2_step(&machine);
      if (result != PPCVM_OK) {
        fprintf(stderr, "stop: step=%zu pc=0x%08" PRIx32 " result=%d\\n",
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
  rc = result == PPCVM_OK ? 0 : 1;
done:
  if (initialized) ppcvm_pegasos2_destroy(&machine);
  free(image);
  if (fp) fclose(fp);
  return rc;
}
