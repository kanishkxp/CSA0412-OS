/* Q1 - Context switch cost
 * Name: Kanishk C   Reg No: 192511278
 *
 * Two tasks bounce one byte back and forth through two pipes. Both are pinned
 * to the same CPU, so every hand-over forces the kernel to switch from one task
 * to the other. Time / (2 * rounds) gives the cost of one switch (this also
 * includes the read/write system calls, so it is an upper bound).
 *
 * Usage: ./q1_ctxswitch <process|thread> [rounds]
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>

static int ab[2], ba[2];
static int rounds = 200000;

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void pin_to_cpu0(void)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(0, sizeof set, &set);   /* 0 = calling thread/process */
}

static void *echo(void *arg)
{
    (void)arg;
    pin_to_cpu0();
    char c;
    for (int i = 0; i < rounds; i++) {
        if (read(ab[0], &c, 1) != 1) break;
        if (write(ba[1], &c, 1) != 1) break;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: %s process|thread [rounds]\n", argv[0]); return 1; }
    if (argc > 2) rounds = atoi(argv[2]);
    if (pipe(ab) || pipe(ba)) { perror("pipe"); return 1; }
    pin_to_cpu0();

    pthread_t th;
    pid_t pid = 0;
    if (strcmp(argv[1], "process") == 0) {
        pid = fork();
        if (pid == 0) { echo(NULL); _exit(0); }
    } else {
        pthread_create(&th, NULL, echo, NULL);
    }

    char c = 'x';
    double t0 = now();
    for (int i = 0; i < rounds; i++) {
        if (write(ab[1], &c, 1) != 1) break;
        if (read(ba[0], &c, 1) != 1) break;
    }
    double t1 = now();

    if (pid > 0) waitpid(pid, NULL, 0); else pthread_join(th, NULL);
    printf("%-7s : %d round trips in %.1f ms -> %.2f us per context switch\n",
           argv[1], rounds, (t1 - t0) * 1000.0, (t1 - t0) * 1e6 / (2.0 * rounds));
    return 0;
}
