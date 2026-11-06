/* Q3 - Threads: traffic-camera number-plate reader
 * Name: Kanishk C   Reg No: 192511278
 *
 * 64 cameras, each one has FRAMES frames to analyse. Analysing one frame is
 * simulated with a CPU-bound hash loop (a stand-in for the real plate
 * recognition). The program can run:
 *     serial            - one camera after the other (the slow first version)
 *     per-camera        - one thread for every camera (64 threads)
 *     pool N            - a fixed pool of N threads that take cameras from a shared counter
 *
 * Build: gcc -std=c11 -Wall -Wextra -O2 -pthread -o q3_threads q3_threads.c
 * Run  : ./q3_threads serial | ./q3_threads per-camera | ./q3_threads pool 4
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define CAMERAS 64
#define FRAMES 400
#define ITER_PER_FRAME 50000

static uint64_t result[CAMERAS];                 /* shared array, every camera writes only its own slot */
static int next_camera = 0;                      /* shared work counter for the pool */
static pthread_mutex_t next_lock = PTHREAD_MUTEX_INITIALIZER;

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* pretend to read the plates of all frames of one camera */
static uint64_t process_camera(int cam)
{
    uint64_t x = 88172645463325252ULL + (uint64_t)cam, plates = 0;
    for (int f = 0; f < FRAMES; f++) {
        for (int i = 0; i < ITER_PER_FRAME; i++) {
            x ^= x << 13; x ^= x >> 7; x ^= x << 17;     /* xorshift: cheap, cannot be optimised away */
        }
        if ((x & 7) == 0) plates++;                      /* "a plate was found in this frame" */
    }
    return plates;
}

static void *camera_thread(void *arg)
{
    int cam = (int)(intptr_t)arg;
    result[cam] = process_camera(cam);
    return NULL;
}

static void *pool_worker(void *arg)
{
    (void)arg;
    for (;;) {
        pthread_mutex_lock(&next_lock);
        int cam = next_camera < CAMERAS ? next_camera++ : -1;
        pthread_mutex_unlock(&next_lock);
        if (cam < 0) break;
        result[cam] = process_camera(cam);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: %s serial|per-camera|pool N\n", argv[0]); return 1; }
    double t0 = now();
    int threads = 1;

    if (strcmp(argv[1], "serial") == 0) {
        for (int c = 0; c < CAMERAS; c++) result[c] = process_camera(c);
    } else if (strcmp(argv[1], "per-camera") == 0) {
        pthread_t t[CAMERAS];
        threads = CAMERAS;
        for (int c = 0; c < CAMERAS; c++) pthread_create(&t[c], NULL, camera_thread, (void *)(intptr_t)c);
        for (int c = 0; c < CAMERAS; c++) pthread_join(t[c], NULL);
    } else if (strcmp(argv[1], "pool") == 0 && argc == 3) {
        threads = atoi(argv[2]);
        pthread_t *t = malloc(sizeof(pthread_t) * threads);
        for (int i = 0; i < threads; i++) pthread_create(&t[i], NULL, pool_worker, NULL);
        for (int i = 0; i < threads; i++) pthread_join(t[i], NULL);
        free(t);
    } else { fprintf(stderr, "bad arguments\n"); return 1; }

    double t1 = now();
    uint64_t total = 0;
    for (int c = 0; c < CAMERAS; c++) total += result[c];
    printf("%-10s threads=%-3d time=%.3f s   plates found=%llu\n", argv[1], threads, t1 - t0,
           (unsigned long long)total);
    return 0;
}
