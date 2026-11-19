/* touch_order.c - touch memory in sequential or random order and count the page faults.
 * Usage: ./touch_order seq|rand SIZE_MB [FILE]
 * With FILE the pages are read from a file mapped with mmap (file-backed) instead of anonymous memory.
 * Memory is mapped with mmap (nothing is in RAM yet - demand paging), then one byte of every page is written once. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <fcntl.h>

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
int main(int argc, char **argv)
{
    if (argc < 3) { fprintf(stderr, "usage: %s seq|rand MB\n", argv[0]); return 1; }
    int seq = strcmp(argv[1], "seq") == 0; size_t mb = (size_t)atol(argv[2]), bytes = mb << 20;
    size_t pg = (size_t)sysconf(_SC_PAGESIZE), n = bytes / pg;
    int fd = -1; if (argc > 3) { fd = open(argv[3], O_RDONLY); if (fd < 0) { perror("open"); return 1; } }
    volatile char *p = fd >= 0 ? mmap(NULL, bytes, PROT_READ, MAP_PRIVATE, fd, 0)
                               : mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) { perror("mmap"); return 1; }
    size_t *order = malloc(n * sizeof *order);
    for (size_t i = 0; i < n; i++) order[i] = i;
    if (!seq) { srand(5); for (size_t i = n - 1; i > 0; i--) { size_t j = ((size_t)rand() << 15 ^ (size_t)rand()) % (i + 1); size_t t = order[i]; order[i] = order[j]; order[j] = t; } }
    struct rusage a, b; getrusage(RUSAGE_SELF, &a); double t0 = now();
    unsigned sink = 0;
    for (size_t i = 0; i < n; i++) { if (fd >= 0) sink += (unsigned char)p[order[i] * pg]; else p[order[i] * pg] = 1; }
    double t1 = now(); getrusage(RUSAGE_SELF, &b);
    long minor = b.ru_minflt - a.ru_minflt, major = b.ru_majflt - a.ru_majflt;
    printf("%s %-4s %5zu MB: %7zu pages touched, minor faults = %7ld, major faults = %ld, time = %7.1f ms (%.2f us per page)\n",
           fd >= 0 ? "file-backed:" : "anonymous  :", argv[1], mb, n, minor, major, (t1 - t0) * 1e3, (t1 - t0) * 1e6 / (double)n);
    return (int)(sink & 0);
}
