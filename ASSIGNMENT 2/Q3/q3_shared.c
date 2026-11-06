/* Q3 - What do threads share and what does each thread own?
 * Name: Kanishk C   Reg No: 192511278
 *
 * Three threads print the address of: a global variable, a heap block (shared
 * by pointer), a local variable on their own stack, a thread-local variable,
 * and their process id / thread id.
 *
 * Build: gcc -std=c11 -Wall -Wextra -pthread -o q3_shared q3_shared.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

int global_counter = 100;                 /* data segment: shared */
static __thread int tls_value = 0;        /* thread-local storage: private copy per thread */
int *heap_block;                          /* points to one heap block: shared */
static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_barrier_t all_alive;       /* keeps the three threads alive at the same time */

static void *worker(void *arg)
{
    int id = (int)(long)arg;
    int local = id * 10;                  /* lives on this thread's own stack */
    tls_value = id * 1000;                /* changes only this thread's copy */
    pthread_barrier_wait(&all_alive);     /* wait until all three threads exist together */

    pthread_mutex_lock(&print_lock);
    printf("thread %d: pid=%d tid=%ld\n", id, (int)getpid(), (long)syscall(SYS_gettid));
    printf("          &global_counter = %p   (same for all -> shared)\n", (void *)&global_counter);
    printf("          heap_block      = %p   (same for all -> shared)\n", (void *)heap_block);
    printf("          &local          = %p   (different -> own stack)\n", (void *)&local);
    printf("          &tls_value      = %p   (different -> own copy), value = %d\n", (void *)&tls_value, tls_value);
    pthread_mutex_unlock(&print_lock);
    return NULL;
}

int main(void)
{
    heap_block = malloc(64);
    pthread_t t[3];
    printf("main  : pid=%d\n\n", (int)getpid());
    pthread_barrier_init(&all_alive, NULL, 3);
    for (long i = 1; i <= 3; i++) pthread_create(&t[i - 1], NULL, worker, (void *)i);
    for (int i = 0; i < 3; i++) pthread_join(t[i], NULL);
    pthread_barrier_destroy(&all_alive);
    printf("\nmain thread still sees tls_value = %d (its own copy was never changed)\n", tls_value);
    free(heap_block);
    return 0;
}
