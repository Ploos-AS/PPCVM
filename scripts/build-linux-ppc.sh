#!/usr/bin/env bash
# Build a real 32-bit big-endian PowerPC Linux vmlinux from a supplied source tree.
set -euo pipefail
if (( $# < 1 || $# > 3 )); then
  echo "Usage: $0 linux-source-directory [output-directory] [kernel-defconfig]" >&2
  echo "Example: $0 ../linux build/linux-ppc pmac32_defconfig" >&2
  exit 2
fi
source_dir=$(cd "$1" && pwd)
output_dir=${2:-build/linux-ppc}
defconfig=${3:-pmac32_defconfig}
mkdir -p "$output_dir"
output_dir=$(cd "$output_dir" && pwd)
for command in make powerpc-linux-gnu-gcc powerpc-linux-gnu-ld readelf sha256sum; do
  command -v "$command" >/dev/null || { echo "Missing dependency: $command" >&2; exit 2; }
done
[[ -f "$source_dir/Makefile" && -d "$source_dir/arch/powerpc" ]] || {
  echo "Not a Linux PowerPC source tree: $source_dir" >&2; exit 2;
}
printf 'Source: %s\nOutput: %s\nConfig: %s\n' "$source_dir" "$output_dir" "$defconfig" > "$output_dir/build-metadata.txt"
if git -C "$source_dir" rev-parse HEAD >/dev/null 2>&1; then
  git -C "$source_dir" rev-parse HEAD >> "$output_dir/build-metadata.txt"
fi
make -C "$source_dir" O="$output_dir" ARCH=powerpc CROSS_COMPILE=powerpc-linux-gnu- "$defconfig"
make -C "$source_dir" O="$output_dir" ARCH=powerpc CROSS_COMPILE=powerpc-linux-gnu- -j"${JOBS:-2}" vmlinux
readelf -h -l "$output_dir/vmlinux" > "$output_dir/vmlinux.readelf.txt"
header=$(readelf -h "$output_dir/vmlinux")
if ! grep -Eq 'Class:[[:space:]]*ELF32' <<< "$header" ||
   ! grep -Eq 'Data:[[:space:]]*2.s complement, big endian' <<< "$header" ||
   ! grep -Eq 'Machine:[[:space:]]*PowerPC' <<< "$header"; then
  echo "Built kernel is not ELF32 big-endian PowerPC; inspect $output_dir/vmlinux.readelf.txt" >&2
  exit 1
fi
sha256sum "$output_dir/vmlinux" > "$output_dir/vmlinux.sha256"
echo "Built ELF32 PPC kernel: $output_dir/vmlinux"
echo "Run: bash scripts/probe-linux-ppc.sh '$output_dir/vmlinux' ./build/ppcvm '$output_dir/probe'"
