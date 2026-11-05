#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>

/* Keeps N processes or N threads alive at the same time and reports the memory
   they use, read from /proc:
     VmSize = virtual address space, VmRSS = pages really in RAM,
     Pss    = proportional set size (shared pages are divided between the sharers).
   usage: ./alive_memory proc|thread N                                            */
static long field(const char *path, const char *key)
{
    FILE *f = fopen(path, "r"); if (!f) return -1;
    char line[256]; long v = -1; size_t k = strlen(key);
    while (fgets(line, sizeof line, f)) if (!strncmp(line, key, k)) { v = atol(line + k); break; }
    fclose(f); return v;
}
static void *sleeper(void *a){ (void)a; pause(); return NULL; }

int main(int argc, char **argv)
{
    if (argc != 3) return 1;
    int n = atoi(argv[2]);
    char path[64];
    if (argv[1][0] == 'p') {
        pid_t *pids = malloc(n * sizeof *pids);
        for (int i = 0; i < n; i++) { pids[i] = fork(); if (pids[i] == 0) { pause(); _exit(0); } }
        sleep(1);
        double vsz = 0, rss = 0, pss = 0;
        for (int i = 0; i < n; i++) {
            snprintf(path, sizeof path, "/proc/%d/status", pids[i]);
            vsz += field(path, "VmSize:"); rss += field(path, "VmRSS:");
            snprintf(path, sizeof path, "/proc/%d/smaps_rollup", pids[i]);
            pss += field(path, "Pss:");
        }
        printf("%d processes: per process VmSize %.0f kB, VmRSS %.0f kB, Pss %.1f kB\n", n, vsz / n, rss / n, pss / n);
        for (int i = 0; i < n; i++) kill(pids[i], SIGKILL);
        for (int i = 0; i < n; i++) waitpid(pids[i], NULL, 0);
    } else {
        long v0 = field("/proc/self/status", "VmSize:"), r0 = field("/proc/self/status", "VmRSS:");
        long p0 = field("/proc/self/smaps_rollup", "Pss:");
        pthread_t *t = malloc(n * sizeof *t);
        for (int i = 0; i < n; i++) pthread_create(&t[i], NULL, sleeper, NULL);
        sleep(1);
        long v1 = field("/proc/self/status", "VmSize:"), r1 = field("/proc/self/status", "VmRSS:");
        long p1 = field("/proc/self/smaps_rollup", "Pss:");
        printf("%d threads: per thread VmSize +%.0f kB, VmRSS +%.1f kB, Pss +%.1f kB\n", n,
               (double)(v1 - v0) / n, (double)(r1 - r0) / n, (double)(p1 - p0) / n);
        fflush(stdout);
        _exit(0);
    }
    return 0;
}
