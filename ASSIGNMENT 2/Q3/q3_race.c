/* Q3 - Two threads update a shared counter: without protection, with a mutex, with an atomic
 * Name: Kanishk C   Reg No: 192511278
 *
 * Each thread adds 1 to the counter UPDATES times. The correct final value is
 * THREADS * UPDATES. "volatile" forces a real load and store on every update so
 * that the race is visible even with optimisation turned on.
 *
 * Build: gcc -std=c11 -Wall -Wextra -O2 -pthread -o q3_race q3_race.c
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define THREADS 2
#define UPDATES 5000000L

static volatile long unsafe_counter = 0;
static long locked_counter = 0;
static atomic_long atomic_counter = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *add_unsafe(void *a) { (void)a; for (long i = 0; i < UPDATES; i++) unsafe_counter++; return NULL; }
static void *add_locked(void *a)
{
    (void)a;
    for (long i = 0; i < UPDATES; i++) {
        pthread_mutex_lock(&lock);
        locked_counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}
static void *add_atomic(void *a) { (void)a; for (long i = 0; i < UPDATES; i++) atomic_fetch_add(&atomic_counter, 1); return NULL; }

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static double run(void *(*fn)(void *))
{
    pthread_t t[THREADS];
    double t0 = now();
    for (int i = 0; i < THREADS; i++) pthread_create(&t[i], NULL, fn, NULL);
    for (int i = 0; i < THREADS; i++) pthread_join(t[i], NULL);
    return now() - t0;
}

int main(void)
{
    long expected = THREADS * UPDATES;
    double a = run(add_unsafe), b = run(add_locked), c = run(add_atomic);
    printf("expected value      : %ld\n", expected);
    printf("no protection       : %ld  (lost %ld updates)  %.3f s\n", (long)unsafe_counter, expected - (long)unsafe_counter, a);
    printf("mutex               : %ld  (lost %ld updates)  %.3f s\n", locked_counter, expected - locked_counter, b);
    printf("atomic_fetch_add    : %ld  (lost %ld updates)  %.3f s\n", (long)atomic_counter, expected - (long)atomic_counter, c);
    return 0;
}
