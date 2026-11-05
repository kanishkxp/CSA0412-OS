#!/bin/bash
# Task 10: user time and kernel (system) time of the copy program for different buffer sizes.
# Uses the copy_bench program of Task06 and the bash "time" keyword.
BENCH=../Task06/copy_bench
head -c 4194304 /dev/urandom > /tmp/t10_src.dat      # 4 MB test file
TIMEFORMAT="real %3Rs   user %3Us   sys %3Ss"
for b in 1 64 512 4096 65536; do
  echo "--- buffer $b bytes (4 MB file)"
  time $BENCH sys /tmp/t10_src.dat /tmp/t10_dst.dat $b > /dev/null
done
rm -f /tmp/t10_src.dat /tmp/t10_dst.dat
