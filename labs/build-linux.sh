#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
if [[ $(uname -s) != Linux ]]; then
  echo 'This build script targets the company Linux environment.' >&2
  exit 2
fi
mode=${1:-portable}
cxx=${CXX:-g++}
flags=(-std=c++17 -O3 -g -Wall -Wextra -Wconversion -fno-lto -ffp-contract=off)
isa=()
defines=()
case "$mode" in
  portable) ;;
  avx2)
    [[ $(uname -m) == x86_64 ]] || { echo 'avx2 requires x86_64' >&2; exit 2; }
    grep -qw avx2 /proc/cpuinfo || { echo 'Confirm AVX2 CPU/OS support first' >&2; exit 2; }
    isa=(-mavx2); defines=(-DUSE_AVX2) ;;
  sve)
    [[ $(uname -m) == aarch64 ]] || { echo 'sve requires aarch64' >&2; exit 2; }
    grep -qw sve /proc/cpuinfo || { echo 'Confirm SVE CPU/OS support first' >&2; exit 2; }
    isa=(-march=armv8.2-a+sve -msve-vector-bits=scalable); defines=(-DUSE_SVE) ;;
  *) echo 'usage: bash build-linux.sh portable|avx2|sve' >&2; exit 2 ;;
esac
out="build/$mode"
mkdir -p "$out"
"$cxx" --version > "$out/compiler.txt"
"$cxx" "${flags[@]}" "${isa[@]}" -fno-tree-vectorize -c scalar.cpp -o "$out/scalar.o"
"$cxx" "${flags[@]}" "${isa[@]}" -fopt-info-vec-optimized -fopt-info-vec-missed \
    -c auto.cpp -o "$out/auto.o" 2> "$out/vectorization.txt"
"$cxx" "${flags[@]}" "${defines[@]}" -c main.cpp -o "$out/main.o"
objects=("$out/main.o" "$out/scalar.o" "$out/auto.o")
if [[ $mode == avx2 || $mode == sve ]]; then
  "${cxx}" "${flags[@]}" "${isa[@]}" "${defines[@]}" -c "$mode.cpp" -o "$out/simd.o"
  objects+=("$out/simd.o")
fi
"$cxx" "${flags[@]}" "${objects[@]}" -o "$out/demo"
echo "Built $out/demo. Run correctness check before benchmarking."
