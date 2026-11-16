# mos-sdk

An SDK for 6502-family computers and consoles, built with clang from
[clang-retro/llvm](https://github.com/clang-retro/llvm), whose MOS backend
comes from [llvm-mos](https://llvm-mos.org). It is the
[llvm-mos SDK](https://github.com/llvm-mos/llvm-mos-sdk) (imported from
upstream commit `555253f`, October 28 2026), with clang-retro's build
script, tests and CI around it.

It has, for each platform:

- the C library, startup code and linker scripts, and C++ without
  exceptions or RTTI;
- a `mos-PLATFORM-clang` driver (and `clang++`, `clang-cpp`) with the
  platform's `.cfg` file;
- examples (in `examples/`).

and `mos-sim`, a 6502 simulator for running programs in tests.

The platforms are the Apple II (ProDOS), Atari 2600, 5200, 8-bit (DOS and
cartridges) and Lynx, Ben Eater's breadboard 6502, the Commander X16,
Commodore 64, 128, PET and VIC-20, GEOS, the MEGA65, CP/M-65, the Dodo,
the NES (NROM, CNROM, UNROM, UNROM-512, GTROM, MMC1, MMC3, Action 53, Famicom
Disk System), the Ohio Scientific Challenger 1P, the Neo6502, the
Picocomputer 6502, the PC Engine (HuCard and CD), the RPC/8e, the
Supervision and the simulator. See
[README.llvm-mos-sdk.md](README.llvm-mos-sdk.md) for the full list with
the variants of each, and the llvm-mos
[wiki](https://llvm-mos.org/wiki/Welcome) for the C ABI, the zero page,
inline assembly and the attributes (`__zeropage`, `leaf`, `interrupt` and
others).

## Using it

    mos-c64-clang -Os hello.c -o hello.prg
    mos-nes-nrom-clang -Os game.c -o game.nes
    mos-sim-clang -Os test.c -o test && mos-sim test

`clang --target=mos` without a platform compiles 6502 code with no C
library. `-mcpu=mos65c02` (and `mosw65816`, `mos65ce02`, `moshuc6280`,
`mosspc700` and the others) selects another member of the family; each
platform's `.cfg` sets its own.

Programs are compiled with link-time optimization by default (`-flto`, in
`mos-common.cfg`): the whole program is optimized together, which suits
the 6502 and programs under 64 KiB.

## Building it

[`build-sdk.sh`](build-sdk.sh) builds compiler-rt's builtins for
`mos-unknown-unknown` and then the SDK, and installs both into the
toolchain:

    PREFIX=/path/to/toolchain LLVM_SRC=/path/to/clang-retro/llvm ./build-sdk.sh

`PREFIX/bin` must hold a clang with the MOS target, lld, llvm-ar, llvm-nm
and llvm-ranlib (the clang-dos packages and the llvm-prebuilt release have
them). The SDK goes into `PREFIX` itself, because clang reads a
`mos-PLATFORM.cfg` from the directory it runs from: `PREFIX/bin` gets the
drivers and the `.cfg` files, `PREFIX/mos-platform` the headers and
libraries. `EXAMPLES=ON` also builds every platform's examples. It needs
cmake, ninja and a host C and C++ compiler (for `mos-sim` and the other
host utilities).

## Tests

[`tests/run-tests.sh`](tests/run-tests.sh) builds the programs in `tests/`
(integer arithmetic at every width, calls and the ABI, the C library,
software floating point, C++) with `mos-sim-clang` at `-Os`, `-O0` and
`-O2`, runs them in `mos-sim` and compares their output with the `.out`
files, which come from the same programs compiled for the host:

    PREFIX=/path/to/toolchain tests/run-tests.sh

[`tests/torture/run-torture.py`](tests/torture/run-torture.py) runs the
GCC C torture `execute` tests (about 1,500 programs that check their own
results) from llvm-mos's fork of
[llvm-test-suite](https://github.com/llvm-mos/llvm-test-suite) the same
way:

    PREFIX=... TORTURE_SRC=.../SingleSource/Regression/C/gcc-c-torture/execute \
      tests/torture/run-torture.py -Os -O0

`tests/torture/xfail-LEVEL.txt` lists the tests expected to fail: the ones
the 6502 cannot run (32-bit `int`, deep stacks, GCC's nested functions,
x86 assembly) and the few that fail with stock llvm-mos as well. With
clang-retro/llvm, 1358 of 1514 pass at `-Os` and 1351 at `-O0`, the same
tests as with stock llvm-mos.

The [CI](.github/workflows/check.yml) builds the SDK with every platform's
examples, and runs both, once clang-retro/llvm's llvm-prebuilt release has
the MOS target.

## Licensing

The parts imported from the llvm-mos SDK (`CMakeLists.txt`, `cmake/`,
`examples/`, `mos-platform/`, `test/`, `utils/`, `README.llvm-mos-sdk.md`)
are under the Apache License v2.0 with LLVM Exceptions, in
[LICENSE.llvm-mos-sdk](LICENSE.llvm-mos-sdk), by the llvm-mos authors
listed in [AUTHORS](AUTHORS). The NES libraries in `mos-platform/nes`
(neslib, nesdoug, FamiTone2) keep their own licenses next to them. The
rest (`build-sdk.sh`, `tests/`, the CI) is under the GNU Affero General
Public License v3.0 only, in [LICENSE](LICENSE).
