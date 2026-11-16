#!/bin/sh
# Build the MOS (6502) SDK with clang from clang-retro/llvm and install it
# into the toolchain: compiler-rt's builtins for mos-unknown-unknown, then
# the llvm-mos SDK (the C library, startup code and linker scripts for every
# platform, their mos-PLATFORM-clang drivers and .cfg files, and the mos-sim
# 6502 simulator).
#
#   PREFIX=... LLVM_SRC=... ./build-sdk.sh
#
#   PREFIX    the toolchain directory: PREFIX/bin has clang (with the MOS
#             target), lld, llvm-ar, llvm-nm and llvm-ranlib. The SDK is
#             installed there, because clang looks for a mos-PLATFORM.cfg
#             next to itself, and the builtins go to clang's resource
#             directory.
#   LLVM_SRC  clang-retro/llvm checkout (compiler-rt)
#   WORK      build directory (default: ./build-mos)
#   EXAMPLES  ON to build every platform's examples too (default: OFF)
#
# Needs cmake, ninja and a host C/C++ compiler (CC, CXX; default cc, c++)
# for the simulator and the other host utilities.
set -e
: "${PREFIX:?set PREFIX}" "${LLVM_SRC:?set LLVM_SRC}"
src=$(cd "$(dirname "$0")" && pwd)
WORK=${WORK:-$PWD/build-mos}
mkdir -p "$WORK"
PREFIX=$(cd "$PREFIX" && pwd) WORK=$(cd "$WORK" && pwd)
bin=$PREFIX/bin

"$bin/clang" --target=mos -E -dM -x c /dev/null | grep -q __mos__ || {
  echo "build-sdk.sh: $bin/clang has no MOS target" >&2
  exit 1
}

# The llvm-mos SDK finds the compiler as mos-clang; invoked under that name
# clang defaults to --target=mos.
for l in mos-clang mos-clang++ mos-clang-cpp; do
  [ -e "$bin/$l" ] || ln -s clang "$bin/$l"
done

resource=$("$bin/clang" -print-resource-dir)

# compiler-rt's builtins, which the SDK merges into its libcrt.a.
rm -rf "$WORK/builtins"
cmake -G Ninja -S "$LLVM_SRC/compiler-rt" -B "$WORK/builtins" \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCMAKE_SYSTEM_NAME=Generic \
  -DCMAKE_C_COMPILER="$bin/mos-clang" \
  -DCMAKE_CXX_COMPILER="$bin/mos-clang++" \
  -DCMAKE_ASM_COMPILER="$bin/mos-clang" \
  -DCMAKE_AR="$bin/llvm-ar" -DCMAKE_NM="$bin/llvm-nm" \
  -DCMAKE_RANLIB="$bin/llvm-ranlib" \
  -DCMAKE_C_COMPILER_TARGET=mos-unknown-unknown \
  -DCMAKE_ASM_COMPILER_TARGET=mos-unknown-unknown \
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
  -DCOMPILER_RT_BAREMETAL_BUILD=ON \
  -DCOMPILER_RT_BUILTINS_ENABLE_PIC=OFF \
  -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
  -DCOMPILER_RT_BUILD_BUILTINS=ON -DCOMPILER_RT_BUILD_CRT=OFF \
  -DCOMPILER_RT_BUILD_SANITIZERS=OFF -DCOMPILER_RT_BUILD_XRAY=OFF \
  -DCOMPILER_RT_BUILD_LIBFUZZER=OFF -DCOMPILER_RT_BUILD_PROFILE=OFF \
  -DCOMPILER_RT_BUILD_MEMPROF=OFF -DCOMPILER_RT_BUILD_ORC=OFF \
  -DCOMPILER_RT_BUILD_CTX_PROFILE=OFF \
  -DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON \
  -DCOMPILER_RT_INSTALL_PATH="$resource"
ninja -C "$WORK/builtins" install

# The SDK. It builds its host utilities (mos-sim among them) with the host
# compiler, then itself with mos-clang.
rm -rf "$WORK/sdk"
cmake -G Ninja -S "$src" -B "$WORK/sdk" \
  -DCMAKE_C_COMPILER="${CC:-cc}" -DCMAKE_CXX_COMPILER="${CXX:-c++}" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DLLVM_MOS="$PREFIX" \
  -DLLVM_MOS_BOOTSTRAP_COMPILER=OFF \
  -DLLVM_MOS_BUILD_EXAMPLES="${EXAMPLES:-OFF}"
ninja -C "$WORK/sdk" install
