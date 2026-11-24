#define _POSIX_C_SOURCE 199309L
/* Measures how long it takes to reach block k of a file with
   (a) a FAT held in memory (follow the chain, O(k)) and
   (b) an index (direct lookup, O(1)).
   The file is 4 GB with 4 KB blocks = 1,048,576 blocks.                       */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NBLOCKS 1048576u

static double now_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

int main(void)
{
    unsigned *fat = malloc(NBLOCKS * sizeof *fat);       /* 4 bytes per entry */
    unsigned *index = malloc(NBLOCKS * sizeof *index);
    unsigned *perm = malloc(NBLOCKS * sizeof *perm);
    if (!fat || !index || !perm) return 1;

    /* scatter the blocks so that the chain is not simply 0,1,2,... */
    for (unsigned i = 0; i < NBLOCKS; i++) perm[i] = i;
    srand(1);
    for (unsigned i = NBLOCKS - 1; i > 0; i--) {
        unsigned j = (unsigned)rand() % (i + 1);
        unsigned t = perm[i]; perm[i] = perm[j]; perm[j] = t;
    }
    for (unsigned i = 0; i + 1 < NBLOCKS; i++) fat[perm[i]] = perm[i + 1];
    fat[perm[NBLOCKS - 1]] = 0xFFFFFFFFu;                 /* end of chain */
    for (unsigned i = 0; i < NBLOCKS; i++) index[i] = perm[i];

    unsigned ks[] = {10, 1000, 100000, 1048575};
    printf("%10s %16s %16s %12s\n", "block k", "FAT walk (ms)", "index (ms)", "same block");
    for (int t = 0; t < 4; t++) {
        unsigned k = ks[t];
        double t0 = now_ms();
        unsigned cur = perm[0];
        for (unsigned i = 0; i < k; i++) cur = fat[cur];
        double fat_ms = now_ms() - t0;

        t0 = now_ms();
        volatile unsigned r = 0;
        for (int rep = 0; rep < 1000000; rep++) r = index[k];   /* repeat to get a readable time */
        double idx_ms = (now_ms() - t0) / 1000000.0;
        printf("%10u %16.4f %16.9f %12s\n", k, fat_ms, idx_ms, (cur == r) ? "yes" : "NO");
    }
    printf("FAT memory for this file: %.1f MB\n", NBLOCKS * 4.0 / (1024 * 1024));
    free(fat); free(index); free(perm);
    return 0;
}
