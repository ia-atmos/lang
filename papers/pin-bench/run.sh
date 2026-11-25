#!/bin/sh
# Runs each program in each variant REPS times; one line per run in results.csv:
# program,size,variant,rep,seconds,peak_rss_kb,exit
# Build first: see README.md. Output of the programs goes to /dev/null.
REPS=${REPS:-5}
echo "program,size,variant,rep,seconds,peak_rss_kb,exit"
for p in binarytrees:16 fannkuch:9 fasta:1000000 fib:35 mandelbrot:1000 nbody:500000 spectralnorm:500; do
    n=${p%%:*}; a=${p##*:}
    for r in $(seq 1 $REPS); do
        for v in c lua54 luajit off unchecked on; do
            case $v in
                c)      cmd="c/$n $a" ;;
                lua54)  cmd="lua5.4 lua/$n.lua $a" ;;
                luajit) cmd="luajit lua/$n.lua $a" ;;
                *)      cmd="bin/$n-$v $a" ;;
            esac
            set -- $(./runm $cmd)
            echo "$n,$a,$v,$r,$1,$2,$3"
        done
    done
done
