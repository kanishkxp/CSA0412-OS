/* Q4 - Alternative concept: time to transfer n readings (one int at a time)
 * Name: Kanishk C   Reg No: 192511278
 *
 * Same job as the weather station: the parent sends n integer readings to the
 * child, which adds them up. Method "pipe" uses one write()/read() per reading
 * (two copies through the kernel); method "shm" uses a shared ring buffer and
 * two semaphores (no copy through the kernel, but a semaphore operation for
 * every reading).
 *
 * Build: gcc -std=c11 -Wall -Wextra -O2 -pthread -o q4_readings q4_readings.c
 * Run  : ./q4_readings pipe|shm N
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>

#define SLOTS 64

struct shared {
    sem_t empty, full;
    int buf[SLOTS];
    long result;
};

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char **argv)
{
    if (argc != 3) { fprintf(stderr, "usage: %s pipe|shm N\n", argv[0]); return 1; }
    long n = atol(argv[2]);
    int use_pipe = strcmp(argv[1], "pipe") == 0;
    int fd[2];
    struct shared *sh = mmap(NULL, sizeof *sh, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(&sh->empty, 1, SLOTS);
    sem_init(&sh->full, 1, 0);
    if (use_pipe && pipe(fd) == -1) { perror("pipe"); return 1; }

    double t0 = now();
    pid_t pid = fork();
    if (pid == 0) {
        long sum = 0;
        int v, out = 0;
        if (use_pipe) {
            close(fd[1]);
            while (read(fd[0], &v, sizeof v) == (ssize_t)sizeof v) sum += v;
        } else {
            for (long i = 0; i < n; i++) {
                sem_wait(&sh->full);
                sum += sh->buf[out]; out = (out + 1) % SLOTS;
                sem_post(&sh->empty);
            }
        }
        sh->result = sum;
        _exit(0);
    }
    long expected = 0;
    int in = 0;
    if (use_pipe) close(fd[0]);
    for (long i = 0; i < n; i++) {
        int v = (int)(i % 100);
        expected += v;
        if (use_pipe) { if (write(fd[1], &v, sizeof v) < 0) { perror("write"); break; } }
        else { sem_wait(&sh->empty); sh->buf[in] = v; in = (in + 1) % SLOTS; sem_post(&sh->full); }
    }
    if (use_pipe) close(fd[1]);
    waitpid(pid, NULL, 0);
    double t1 = now();
    printf("%-4s n=%-8ld time=%8.2f ms  %7.1f ns per reading   sum %s\n", argv[1], n, (t1 - t0) * 1000.0,
           (t1 - t0) * 1e9 / n, sh->result == expected ? "OK" : "WRONG");
    return 0;
}
