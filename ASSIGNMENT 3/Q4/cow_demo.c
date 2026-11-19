/* cow_demo.c - copy-on-write with fork().
 * The parent allocates and fills 200 MB. After fork() parent and child share the same physical frames (marked read-only).
 * Only when somebody WRITES is the page copied. We measure with /proc/self/smaps_rollup and /proc/meminfo.
 *   Rss  = pages mapped by this process      Pss = Rss where shared pages are divided among the sharers
 *   Private_Dirty = pages owned only by this process (a copy was made)               */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define MB 200
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }
static long field(const char *file, const char *key)
{
    FILE *f = fopen(file, "r"); char line[256]; long v = -1;
    while (f && fgets(line, sizeof line, f)) if (strncmp(line, key, strlen(key)) == 0) { sscanf(line + strlen(key), " %ld", &v); break; }
    if (f) fclose(f);
    return v;
}
static void show(const char *who, const char *when)
{
    printf("%-7s %-34s Rss=%7ld kB  Pss=%7ld kB  Shared_Dirty=%7ld kB  Private_Dirty=%7ld kB | system MemAvailable=%8ld kB\n", who, when,
           field("/proc/self/smaps_rollup", "Rss:"), field("/proc/self/smaps_rollup", "Pss:"), field("/proc/self/smaps_rollup", "Shared_Dirty:"),
           field("/proc/self/smaps_rollup", "Private_Dirty:"), field("/proc/meminfo", "MemAvailable:"));
}
int main(void)
{
    size_t bytes = (size_t)MB << 20, pg = 4096, n = bytes / pg;
    char *p = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memset(p, 1, bytes);
    show("parent", "after filling 200 MB");
    fflush(stdout);
    double t0 = now(); pid_t c = fork(); double t1 = now();
    if (c == 0) {
        char go; usleep(100000);
        show("child", "just after fork (before any write)");
        for (size_t i = 0; i < n / 10; i++) p[i * pg] = 2;
        show("child", "after writing to 10% of the pages");
        for (size_t i = 0; i < n; i++) p[i * pg] = 2;
        show("child", "after writing to ALL pages");
        (void)go; return 0;
    }
    printf("fork() of a process with %d MB took %.3f ms\n", MB, t1 - t0);
    waitpid(c, NULL, 0);
    show("parent", "after child exited");
    /* for comparison: copying 200 MB eagerly */
    char *q = malloc(bytes); double t2 = now(); memcpy(q, p, bytes); double t3 = now();
    printf("copying %d MB eagerly with memcpy() took %.1f ms (that is what fork() would cost without copy-on-write)\n", MB, t3 - t2);
    return q[0] & 0;
}
