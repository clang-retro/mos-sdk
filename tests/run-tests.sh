#!/bin/sh
# Build the tests with each mos-sim driver at -Os, -O0 and -O2, run them in
# mos-sim, the SDK's 6502 simulator, and compare their output with
# NAME.out.
#
#   PREFIX=... tests/run-tests.sh
#
#   PREFIX  the toolchain directory, with the SDK installed by build-sdk.sh
#   OUT     output directory (default: ./tests-out)
#   OPTS    optimization levels (default: -Os -O0 -O2)
set -e
: "${PREFIX:?set PREFIX}"
here=$(cd "$(dirname "$0")" && pwd)
OUT=${OUT:-$PWD/tests-out}
mkdir -p "$OUT"
bin=$PREFIX/bin
pass=0 fail=0

for src in "$here"/*.c "$here"/*.cc; do
  name=$(basename "$src")
  name=${name%.*}
  case $src in
  *.cc) cc=$bin/mos-sim-clang++ ;;
  *) cc=$bin/mos-sim-clang ;;
  esac
  libs=
  [ "$name" = float ] && libs=-lprintf_flt
  for opt in ${OPTS:--Os -O0 -O2}; do
    exe=$OUT/$name$opt
    if ! "$cc" $opt -Wall -Werror "$src" -o "$exe" $libs 2>"$exe.log"; then
      echo "FAIL build $name $opt"
      sed 's/^/    /' "$exe.log"
      fail=$((fail + 1))
      continue
    fi
    if "$bin/mos-sim" "$exe" >"$exe.out" 2>&1 && cmp -s "$here/$name.out" "$exe.out"; then
      echo "PASS $name $opt"
      pass=$((pass + 1))
    else
      echo "FAIL run $name $opt"
      diff "$here/$name.out" "$exe.out" | sed 's/^/    /' || true
      fail=$((fail + 1))
    fi
  done
done

echo "== $pass passed, $fail failed"
[ "$fail" -eq 0 ]
