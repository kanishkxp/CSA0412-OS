#!/bin/bash
# Task 5: collect the strace evidence used in the report.
gcc -std=c11 -Wall -Wextra -o hello_write hello_write.c
strace -o hello_write.strace ./hello_write
echo "--- only the file related calls of hello_write:"
grep -E "openat\(AT_FDCWD, \"patient|read\(3|write\(|close\(3|unlink|exit_group" hello_write.strace
echo "--- system call summary of 'ls /':"
strace -c -o ls_summary.txt ls / > /dev/null; cat ls_summary.txt | head -12
echo "--- the read() of 'cat' on a small file, with timestamps and durations:"
echo "hello hospital" > small.txt
strace -T -e trace=openat,read,write,close cat small.txt 2>&1 | tail -8
rm -f small.txt
