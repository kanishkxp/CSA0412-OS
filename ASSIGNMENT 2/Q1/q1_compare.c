/* Q1 - Alternative concept: one process per session vs one thread per session
 * Name: Kanishk C   Reg No: 192511278
 *
 * Creates N "student sessions" either as processes (fork) or as threads
 * (pthread_create). Each session touches a small working set of memory,
 * then exits. We measure the total creation + join time and the
 * resident memory of the whole group.
 *
 * Usage: ./q1_compare <process|thread> <N>
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>

#define WORK_BYTES (256 * 1024)   /* 256 KB of private data per session */

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void session_work(void)
{
    volatile char *buf = malloc(WORK_BYTES);
    if (!buf) return;
    for (size_t i = 0; i < WORK_BYTES; i += 4096) buf[i] = 1;   /* touch every page */
    free((void *)buf);
}

static void *thread_main(void *arg)
{
    (void)arg;
    session_work();
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 3) { fprintf(stderr, "usage: %s process|thread N\n", argv[0]); return 1; }
    int n = atoi(argv[2]);
    double t0 = now();

    if (strcmp(argv[1], "process") == 0) {
        for (int i = 0; i < n; i++) {
            pid_t pid = fork();
            if (pid < 0) { perror("fork"); return 1; }
            if (pid == 0) { session_work(); _exit(0); }
            if ((i % 50) == 49) {        /* reap in batches so we do not hit limits */
                while (waitpid(-1, NULL, WNOHANG) > 0) ;
            }
        }
        while (wait(NULL) > 0) ;
    } else {
        pthread_t *t = malloc(sizeof(pthread_t) * n);
        for (int i = 0; i < n; i++) {
            if (pthread_create(&t[i], NULL, thread_main, NULL) != 0) { perror("pthread_create"); return 1; }
            if ((i % 50) == 49) {
                for (int j = i - 49; j <= i; j++) pthread_join(t[j], NULL);
            }
        }
        for (int j = (n / 50) * 50; j < n; j++) pthread_join(t[j], NULL);
        free(t);
    }

    double t1 = now();
    printf("%s x %d : total time = %.2f ms, per session = %.1f us\n",
           argv[1], n, (t1 - t0) * 1000.0, (t1 - t0) * 1e6 / n);
    return 0;
}
