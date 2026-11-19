/* mem_pressure.c - run time when the working set grows beyond the RAM that is available to the process.
 * The data file is mapped with mmap(); pages are brought in on demand (demand paging). The process is run inside a
 * memory cgroup with a fixed limit (see run_pressure.sh), so when the working set is larger than the limit the
 * kernel has to throw pages out and read them again from disk (page replacement).
 * Usage: ./mem_pressure FILE WORKING_SET_MB ACCESSES
 * Phase 1: read every page of the working set once (cold start).  Phase 2: ACCESSES random page reads. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/resource.h>

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
int main(int argc, char **argv)
{
    if (argc < 4) { fprintf(stderr, "usage: %s file MB accesses\n", argv[0]); return 1; }
    size_t mb = (size_t)atol(argv[2]), bytes = mb << 20, pg = 4096, n = bytes / pg; long acc = atol(argv[3]);
    int fd = open(argv[1], O_RDONLY); if (fd < 0) { perror("open"); return 1; }
    volatile unsigned char *p = mmap(NULL, bytes, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED) { perror("mmap"); return 1; }
    struct rusage r0, r1, r2; unsigned sum = 0;
    getrusage(RUSAGE_SELF, &r0); double t0 = now();
    for (size_t i = 0; i < n; i++) sum += p[i * pg];                   /* phase 1: sequential, cold */
    double t1 = now(); getrusage(RUSAGE_SELF, &r1);
    unsigned long x = 88172645463325252ull;
    for (long i = 0; i < acc; i++) { x ^= x << 13; x ^= x >> 7; x ^= x << 17; sum += p[(x % n) * pg]; }   /* phase 2: random */
    double t2 = now(); getrusage(RUSAGE_SELF, &r2);
    long maj2 = r2.ru_majflt - r1.ru_majflt;
    printf("%8zu %11.2f %12.3f %12.2f %13ld %9.2f%%\n", mb, t1 - t0, t2 - t1, (t2 - t1) * 1e6 / (double)acc, maj2, 100.0 * (double)maj2 / (double)acc);
    return (int)(sum & 0);
}
