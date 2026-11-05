#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

/* A "monitor task" wakes up every 1 ms. We measure how late each wake-up is,
   while several CPU-hungry programs compete for the processor.
   usage: ./wakeup_latency normal|rt [loops]
     normal : the default time-sharing scheduler (SCHED_OTHER)
     rt     : the real-time scheduler (SCHED_FIFO, priority 80)                    */
static int cmp(const void *a, const void *b){ double x=*(double*)a, y=*(double*)b; return (x>y)-(x<y); }

int main(int argc, char **argv)
{
    if (argc < 2) return 1;
    int loops = argc > 2 ? atoi(argv[2]) : 5000;
    pid_t hogs[4];
    for (int i = 0; i < 4; i++) {                 /* four competing CPU hogs */
        hogs[i] = fork();
        if (hogs[i] == 0) { volatile unsigned long x = 0; for (;;) x++; }
    }
    if (!strcmp(argv[1], "rt")) {
        struct sched_param sp = { .sched_priority = 80 };
        if (sched_setscheduler(0, SCHED_FIFO, &sp) != 0) { perror("sched_setscheduler"); goto out; }
    }
    double *lat = malloc(loops * sizeof *lat);
    struct timespec next, now;
    clock_gettime(CLOCK_MONOTONIC, &next);
    for (int i = 0; i < loops; i++) {
        next.tv_nsec += 1000000;                  /* next wake-up in 1 ms */
        if (next.tv_nsec >= 1000000000) { next.tv_nsec -= 1000000000; next.tv_sec++; }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
        clock_gettime(CLOCK_MONOTONIC, &now);
        lat[i] = (now.tv_sec - next.tv_sec) * 1e6 + (now.tv_nsec - next.tv_nsec) / 1e3;   /* microseconds late */
    }
    double sum = 0; for (int i = 0; i < loops; i++) sum += lat[i];
    qsort(lat, loops, sizeof *lat, cmp);
    printf("%-6s wake-up lateness over %d wake-ups: average %.1f us, 99th percentile %.1f us, worst %.1f us\n",
           argv[1], loops, sum / loops, lat[(int)(loops * 0.99)], lat[loops - 1]);
    free(lat);
out:
    for (int i = 0; i < 4; i++) kill(hogs[i], SIGKILL);
    for (int i = 0; i < 4; i++) waitpid(hogs[i], NULL, 0);
    return 0;
}
