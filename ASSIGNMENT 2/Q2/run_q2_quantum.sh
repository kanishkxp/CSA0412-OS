#!/bin/bash
# Builds q2_rr.c with different time quanta (only the "int quantum" line changes)
# and prints the result of each run together with the number of context switches.
# A context switch is counted only when two neighbouring time slices belong to
# different processes (a process that keeps the CPU for the next slice is not a switch).
for q in 1 3 5 20; do
    sed "s/int quantum = 3;/int quantum = $q;/" q2_rr.c > /tmp/q2_rr_q$q.c
    gcc -std=c11 -Wall -Wextra -o /tmp/q2_rr_q$q /tmp/q2_rr_q$q.c || exit 1
    echo "=============== quantum = $q ==============="
    out=$(/tmp/q2_rr_q$q)
    echo "$out"
    echo "$out" | grep -o 'P[0-9]*([0-9]*-[0-9]*)' | sed 's/(.*//' | \
        awk 'NR>1 && $0!=prev {sw++} {prev=$0; n++} END {print "time slices = " n ", context switches = " sw+0}'
done
