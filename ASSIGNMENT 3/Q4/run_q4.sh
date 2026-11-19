#!/bin/bash
# runs all Q4 experiments (the memory-limit run needs root because it creates a memory cgroup)
cd "$(dirname "$0")"
[ -f /tmp/q4_data.bin ] || { dd if=/dev/urandom of=/tmp/q4_data.bin bs=1M count=1024 status=none; sync; }
./run_pressure.sh > output_pressure.txt 2>&1
{
echo '$ ./touch_order seq 200 ; ./touch_order rand 200   (anonymous memory)'; ./touch_order seq 200; ./touch_order rand 200
echo; echo '$ ./touch_order seq 200 /tmp/q4_data.bin ; ./touch_order rand 200 /tmp/q4_data.bin   (file-backed, file already in page cache)'; ./touch_order seq 200 /tmp/q4_data.bin; ./touch_order rand 200 /tmp/q4_data.bin
echo; echo '$ ./run_pressure.sh   (saved from the run in output_pressure.txt)'; cat output_pressure.txt
echo; echo '$ ./cow_demo'; ./cow_demo
echo; echo '$ ./calc_q4'; ./calc_q4
echo; echo '$ ./faults_frames'; ./faults_frames
} > output_q4.txt 2>&1
