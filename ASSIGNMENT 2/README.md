# CSA04-192511278-Assignment2

**Course:** CSA04 – Operating Systems, Assignment 2 (CO2)
**Name:** Kanishk C
**Register number:** 192511278
**Institution:** SIMATS Engineering, Saveetha Institute of Medical and Technical Sciences

This repository holds the code, command outputs and pictures for all six tasks of Assignment 2
(processes, CPU scheduling, threads, inter-process communication, synchronisation, deadlock handling).
The written report is in `Kanishk_C_192511278_OS_Assignment2.docx`.

## Environment

| Item | Value |
|---|---|
| Linux distribution | Ubuntu 24.04.5 LTS |
| Kernel | 6.18.44 |
| Compiler | gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0 |
| Machine | 2 virtual CPUs (Intel Xeon @ 2.10 GHz), 8 GB RAM (a cloud Linux container) |
| Other tools | GNU make, Python 3 with matplotlib and Pillow (only for the pictures) |

All timings in the report were measured on this machine. It has **2 cores**, which matters for the
thread speed-up in Q3 and for the locking results in Q5.

## How to compile

The exact flags are in the `Makefile`. In short:

```
gcc -std=c11 -Wall -Wextra -pthread [-O2] -o <program> <program>.c
```

Three programs need GNU extensions and use `-std=gnu11` instead (`Q1/q1_ctxswitch.c` for
`sched_setaffinity`, `Q3/q3_shared.c` for `gettid`, `Q4/q4_bench.c` for POSIX message queues, which also needs `-lrt`).
The programs that are timed also use `-O2`.

```
make          # builds all 30 programs, no warnings
make Q3       # builds only the programs of one task
make clean    # removes the binaries
```

## Folders

Every task has its own folder with the C source files, shell scripts, the captured command outputs
(`*_output.txt`) and a `screenshots/` folder with diagrams, charts and terminal output pictures.
The terminal pictures are drawn by `tools/make_terminal_images.py` from the saved output files,
and the charts by `tools/make_figures.py` from the measured numbers.

## Task by task: how to run, and what came out

### Q1 – Processes and process states (`Q1/`)

| Program | Run | What it does |
|---|---|---|
| `q1_fork.c` | `./q1_fork` | Creates 4 children with `fork()`; shows `ps` and `/proc/<pid>/status` while they run, and zombies before `wait()` |
| `q1_compare.c` | `./q1_compare process 2000` or `thread 2000` | Time to create 2,000 sessions as processes or as threads |
| `q1_memory.c` | `./q1_memory process 2000` | Memory used by 2,000 live sessions |
| `q1_ctxswitch.c` | `./q1_ctxswitch process` or `thread` | Cost of one context switch (two tasks on the same CPU, pipe ping-pong) |

**Result:** the `ps` output showed state R for the busy child, S for the three sleeping children and Z for finished
children that were not yet waited for. Creating a process took about 105 µs against 32 µs for a thread, and a live
process session used about 369 KB against 278 KB for a thread session. A context switch cost about 1.3 to 1.7 µs for both.
Design decision: a fixed pool of worker processes, each handling many sessions.

### Q2 – CPU scheduling (`Q2/`)

| Program | Run | What it does |
|---|---|---|
| `q2_rr.c` | `./q2_rr` | The given Round Robin program with TODO 1 completed |
| `run_q2_quantum.sh` | `./run_q2_quantum.sh` | Rebuilds `q2_rr.c` with quantum 1, 3, 5 and 20 and counts context switches |
| `q2_schedulers.c` | `./q2_schedulers` | FCFS, SJF, SRTF, priority, Round Robin and MLFQ; quantum study; video-service workload; starvation test |
| `q2_decision_cost.c` | `./q2_decision_cost` | Time of one scheduling decision: SJF scan vs SJF heap vs Round Robin queue |

**Result:** `q2_rr` prints exactly the expected output (average TAT 20.00, average WT 13.50). Average waiting time
for quantum 1 / 3 / 5 / 20 was 12.50 / 13.50 / 11.00 / 8.75 and context switches were 23 / 9 / 5 / 3.
SRTF gave the best average waiting time (6.50) for the four jobs. Recommended policy for the video service: MLFQ with ageing.

### Q3 – Threads (`Q3/`)

| Program | Run | What it does |
|---|---|---|
| `q3_threads.c` | `./q3_threads serial` / `per-camera` / `pool 4` | 64 cameras: serial, one thread per camera, thread pool of N |
| `q3_procs.c` | `./q3_procs 4` | Same work with N processes and a pipe to collect results |
| `q3_shared.c` | `./q3_shared` | Prints which addresses threads share and which are private |
| `q3_race.c` | `./q3_race` | Two threads on one counter: no protection, mutex, atomic |

**Result:** serial 2.76 s; every parallel version (4, 8, 16, 64 threads and processes) about 1.38 s, a speed-up of
about 2.0. It stops at 2 because the machine has 2 cores. The unprotected counter lost 3.6 million of 10 million updates.

### Q4 – Inter-process communication (`Q4/`)

| Program | Run | What it does |
|---|---|---|
| `q4_pipe.c` | `./q4_pipe` | The given weather pipeline with TODO 1 completed |
| `q4_pipe_noclose.c` | `timeout 5 ./q4_pipe_noclose` | Same program with `close(fd[1])` removed from the parent: it hangs |
| `q4_shm.c` | `./q4_shm` | Same pipeline with shared memory and semaphores |
| `q4_bench.c` | `./q4_bench pipe|shm|mq 100` | Time to move 1, 10 or 100 MB between two processes |
| `q4_readings.c` | `./q4_readings pipe|shm 100000` | Time to move n single readings |

**Result:** both versions print `analyser: 6 readings, average 35.5, max 41`. Without `close(fd[1])` the program was
still blocked after 5 s (the child waits in `read`). For 100 MB: shared memory 58 ms, message queue 75 ms, pipe 130 ms.
For many tiny readings the pipe was as fast or faster than shared memory, because of the semaphore cost per reading.

### Q5 – Synchronisation (`Q5/`)

| Program | Run | What it does |
|---|---|---|
| `q5_bounded_buffer.c` | `./q5_bounded_buffer` | The given bounded buffer with TODO 1 and TODO 2 completed |
| `run_q5_100times.sh` | `./run_q5_100times.sh` | Runs it 100 times |
| `q5_bb_multi.c` | `./q5_bb_multi 4 4` (and `q5_bb_multi_nomutex`) | Many producers and consumers; checks every item is consumed exactly once |
| `q5_peterson.c` | `./q5_peterson_plain`, `./q5_peterson_atomic` | Peterson's solution with plain variables and with atomics |
| `q5_philosophers.c` | `./q5_philosophers`, `timeout 5 ./q5_philosophers_naive` | Dining philosophers, safe and deadlocking |
| `q5_readers_writers.c` | `./q5_readers_writers`, `./q5_rw_nosync` | Readers-writers with and without semaphores |
| `q5_monitor.c` | `./q5_monitor 4 4` | Bounded buffer as a monitor (mutex and condition variables) |
| `q5_booking.c` | `./q5_booking coach 8 50000 16 8 72 1000` | Railway booking with no lock, one global lock, a lock per train, a lock per coach |
| `q5_counter_compare.c` | `./q5_counter_compare mutex 4 2000000 0` | Mutex, semaphore, spin lock and atomic add compared |

**Result:** the bounded buffer printed the expected `sum = 210` in all 100 runs. Without the mutex, 1 producer and
1 consumer were still correct (0 wrong in 100 runs), but 4 producers and 4 consumers gave wrong results in 98 of 100 runs
(items lost and duplicated). With the mutex there were 0 wrong results in 300 runs. With 8 threads,
a lock per coach gave about 2.5 times the throughput of one global lock. Recommended: a lock per coach.

### Q6 – Deadlock handling and the Banker's algorithm (`Q6/`)

| Program | Run | What it does |
|---|---|---|
| `q6_banker.c` | `./q6_banker` | The given Banker's algorithm with TODO 1 completed |
| `q6_bench.c` | `./q6_bench` | Times the safety algorithm for n = 10 to 2,000 and different m; cost of one request |
| `q6_detect.c` | `./q6_detect` | Deadlock detection and recovery for the research-centre cluster |

**Result:** the output is exactly the expected one (safe sequence P1 P3 P4 P0 P2, P4 must wait, P0 denied as unsafe).
The safety check took 0.19 µs, 15.8 µs and 1,231 µs for 10, 100 and 1,000 processes in the worst case, which fits the
predicted O(m·n²) growth. Recommended: avoidance only where maximum demands are known, otherwise detection and recovery.

## Re-creating the pictures

```
python3 tools/make_figures.py            # diagrams and charts
python3 tools/make_terminal_images.py    # terminal-style output pictures
```
