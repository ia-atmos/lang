#!/bin/bash

# Runs the golden tests.
#
#   tst/run.sh            all suites
#   tst/run.sh lex        one suite (lex, ast, cases, golden, guide)
#
# Each case is a `NAME.atm` with the expected output in `NAME.out`
# (stdout followed by stderr). An address, as in `function: 0x5040...`,
# is compared as `function: 0x?`.

cd "$(dirname "$0")"

ATMOS=../../atmos

suites=${@:-lex ast cases golden}

# programs are built with the address sanitizer (leaks included)
export ATMOS_ASAN=1
export ATMOS_CFLAGS=-O0
# the objects of the modules (`import`) stay in the build
export ATMOS_CACHE=$PWD/../build/tst/cache

pass=0
fail=0

for suite in $suites; do
    case $suite in
        lex) flags="--lex" ;;
        ast) flags="--ast" ;;
        cases)  flags="" ;;
        golden) flags="" ;;
        guide)  flags="" ;;
        *)   echo "unknown suite: $suite" ; exit 1 ;;
    esac
    cd $suite
    for atm in *.atm; do
        name=${atm%.atm}
        # extra flags of the case
        more=""
        if [ -f $name.flags ]; then
            more=$(cat $name.flags)
        fi
        # variables of the environment of the case, as `ATMOS_FLOAT=32`
        vars=""
        if [ -f $name.env ]; then
            vars=$(cat $name.env)
        fi
        # addresses change from one build to another
        out=$(env $vars $ATMOS $flags $more $atm 2>&1 | sed -E 's/: 0x[0-9a-f]+/: 0x?/g')
        if [ "$out" == "$(cat $name.out)" ]; then
            pass=$((pass+1))
        else
            fail=$((fail+1))
            echo "FAIL: $suite/$name"
            diff <(echo "$out") $name.out | head -10
        fi
    done
    cd ..
done

echo "pass=$pass fail=$fail"
[ $fail -eq 0 ]
