/* Q3 - Alternative concept: the same camera work done by N processes instead of N threads
 * Name: Kanishk C   Reg No: 192511278
 *
 * Processes do not share variables, so every child has to send its result back
 * to the parent through a pipe (IPC). Camera c is handled by process (c % N).
 *
 * Build: gcc -std=c11 -Wall -Wextra -O2 -o q3_procs q3_procs.c
 * Run  : ./q3_procs 4
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define CAMERAS 64
#define FRAMES 400
#define ITER_PER_FRAME 50000

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static uint64_t process_camera(int cam)          /* identical to the thread version */
{
    uint64_t x = 88172645463325252ULL + (uint64_t)cam, plates = 0;
    for (int f = 0; f < FRAMES; f++) {
        for (int i = 0; i < ITER_PER_FRAME; i++) { x ^= x << 13; x ^= x >> 7; x ^= x << 17; }
        if ((x & 7) == 0) plates++;
    }
    return plates;
}

int main(int argc, char **argv)
{
    int n = argc > 1 ? atoi(argv[1]) : 4;
    int fd[2];
    if (pipe(fd) == -1) { perror("pipe"); return 1; }
    double t0 = now();

    for (int p = 0; p < n; p++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return 1; }
        if (pid == 0) {
            close(fd[0]);
            uint64_t sum = 0;
            for (int c = p; c < CAMERAS; c += n) sum += process_camera(c);
            if (write(fd[1], &sum, sizeof sum) != sizeof sum) perror("write");   /* result goes back by IPC */
            _exit(0);
        }
    }
    close(fd[1]);
    uint64_t total = 0, part;
    while (read(fd[0], &part, sizeof part) == sizeof part) total += part;
    while (wait(NULL) > 0) ;

    printf("%-10s procs=%-3d time=%.3f s   plates found=%llu\n", "processes", n, now() - t0,
           (unsigned long long)total);
    return 0;
}
