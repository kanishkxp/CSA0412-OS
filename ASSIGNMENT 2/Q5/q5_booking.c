/* Q5 - Railway ticket booking: one lock for everything, one per train or one per coach
 * Name: Kanishk C   Reg No: 192511278
 *
 * seat[train][coach][seat] is 0 (free) or 1 (booked). Every operation picks a seat and
 * either books it (if free) or cancels it (if booked). The check and the update are
 * separated by a little work (W spin iterations, standing for payment / validation), which
 * is exactly where an unprotected system sells the same seat twice.
 *
 * Correctness check at the end:  booked - cancelled  must equal the number of seats that are
 * really occupied. Any difference = seats sold twice / lost updates.
 *
 * Build: gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_booking q5_booking.c
 * Run  : ./q5_booking <none|global|train|coach> <threads> <ops per thread> <trains> <coaches> <seats> <W> [hot%]
 *        hot% = share of requests that go to train 0 (a popular train); default 0 = uniform
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define MAXTRAINS 64
#define MAXCOACH 16
#define MAXSEATS 128

static volatile char seat[MAXTRAINS][MAXCOACH][MAXSEATS];
static pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t train_lock[MAXTRAINS];
static pthread_mutex_t coach_lock[MAXTRAINS][MAXCOACH];

static int mode;                       /* 0 none, 1 global, 2 train, 3 coach */
static int threads, trains, coaches, seats, W, hot;
static long ops;

typedef struct { long booked, cancelled; uint64_t rng; } Stat;

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static inline uint64_t next(uint64_t *x)
{
    *x ^= *x << 13; *x ^= *x >> 7; *x ^= *x << 17;
    return *x;
}

static void spin(int n) { for (volatile int i = 0; i < n; i++) ; }

static void *agent(void *arg)
{
    Stat *st = arg;
    for (long k = 0; k < ops; k++) {
        int t = (hot && (int)(next(&st->rng) % 100) < hot) ? 0 : (int)(next(&st->rng) % trains);
        int c = (int)(next(&st->rng) % coaches);
        int s = (int)(next(&st->rng) % seats);

        pthread_mutex_t *m = NULL;
        if (mode == 1) m = &global_lock;
        else if (mode == 2) m = &train_lock[t];
        else if (mode == 3) m = &coach_lock[t][c];
        if (m) pthread_mutex_lock(m);

        if (seat[t][c][s] == 0) {              /* check: is the seat free?          */
            spin(W);                           /* payment / validation window       */
            seat[t][c][s] = 1;                 /* act: book it                      */
            st->booked++;
        } else {
            spin(W);
            seat[t][c][s] = 0;                 /* cancel                            */
            st->cancelled++;
        }

        if (m) pthread_mutex_unlock(m);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 8) { fprintf(stderr, "usage: %s none|global|train|coach threads ops trains coaches seats W [hot%%]\n", argv[0]); return 1; }
    mode = !strcmp(argv[1], "none") ? 0 : !strcmp(argv[1], "global") ? 1 : !strcmp(argv[1], "train") ? 2 : 3;
    threads = atoi(argv[2]); ops = atol(argv[3]); trains = atoi(argv[4]); coaches = atoi(argv[5]);
    seats = atoi(argv[6]); W = atoi(argv[7]); hot = argc > 8 ? atoi(argv[8]) : 0;
    for (int t = 0; t < MAXTRAINS; t++) {
        pthread_mutex_init(&train_lock[t], NULL);
        for (int c = 0; c < MAXCOACH; c++) pthread_mutex_init(&coach_lock[t][c], NULL);
    }

    pthread_t th[64];
    Stat st[64];
    double t0 = now();
    for (int i = 0; i < threads; i++) {
        memset(&st[i], 0, sizeof st[i]);
        st[i].rng = 88172645463325252ULL + 7919ULL * (uint64_t)(i + 1);
        pthread_create(&th[i], NULL, agent, &st[i]);
    }
    for (int i = 0; i < threads; i++) pthread_join(th[i], NULL);
    double dt = now() - t0;

    long booked = 0, cancelled = 0, occupied = 0;
    for (int i = 0; i < threads; i++) { booked += st[i].booked; cancelled += st[i].cancelled; }
    for (int t = 0; t < trains; t++)
        for (int c = 0; c < coaches; c++)
            for (int s = 0; s < seats; s++) occupied += seat[t][c][s];
    long expected = booked - cancelled;

    printf("%-6s threads=%-2d hot=%-2d%% ops=%-8ld time=%7.3f s  throughput=%9.0f ops/s   booked-cancelled=%ld occupied=%ld  -> %s\n",
           argv[1], threads, hot, ops * threads, dt, (double)ops * threads / dt, expected, occupied,
           expected == occupied ? "consistent" : "INCONSISTENT (seats sold twice / lost)");
    return 0;
}
