#!/usr/bin/env python3
"""Run the GCC C torture execute tests on the 6502 simulator.

    PREFIX=... TORTURE_SRC=... tests/torture/run-torture.py [-Os] [-O0] ...

PREFIX is the toolchain with the SDK installed (build-sdk.sh). TORTURE_SRC
is SingleSource/Regression/C/gcc-c-torture/execute from llvm-mos's fork of
llvm-test-suite (github.com/llvm-mos/llvm-test-suite), whose CMakeLists.txt
gives the per-test flags. Each test is built with mos-sim-clang and run in
mos-sim; it passes when it exits with 0.

The tests expected to fail at each level are in xfail-LEVEL.txt next to
this script: those the 6502 cannot run (32-bit int, too much stack, nested
functions, x86 asm and so on) and the ones stock llvm-mos fails too. The
script fails on a test that fails unexpectedly, and lists the ones that
pass unexpectedly so the list can be trimmed.
"""
import concurrent.futures
import os
import re
import subprocess
import sys

here = os.path.dirname(os.path.abspath(__file__))
prefix = os.environ['PREFIX']
src = os.environ['TORTURE_SRC']
out_dir = os.environ.get('OUT', os.path.join(os.getcwd(), 'torture-out'))
jobs = int(os.environ.get('JOBS', os.cpu_count() or 1))
levels = sys.argv[1:] or ['-Os']

cmake = open(os.path.join(src, 'CMakeLists.txt')).read()


def glob_list(var):
    m = re.search(r'file\(GLOB ' + var + r' CONFIGURE_DEPENDS(.*?)\n\s*\)',
                  cmake, re.S)
    return set(re.findall(r'[\w.-]+\.c', m.group(1))) if m else set()


extra = {}
for var, flags in [('TestRequiresLibM', ['-lm']),
                   ('TestRequiresWNoReturnType',
                    ['-Wno-return-type', '-Wno-return-mismatch']),
                   ('TestRequiresFWrapV', ['-fwrapv']),
                   ('TestRequiresLibPrintfFlt', ['-lprintf_flt'])]:
    for t in glob_list(var):
        extra.setdefault(t, []).extend(flags)

tests = sorted(f for f in os.listdir(src) if f.endswith('.c'))


def run(level, test):
    exe = os.path.join(out_dir, level, test[:-2])
    cmd = [os.path.join(prefix, 'bin', 'mos-sim-clang'), level, '-w',
           '-Wno-implicit-int', '-Wno-int-conversion',
           '-Wno-implicit-function-declaration', os.path.join(src, test),
           '-o', exe] + extra.get(test, [])
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        if r.returncode != 0:
            return test, 'build', r.stderr[-2000:]
        r = subprocess.run([os.path.join(prefix, 'bin', 'mos-sim'), exe],
                           capture_output=True, text=True, timeout=120)
    except subprocess.TimeoutExpired:
        return test, 'timeout', ''
    if r.returncode != 0:
        return test, 'run', 'exit %d' % r.returncode
    return test, 'pass', ''


status = 0
for level in levels:
    os.makedirs(os.path.join(out_dir, level), exist_ok=True)
    xfail_file = os.path.join(here, 'xfail%s.txt' % level)
    xfail = set()
    if os.path.exists(xfail_file):
        xfail = {l.split()[0] for l in open(xfail_file)
                 if l.strip() and not l.startswith('#')}
    with concurrent.futures.ThreadPoolExecutor(jobs) as ex:
        results = list(ex.map(lambda t: run(level, t), tests))
    passed = [t for t, s, _ in results if s == 'pass']
    failed = [(t, s, m) for t, s, m in results if s != 'pass']
    unexpected = [(t, s, m) for t, s, m in failed if t not in xfail]
    xpass = sorted(set(passed) & xfail)
    with open(os.path.join(out_dir, 'failures%s.txt' % level), 'w') as f:
        for t, s, _ in failed:
            f.write('%s %s\n' % (t, s))
    for t, s, m in unexpected:
        print('FAIL %s %s (%s)' % (level, t, s))
        for line in m.strip().splitlines()[-15:]:
            print('    ' + line)
    for t in xpass:
        print('XPASS %s %s' % (level, t))
    print('== %s: %d passed, %d expected failures, %d unexpected failures, '
          '%d unexpected passes' % (level, len(passed), len(failed) -
                                    len(unexpected), len(unexpected),
                                    len(xpass)))
    if unexpected:
        status = 1
sys.exit(status)
