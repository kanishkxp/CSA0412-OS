#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <spawn.h>
#include <time.h>
#include <sys/wait.h>

/* Time to start N copies of /bin/true three ways:
   fork+exec, vfork+exec, posix_spawn.   usage: ./spawn_bench N heap_MB          */
extern char **environ;
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }

int main(int argc, char **argv)
{
    int n = argc > 1 ? atoi(argv[1]) : 1000;
    size_t mb = argc > 2 ? (size_t)atol(argv[2]) : 0;
    char *heap = NULL;
    if (mb) { heap = malloc(mb << 20); memset(heap, 1, mb << 20); }
    char *args[] = { "true", NULL };

    double t0 = now();
    for (int i = 0; i < n; i++) {
        pid_t p = fork();
        if (p == 0) { execv("/bin/true", args); _exit(127); }
        waitpid(p, NULL, 0);
    }
    double a = now() - t0;

    t0 = now();
    for (int i = 0; i < n; i++) {
        pid_t p = vfork();
        if (p == 0) { execv("/bin/true", args); _exit(127); }
        waitpid(p, NULL, 0);
    }
    double b = now() - t0;

    t0 = now();
    for (int i = 0; i < n; i++) {
        pid_t p;
        posix_spawn(&p, "/bin/true", NULL, NULL, args, environ);
        waitpid(p, NULL, 0);
    }
    double c = now() - t0;
    printf("N=%d heap=%zuMB  fork+exec %.3f s (%.0f us)  vfork+exec %.3f s (%.0f us)  posix_spawn %.3f s (%.0f us)\n",
           n, mb, a, a / n * 1e6, b, b / n * 1e6, c, c / n * 1e6);
    free(heap);
    return 0;
}
