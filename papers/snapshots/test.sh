#!/bin/sh
# The test of the example of the paper on snapshots: compiles and runs
# `board.atm` with the compiler of this repository (with the address
# sanitizer), and compares its output
# with `expected.txt`.
set -e
cd "$(dirname "$0")"
make -s -C ../.. >/dev/null
ATMOS_ASAN=1 ../../atmos board.atm > "${TMPDIR:-/tmp}/snapshots-board.out" 2>&1
diff -u expected.txt "${TMPDIR:-/tmp}/snapshots-board.out"
echo "papers/snapshots: ok"
