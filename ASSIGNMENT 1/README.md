# CSA04 - Operating Systems - Assignment 1

**Title:** Evaluate and Justify the Operating-System Design of a Smart Hospital

| | |
|---|---|
| Name | Kanishk C |
| Register number | 192511278 |
| Course | CSA04 - Operating Systems |
| Compiler | gcc 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) |
| Linux distribution | Ubuntu 24.04.5 LTS (kernel 6.18, 1 virtual CPU, 4 GB RAM) |
| Other tools | strace 6.8, bash, Python 3 (only for the small calculation scripts and the charts) |

> All outputs in this repository were produced by running the programs on the machine above.
> If you run them on a different computer the exact times will be different, but the system-call
> counts and the trends should be the same.

## How to compile

Every C file is compiled with the same command:

```
gcc -std=c11 -Wall -Wextra -pthread -o <name> <name>.c
```

or compile everything at once with `./build_all.sh`.

## Folder layout

| Folder | Contents |
|---|---|
| Task01 | `wakeup_latency.c` - wake-up delay of a 1 ms task under load, normal versus real-time scheduler |
| Task02 | `availability.py` - downtime of one big server versus a two-node cluster, and processor speed-up |
| Task03 | `syscall_vs_ipc.c` - function call versus system call versus message passing |
| Task04 | `protection_demo.sh` - the OS stops one user reading another user's record |
| Task05 | `hello_write.c`, `stdio_vs_syscall.c`, `strace_commands.sh` - system calls seen with strace |
| Task06 | `copy_file.c` (the completed code), `copy_bench.c`, `run_benchmark.sh` - buffer size and system calls |
| Task07 | `shell_jobs.c` (the completed code), `shell_jobs_noflush.c`, `spawn_bench.c` |
| Task08 | `mode_protection.c`, `polling_vs_interrupt.c` |
| Task09 | `memlat.c`, `io_calculation.py` |
| Task10 | `fork_vs_thread.c`, `alive_memory.c`, `time_copy.sh` |
| Task11 | notes for the final plan (no program) |
| Task12 | notes for the design challenge (no program) |
| screenshots | PNG pictures of the terminal output of each program (drawn by `make_screenshots.py` from the saved output files) |
| figures | Charts and diagrams used in the report (`make_charts.py`, `make_diagrams.py`) |

## How to run each program and what it showed

**Task 1** - `cd Task01 && gcc -std=c11 -Wall -Wextra -pthread -o wakeup_latency wakeup_latency.c && ./wakeup_latency normal 5000 && ./wakeup_latency rt 5000`
(the `rt` run needs root). With four CPU-hungry programs running, the normal scheduler woke the 1 ms task up to about 6 ms late
(99th percentile about 3.8 ms). The real-time scheduler kept the 99th percentile at about 70 microseconds.

**Task 2** - `python3 Task02/availability.py`. One server with 99.6 % availability is down about 35 hours a year; a two-node cluster with fail-over is down about 9 minutes a year (under the stated assumptions).

**Task 3** - `cd Task03 && gcc ... -o syscall_vs_ipc syscall_vs_ipc.c && ./syscall_vs_ipc`.
A function call took about 1.4 ns, a system call about 118 ns, and a message round trip between two processes about 2.8 microseconds (about 24 times a system call).

**Task 4** - `sudo ./Task04/protection_demo.sh`. The owner can read a file with mode 600; another user gets "Permission denied" (EACCES).

**Task 5** - `cd Task05 && ./strace_commands.sh`. Shows `openat`, `read`, `write`, `close` and `exit_group` for a real program, and the effect of library buffering.

**Task 6 (Complete the Code)** - `cd Task06 && gcc -std=c11 -Wall -Wextra -pthread -o copy_file copy_file.c && ./copy_file`.
Output matches the expected output: 10001, 21 and 4 `read()` calls. `strace -c -e trace=read,write ./copy_file` reports 10027 reads;
this is 10001 + 21 + 4 plus one read made by the dynamic loader. `./run_benchmark.sh` creates `benchmark_results.csv`.

**Task 7 (Complete the Code)** - `cd Task07 && gcc -std=c11 -Wall -Wextra -pthread -o shell_jobs shell_jobs.c && ./shell_jobs`.
Output matches the expected output (42, exit status 0, exit status 3).
`shell_jobs_noflush` has the second `fflush` removed and `_exit(3)` changed to `exit(3)`. With the output redirected to a file the text "shell: running job 2" appears twice.
`spawn_bench.c` compares fork+exec, vfork+exec and posix_spawn.

**Task 8** - `./mode_protection cli|kernel|null` shows that a user program cannot run `cli` or touch kernel memory (it gets SIGSEGV).
`./polling_vs_interrupt busy|p10|intr` compares busy polling, polling every 10 ms and sleeping until woken.

**Task 9** - `./memlat > memlat_results.csv` (takes about 1 minute) gives the access time for working sets of 4 KB to 256 MB. `python3 io_calculation.py` compares interrupt-driven I/O with DMA for a 500 MB image.

**Task 10** - `./fork_vs_thread 1000 0` (and with 100 and 400 for the size of the parent's heap in MB) and `./alive_memory proc 1000` / `./alive_memory thread 1000`.
Creating 1000 processes took about 70-100 ms with a small parent and several seconds with a 400 MB parent; 1000 threads took about 10-30 ms in every case.

## Commit history

One commit was made for each task (see `git log`).
