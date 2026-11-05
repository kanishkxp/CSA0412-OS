#!/bin/bash
# Task 6 / Task 10: copy a file with different buffer sizes and record the time.
# usage: ./run_benchmark.sh      (needs copy_bench built with: gcc -std=gnu11 -O0 -Wall -Wextra -o copy_bench copy_bench.c)
set -e
head -c 1048576  /dev/urandom > file_1MB.dat
head -c 67108864 /dev/urandom > file_64MB.dat
echo "file,mode,bufsize,run,reads,real_s,user_s,sys_s" > benchmark_results.csv
run() {  # file mode bufsize
  for r in 1 2 3 4 5; do
    out=$(./copy_bench $2 $1 /tmp/bench_out.dat $3)
    reads=$(echo "$out" | sed 's/.*reads=\([0-9]*\).*/\1/')
    real=$(echo "$out"  | sed 's/.*real=\([0-9.]*\).*/\1/')
    user=$(echo "$out"  | sed 's/.* user=\([0-9.]*\).*/\1/')
    sys=$(echo "$out"   | sed 's/.* sys=\([0-9.]*\).*/\1/')
    echo "$1,$2,$3,$r,$reads,$real,$user,$sys" >> benchmark_results.csv
  done
}
for b in 1 16 64 512 4096 65536 1048576; do run file_1MB.dat sys $b; done
for b in 512 4096 16384 65536 262144 1048576; do run file_64MB.dat sys $b; done
for b in 4096 65536; do run file_64MB.dat stdio $b; done
run file_64MB.dat mmap 0
rm -f /tmp/bench_out.dat file_1MB.dat file_64MB.dat
