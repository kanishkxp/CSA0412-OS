#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/wait.h>

/* Compares the cost of
   (a) a normal function call,
   (b) one system call (monolithic kernel: the service runs inside the kernel),
   (c) a request/reply message between two processes through pipes
       (microkernel style: the service runs in another process).            */
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }
__attribute__((noinline)) static long add_one(long x){ __asm__ volatile(""); return x + 1; }

int main(void)
{
    const long N = 2000000, M = 100000;
    double t0 = now(); long s = 0;
    for (long i = 0; i < N; i++) s = add_one(s);
    double t_func = (now() - t0) / N;

    t0 = now();
    for (long i = 0; i < N; i++) syscall(SYS_getppid);
    double t_sys = (now() - t0) / N;

    int req[2], rep[2];
    pipe(req); pipe(rep);
    pid_t p = fork();
    if (p == 0) {                         /* the "server" process */
        char c;
        close(req[1]); close(rep[0]);         /* close the ends the server does not use */
        while (read(req[0], &c, 1) == 1) write(rep[1], &c, 1);
        _exit(0);
    }
    char c = 'x';
    t0 = now();
    for (long i = 0; i < M; i++) { write(req[1], &c, 1); read(rep[0], &c, 1); }
    double t_ipc = (now() - t0) / M;
    close(req[1]); waitpid(p, NULL, 0);

    printf("function call        : %9.1f ns\n", t_func * 1e9);
    printf("system call (getppid): %9.1f ns\n", t_sys * 1e9);
    printf("message round trip   : %9.1f ns   (%.0f x a system call)\n", t_ipc * 1e9, t_ipc / t_sys);
    return (int)(s & 0);
}
