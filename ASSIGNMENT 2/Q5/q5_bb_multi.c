/* Q5 - Bounded buffer with P producers and C consumers: is every item consumed exactly once?
 * Name: Kanishk C   Reg No: 192511278
 *
 * Every item has a unique number. Each consumer counts how many times it saw
 * each number (seen[]). At the end every number must have been seen exactly once.
 *
 * Build (with mutex)   : gcc -std=c11 -Wall -Wextra -O2 -pthread -o q5_bb_multi q5_bb_multi.c
 * Build (without mutex): gcc -std=c11 -Wall -Wextra -O2 -pthread -DNO_MUTEX -o q5_bb_multi_nomutex q5_bb_multi.c
 * Run                  : ./q5_bb_multi <producers> <consumers>
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>

#define SIZE 5
#define PER_PRODUCER 5000          /* items made by each producer */
#define MAXITEMS 40000

#ifdef NO_MUTEX
#define LOCK()
#define UNLOCK()
#else
#define LOCK()   pthread_mutex_lock(&lock)
#define UNLOCK() pthread_mutex_unlock(&lock)
#endif

static int buffer[SIZE], in = 0, out = 0;
static sem_t empty, full;
static pthread_mutex_t lock __attribute__((unused)) = PTHREAD_MUTEX_INITIALIZER;
static atomic_int seen[MAXITEMS + 1];
static int P, C, per_consumer;

static void *producer(void *arg)
{
    int id = (int)(long)arg;
    for (int k = 0; k < PER_PRODUCER; k++) {
        int item = id * PER_PRODUCER + k + 1;            /* unique number */
        sem_wait(&empty);
        LOCK();
        buffer[in] = item;
        in = (in + 1) % SIZE;
        UNLOCK();
        sem_post(&full);
    }
    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;
    for (int k = 0; k < per_consumer; k++) {
        sem_wait(&full);
        LOCK();
        int item = buffer[out];
        out = (out + 1) % SIZE;
        UNLOCK();
        sem_post(&empty);
        if (item >= 1 && item <= P * PER_PRODUCER) atomic_fetch_add(&seen[item], 1);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    P = argc > 1 ? atoi(argv[1]) : 1;
    C = argc > 2 ? atoi(argv[2]) : 1;
    int total = P * PER_PRODUCER;
    if (total % C != 0 || total > MAXITEMS) { fprintf(stderr, "bad P/C\n"); return 1; }
    per_consumer = total / C;

    sem_init(&empty, 0, SIZE);
    sem_init(&full, 0, 0);
    pthread_t tp[16], tc[16];
    for (long i = 0; i < P; i++) pthread_create(&tp[i], NULL, producer, (void *)i);
    for (long i = 0; i < C; i++) pthread_create(&tc[i], NULL, consumer, NULL);
    for (int i = 0; i < P; i++) pthread_join(tp[i], NULL);
    for (int i = 0; i < C; i++) pthread_join(tc[i], NULL);

    int lost = 0, duplicated = 0;
    for (int i = 1; i <= total; i++) {
        int s = atomic_load(&seen[i]);
        if (s == 0) lost++;
        if (s > 1) duplicated += s - 1;
    }
#ifdef NO_MUTEX
    const char *mode = "no mutex";
#else
    const char *mode = "with mutex";
#endif
    printf("%-10s producers=%d consumers=%d items=%-6d lost=%-5d duplicated=%-5d -> %s\n", mode, P, C, total,
           lost, duplicated, (lost == 0 && duplicated == 0) ? "every item consumed exactly once" : "WRONG");
    return (lost || duplicated) ? 2 : 0;
}
