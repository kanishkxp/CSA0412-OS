#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/wait.h>

/* Time to create and finish N processes (fork + _exit + waitpid)
   versus N threads (pthread_create + pthread_join).
   usage: ./fork_vs_thread N heap_MB                                    */
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }
static void *worker(void *a){ (void)a; return NULL; }

int main(int argc, char **argv)
{
    int n = argc > 1 ? atoi(argv[1]) : 1000;
    size_t mb = argc > 2 ? (size_t)atol(argv[2]) : 0;
    char *heap = NULL;
    if (mb) { heap = malloc(mb << 20); memset(heap, 1, mb << 20); }   /* touch every page */

    double t0 = now();
    for (int i = 0; i < n; i++) {
        pid_t p = fork();
        if (p == 0) _exit(0);
        waitpid(p, NULL, 0);
    }
    double tp = now() - t0;

    t0 = now();
    for (int i = 0; i < n; i++) {
        pthread_t t;
        pthread_create(&t, NULL, worker, NULL);
        pthread_join(t, NULL);
    }
    double tt = now() - t0;
    printf("N=%d heap=%zuMB  processes: %.4f s (%.1f us each)  threads: %.4f s (%.1f us each)  ratio=%.1f\n",
           n, mb, tp, tp / n * 1e6, tt, tt / n * 1e6, tp / tt);
    free(heap);
    return 0;
}
