#!/bin/bash

# Tests of the command line: each option, and the errors of a wrong
# one (the short forms -c, -o, -t, -e are gone). Each check runs the compiler and
# compares the output (stdout and stderr) and the status.

cd "$(dirname "$0")"

ATMOS=../../atmos
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

cat > "$TMP/p.atm" <<'ATM'
print("args", ...)
test { print("test") }
ATM
cat > "$TMP/e.atm" <<'ATM'
print("env")
ATM
P="$TMP/p.atm"
E="$TMP/e.atm"

pass=0
fail=0

# check <expected status> <expected output> <args...>
check () {
    local st=$1 exp=$2
    shift 2
    local out
    out=$($ATMOS "$@" 2>&1)
    local got=$?
    if [ "$got" == "$st" ] && [ "$out" == "$exp" ]; then
        pass=$((pass+1))
    else
        fail=$((fail+1))
        echo "FAIL: atmos $*"
        echo "  status $got (expected $st)"
        diff <(echo "$out") <(echo "$exp") | head -6
    fi
}

# the first line of the output, for the usage
check1 () {
    local st=$1 exp=$2
    shift 2
    $ATMOS "$@" > "$TMP/out" 2>&1
    local got=$?
    local out
    out=$(head -1 "$TMP/out")
    if [ "$got" == "$st" ] && [ "$out" == "$exp" ]; then
        pass=$((pass+1))
    else
        fail=$((fail+1))
        echo "FAIL: atmos $* (status $got, first line '$out')"
    fi
}

TAB=$'\t'

# run
check 0 "args${TAB}1${TAB}2"        $P 1 2
check 0 "args${TAB}--test"          $P --test
check 0 "args${TAB}-t"              -- $P -t

# tests
check 0 $'args\ntest'               --test $P

# environment
check 0 $'env\nargs'                --env=$E $P
check 0 $'env\nargs'                --env $E $P
check 1 "atmos : cannot read the environment '$TMP/none.atm'" --env=$TMP/none.atm $P

# build only
for o in "--out=$TMP/x1" "--out $TMP/x2"; do
    check 0 "" $o $P
done
for x in x1 x2; do
    out=$("$TMP/$x" z) ; [ "$out" == "args${TAB}z" ] && pass=$((pass+1)) || { fail=$((fail+1)); echo "FAIL: $x"; }
done

# C only
for c in --emit-c; do
    out=$($ATMOS $c $P | grep -c 'AStatus atm_main')
    [ "$out" == "1" ] && pass=$((pass+1)) || { fail=$((fail+1)); echo "FAIL: atmos $c"; }
done

# help and version, to the standard output, with status 0
check1 0 "Usage: atmos [<options>] [--] <input> [<args>] ..." --help
out=$($ATMOS --version 2>/dev/null)
[[ "$out" == "atmos "* ]] && pass=$((pass+1)) || { fail=$((fail+1)); echo "FAIL: --version: $out"; }
ATMOS_FLOAT=bad $ATMOS --help >/dev/null 2>&1 && pass=$((pass+1)) || { fail=$((fail+1)); echo "FAIL: --help with a bad variable"; }

# errors
check1 1 "Usage: atmos [<options>] [--] <input> [<args>] ..."
check1 1 "atmos : unknown option '--xyz'"           --xyz $P
check1 1 "atmos : unknown option '-x'"              -x $P
for o in -c -t; do
    check1 1 "atmos : unknown option '$o'"          $o $P
done
check1 1 "atmos : unknown option '-o'"              -o $TMP/x $P
check1 1 "atmos : unknown option '-e'"              -e $E $P
check1 1 "atmos : missing the value of '--out'"     --out
check1 1 "atmos : missing the value of '--out'"     --out= $P
check1 1 "atmos : missing the value of '--env'"     --env
check1 1 "atmos : missing the input after '--'"     --

echo "pass=$pass fail=$fail"
[ $fail -eq 0 ]
