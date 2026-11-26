#!/bin/sh
# instr.sh : instructions per iteration of each operation, by callgrind:
# the difference between a run of 2 million and a run of 1 million
# iterations, divided by 1 million. One line per operation:
#   op  atmos-in-a-function  atmos-in-the-main-body  lua5.4
# Requires valgrind and lua5.4. ATMOS is the compiler (default
# ../../../atmos); the programs are built with ATMOS_PIN=off and -O2.
ATMOS=${ATMOS:-../../../atmos}
T=${TMPDIR:-/tmp}/atm-micro-$$
ir() { valgrind --tool=callgrind --callgrind-out-file=/dev/null "$@" 2>&1 >/dev/null | grep Collected | awk '{print $NF}'; }
ops=$(python3 -I gen.py $T/1 1000000)
python3 -I gen.py $T/2 2000000 >/dev/null
for k in $ops; do
    r="$k"
    for v in f t; do
        for n in 1 2; do
            ATMOS_PIN=off ATMOS_CFLAGS=-O2 $ATMOS -o $T/$n/$k-$v $T/$n/$k-$v.atm || exit 1
        done
        a=$(ir $T/1/$k-$v); b=$(ir $T/2/$k-$v); r="$r $(( (b-a)/1000000 ))"
    done
    a=$(ir lua5.4 $T/1/$k.lua); b=$(ir lua5.4 $T/2/$k.lua); r="$r $(( (b-a)/1000000 ))"
    echo $r
done
rm -rf $T
