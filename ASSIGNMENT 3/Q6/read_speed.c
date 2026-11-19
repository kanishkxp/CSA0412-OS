/* read_speed.c - sequential and random read speed of a file, bypassing the page cache with O_DIRECT.
 * Usage: ./read_speed FILE   (FILE must be at least 512 MB)
 * Sequential: 4 MB blocks over 512 MB.  Random: 20,000 reads of 4 KB at random positions. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: %s file\n", argv[0]); return 1; }
    int fd = open(argv[1], O_RDONLY | O_DIRECT); if (fd < 0) { perror("open O_DIRECT"); return 1; }
    void *buf; if (posix_memalign(&buf, 4096, 4 << 20)) return 1;
    double t0 = now(); long total = 0; ssize_t r;
    while (total < (512L << 20) && (r = read(fd, buf, 4 << 20)) > 0) total += r;
    double t1 = now();
    printf("sequential read (4 MB blocks) : %6.0f MB in %.3f s = %8.1f MB/s\n", total / 1048576.0, t1 - t0, total / 1048576.0 / (t1 - t0));
    long blocks = (512L << 20) / 4096; int n = 20000; unsigned long x = 88172645463325252ull;
    t0 = now();
    for (int i = 0; i < n; i++) { x ^= x << 13; x ^= x >> 7; x ^= x << 17; if (pread(fd, buf, 4096, (off_t)(x % (unsigned long)blocks) * 4096) != 4096) { perror("pread"); return 1; } }
    t1 = now();
    printf("random read (4 KB blocks)     : %6d reads in %.3f s = %8.1f MB/s, %8.0f IOPS, %.1f us per read\n", n, t1 - t0, n * 4096.0 / 1048576.0 / (t1 - t0), n / (t1 - t0), (t1 - t0) * 1e6 / n);
    close(fd); return 0;
}
