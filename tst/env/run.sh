#!/bin/sh
# A compiler dedicated to an environment (`make env`): builds one for
# the environment of this directory and compares what it does with
# `expected.out`.
set -e
cd "$(dirname "$0")"
ROOT=$(cd ../.. && pwd)
make -s -C "$ROOT" env OUT="$ROOT/build/atmos-envt" ENV="$PWD/env.atm" ENV_CFLAGS=-DENVT_BASE=40
make -s -C "$ROOT" env OUT="$ROOT/build/atmos-envn" ENV="$PWD/env.atm" ENV_CFLAGS=-DENVT_BASE=40 ENV_NUM="float 32"
T="$ROOT/build/atmos-envt"
N="$ROOT/build/atmos-envn"
A="$ROOT/atmos"
out=$(mktemp)
(
    set +e      # some of the runs are meant to fail
    # the environment comes with the compiler
    $T prog.atm
    # a program that includes it works as well
    $T incl.atm
    # an error names the file of the user
    $T err.atm ; echo "exit $?"
    # the plain compiler does not have it
    $A prog.atm ; echo "exit $?"
    # the option, with no flags and with them
    $A -e env.atm prog.atm 2>&1 | grep -c "flags of the environment are missing"
    ATMOS_CFLAGS='-O1 -DENVT_BASE=10' $A -e env.atm prog.atm
    # with the address sanitizer
    ATMOS_ASAN=1 $T prog.atm
    # a numeric option that the environment does not declare is refused
    ATMOS_TIME=ms $T prog.atm ; echo "exit $?"
    ATMOS_INT=32 $T prog.atm ; echo "exit $?"
    # and one that it declares is taken
    $N prog.atm
    ATMOS_INT=32 ATMOS_REAL=float $N prog.atm
    ATMOS_TIME=ms $N prog.atm ; echo "exit $?"
) > "$out" 2>&1
if diff "$out" expected.out; then
    echo "pass=1 fail=0"
else
    echo "pass=0 fail=1"
    exit 1
fi
