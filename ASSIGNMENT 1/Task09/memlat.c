#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Pointer chasing: measures the time of one dependent memory access for
   working sets from 4 KB to 256 MB. The steps in the result show the cache levels. */
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }
int main(void)
{
    srand(1);
    printf("size_kb,ns_per_access\n");
    for (size_t kb = 4; kb <= 262144; kb *= 2) {
        size_t n = kb * 1024 / sizeof(void *);
        void **a = malloc(n * sizeof(void *));
        size_t *idx = malloc(n * sizeof(size_t));
        for (size_t i = 0; i < n; i++) idx[i] = i;
        for (size_t i = n - 1; i > 0; i--) { size_t j = ((size_t)rand() * 65537u + rand()) % (i + 1); size_t t = idx[i]; idx[i] = idx[j]; idx[j] = t; }
        for (size_t i = 0; i < n; i++) a[idx[i]] = &a[idx[(i + 1) % n]];     /* one big random cycle */
        void **p = &a[idx[0]];
        size_t steps = 20000000;
        for (size_t i = 0; i < 1000000; i++) p = (void **)*p;                  /* warm up */
        double t0 = now();
        for (size_t i = 0; i < steps; i++) p = (void **)*p;
        double t = now() - t0;
        printf("%zu,%.2f\n", kb, t / steps * 1e9);
        fprintf(stderr, "%zu KB: %.2f ns (%p)\n", kb, t / steps * 1e9, (void *)p);
        free(a); free(idx);
    }
    return 0;
}
