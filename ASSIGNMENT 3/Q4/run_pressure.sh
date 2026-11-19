#!/bin/bash
# Creates a 1 GB data file, then runs mem_pressure inside a memory cgroup limited to 256 MB for several working-set sizes.
# Needs root (to create the cgroup). If the cgroup cannot be created the script stops.
cd "$(dirname "$0")"
LIMIT_MB=${LIMIT_MB:-256}; ACC=${ACC:-60000}
CG=/sys/fs/cgroup/memory/q4test
mkdir -p $CG || { echo "cannot create cgroup"; exit 1; }
echo $((LIMIT_MB*1024*1024)) > $CG/memory.limit_in_bytes
[ -f /tmp/q4_data.bin ] || { dd if=/dev/urandom of=/tmp/q4_data.bin bs=1M count=1024 status=none; sync; }
echo "memory limit for the process (cgroup) = ${LIMIT_MB} MB, random reads per run = $ACC"
printf "%8s %11s %12s %12s %13s %10s\n" "work set" "cold pass" "random reads" "per access" "major faults" "fault rate"
printf "%8s %11s %12s %12s %13s %10s\n" "(MB)" "(s)" "(s)" "(us)" "(random)" "(random)"
for MB in 64 128 192 256 320 384 512 768 1024; do
  python3 - <<PY
import os
fd=os.open('/tmp/q4_data.bin',os.O_RDONLY); os.posix_fadvise(fd,0,0,os.POSIX_FADV_DONTNEED); os.close(fd)   # empty the page cache first
PY
  bash -c "echo \$\$ > $CG/cgroup.procs; exec ./mem_pressure /tmp/q4_data.bin $MB $ACC"
done
echo "cgroup failcnt (times the limit was hit): $(cat $CG/memory.failcnt)"
rmdir $CG 2>/dev/null
