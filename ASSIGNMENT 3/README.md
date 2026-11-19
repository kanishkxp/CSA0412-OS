# CSA04 Operating Systems – Assignment 3 (CO3)

**Name:** Kanishk C  **Register number:** 192511278
**Compiler:** gcc 13.3.0  **Distribution:** Ubuntu 24.04 LTS (Linux x86-64)
**Compile flags:** `gcc -std=c11 -Wall -Wextra -O2 ...` (plus `-lm`, `-ldl`, `-static`, `-fPIC -shared` where a program needs them)

The full report is `Assignment3_Report.docx`. Every program in this repository was compiled and run, and the saved output files are the ones used in the report.

> Note: the runs were done on a Linux cloud machine, not on a personal PC. Absolute numbers (times, MB/s) will differ on another machine; the behaviour should be the same.

## How to build and run
```bash
./build_all.sh        # compiles everything, no warnings expected
./run_all.sh          # runs Q1-Q7, rebuilds screenshots and figures (needs python3, matplotlib, pillow)
```
Or one question at a time: `cd Q3 && ./run_q3.sh`.

Test data files are created by the scripts: `/tmp/q4_data.bin` (1 GB, Q4) and `/tmp/q7_table.bin` (64 MB, Q7). The memory-cgroup run in Q4 and the `mlockall` test in Q7 need root (or a high RLIMIT_MEMLOCK).
`tools/mytime.c` replaces `/usr/bin/time -v` (not installed on the test machine); it prints the same counters using `wait4()` rusage.

## Folders and results
| Folder | Content | Result in short |
|---|---|---|
| Q1 | static vs dynamic linking, shared library update, dlopen | static 825,384 B vs dynamic 16,640 B; library fixed without rebuilding the program |
| Q2 | paging: page_translate (TODO completed), page size, fragmentation, page-table types | output identical to `expected_output.txt` |
| Q3 | segmentation: translation, SIGSEGV demo, fragmentation | segment faults reproduced and explained |
| Q4 | demand paging: fault counts, memory pressure (cgroup), copy-on-write, EAT | EAT calculation with 110 µs service time |
| Q5 | page replacement: lru_complete (TODO completed), FIFO/LRU/OPT/Clock, Belady | output identical to `expected_output.txt`; checked by `verify_python.py` |
| Q6 | RAID: capacity, reliability, parity demo, read speed | RAID 6 chosen for the archive; parity cost measured |
| Q7 | edge device: mlockall test and page-replacement simulation | locked loop: 0 major faults; unlocked: about 175,000 |
| tools | report builder, screenshot and figure scripts | – |

Each `Qn/` folder has its sources, run script, output files, `screenshots/` and `figures/`.

## Q7 assumptions
RT task 100 MB, ML weights 1200 MB + 300 MB activations, logging 64 MB, OS 256 MB, control deadline 1 ms. These are my assumptions; only the 2 GB RAM and the 1.5 GB ML size come from the question.

## Commits
See `commit_each_task.sh`: one commit per task.
