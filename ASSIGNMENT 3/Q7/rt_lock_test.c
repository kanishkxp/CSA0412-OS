/* rt_lock_test.c - worst-case delay of a time-critical loop with and without memory locking.
 *
 * The control task reads a 64 MB calibration table that is mapped from a file (like a model/config file on the edge device).
 * Every iteration it reads 16 random entries and does a little arithmetic; the time of every iteration is recorded.
 * Every PRESSURE iterations we simulate memory pressure: madvise(MADV_PAGEOUT) asks the kernel to reclaim the pages of the
 * table, exactly what the kernel would do when RAM is short. Pages that are locked with mlock() cannot be reclaimed.
 *   unlocked : plain mapping            -> reclaimed pages must be read again from storage (major page fault)
 *   locked   : mlockall + pre-faulting -> pages stay in RAM, no page fault in the loop
 * Usage: ./rt_lock_test unlocked|locked [iterations] [pressure_every]
 * Compile: gcc -std=c11 -Wall -Wextra -O2 -o rt_lock_test rt_lock_test.c */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/resource.h>

#ifndef MADV_PAGEOUT
#define MADV_PAGEOUT 21
#endif
#define TABLE_MB 64
static double now_us(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e6 + t.tv_nsec / 1e3; }
static int cmpd(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return (x > y) - (x < y); }

int main(int argc, char **argv)
{
    int locked = argc > 1 && strcmp(argv[1], "locked") == 0;
    long iters = argc > 2 ? atol(argv[2]) : 20000, every = argc > 3 ? atol(argv[3]) : 1000;
    size_t bytes = (size_t)TABLE_MB << 20, pages = bytes / 4096;
    int fd = open("/tmp/q7_table.bin", O_RDONLY); if (fd < 0) { perror("open /tmp/q7_table.bin (run make_table.sh first)"); return 1; }
    volatile unsigned char *tab = mmap(NULL, bytes, PROT_READ, MAP_PRIVATE, fd, 0);
    if (tab == MAP_FAILED) { perror("mmap"); return 1; }
    if (locked) {
        if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) { perror("mlockall"); return 1; }   /* lock now and everything mapped later */
        puts("mode: LOCKED   (mlockall(MCL_CURRENT|MCL_FUTURE), table pre-faulted)");
    } else puts("mode: UNLOCKED (ordinary pageable memory)");
    unsigned sink = 0; for (size_t i = 0; i < pages; i++) sink += tab[i * 4096];           /* pre-fault: bring every page in once */

    double *lat = calloc((size_t)iters, sizeof(double)); unsigned long x = 88172645463325252ull;
    struct rusage r0, r1; getrusage(RUSAGE_SELF, &r0);
    for (long it = 0; it < iters; it++) {
        if (it % every == 0) madvise((void *)tab, bytes, MADV_PAGEOUT);                   /* "memory pressure" (not timed) */
        double t0 = now_us();
        for (int k = 0; k < 16; k++) { x ^= x << 13; x ^= x >> 7; x ^= x << 17; sink += tab[(x % pages) * 4096 + (x & 63)]; }
        lat[it] = now_us() - t0;
    }
    getrusage(RUSAGE_SELF, &r1);
    double sum = 0, mx = 0; for (long i = 0; i < iters; i++) { sum += lat[i]; if (lat[i] > mx) mx = lat[i]; }
    double *sorted = malloc(sizeof(double) * (size_t)iters); memcpy(sorted, lat, sizeof(double) * (size_t)iters); qsort(sorted, (size_t)iters, sizeof(double), cmpd);
    long over = 0; for (long i = 0; i < iters; i++) if (lat[i] > 1000.0) over++;
    unsigned char *vec = malloc(pages); mincore((void *)tab, bytes, vec); size_t res = 0; for (size_t i = 0; i < pages; i++) res += vec[i] & 1;
    printf("iterations %ld, memory pressure every %ld iterations\n", iters, every);
    printf("loop time per iteration: mean %.2f us | median %.2f us | 99.9%% %.2f us | WORST CASE %.2f us\n", sum / (double)iters, sorted[iters / 2], sorted[(long)(iters * 0.999)], mx);
    printf("iterations slower than 1 ms (a typical control deadline): %ld\n", over);
    printf("page faults inside the loop: major %ld, minor %ld | table pages resident at the end: %zu of %zu\n", r1.ru_majflt - r0.ru_majflt, r1.ru_minflt - r0.ru_minflt, res, pages);
    return (int)(sink & 0);
}
