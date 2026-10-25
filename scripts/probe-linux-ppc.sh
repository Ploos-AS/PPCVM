#!/usr/bin/env bash
# Reproducible first-failure probe for a real, locally supplied Linux/PPC ELF.
set -euo pipefail
if (( $# < 1 || $# > 3 )); then
  echo "Usage: $0 path/to/vmlinux [path/to/ppcvm] [output-directory]" >&2
  exit 2
fi
kernel=$1
emulator=${2:-./build/ppcvm}
output=${3:-build/linux-ppc-probe}
for tool in readelf sha256sum; do
  command -v "$tool" >/dev/null || { echo "Missing tool: $tool" >&2; exit 2; }
done
[[ -f "$kernel" ]] || { echo "Missing kernel: $kernel" >&2; exit 2; }
[[ -x "$emulator" ]] || { echo "Missing executable: $emulator" >&2; exit 2; }
mkdir -p "$output"
readelf -h -l "$kernel" > "$output/readelf.txt"
if ! readelf -h "$kernel" | grep -Eq 'Class:[[:space:]]*ELF32' ||
   ! readelf -h "$kernel" | grep -Eq 'Data:[[:space:]]*2.s complement, big endian' ||
   ! readelf -h "$kernel" | grep -Eq 'Machine:[[:space:]]*PowerPC'; then
  echo "Kernel is not ELF32 big-endian PowerPC; see $output/readelf.txt" >&2
  exit 2
fi
sha256sum "$kernel" > "$output/kernel.sha256"
{
  printf 'Kernel: %s\nEmulator: %s\n' "$kernel" "$emulator"
  printf 'Invocation: %q --elf %q --ram 256 --steps 100000 --trace --trace-limit 200 --dump-registers\n' "$emulator" "$kernel"
} > "$output/metadata.txt"
set +e
"$emulator" --elf "$kernel" --ram 256 --steps 100000 \
  --trace --trace-limit 200 --dump-registers > "$output/stdout.log" 2> "$output/stderr.log"
status=$?
set -e
printf 'Exit status: %d\n' "$status" | tee -a "$output/metadata.txt"
echo "Probe artifacts: $output"
# A nonzero emulator exit is a useful first-failure result; preserve it for CI.
exit "$status"
