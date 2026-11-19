#!/bin/bash
# runs the Q7 experiments: real memory-locking test (3 rounds) and the edge-device simulation
cd "$(dirname "$0")"
{ for i in 1 2 3; do ./make_table.sh >/dev/null; ./rt_lock_test unlocked 20000 1000; echo; ./make_table.sh > /dev/null; ./rt_lock_test locked 20000 1000; echo "-----"; done; } > output_rt_lock.txt 2>&1
./edge_sim > output_edge_sim.txt
