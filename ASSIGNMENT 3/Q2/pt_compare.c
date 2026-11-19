/* pt_compare.c - alternative concept: inverted page table vs per-process 4-level page table.
 * Measures (a) memory used by the tables and (b) time for one address translation, in software.
 * 300 processes, each with a 40 MB region mapped (10240 pages); machine with 16 GB RAM (4,194,304 frames). */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define NPROC 300
#define NPAGES 10240u               /* pages per process (40 MB) */
#define FRAMES 4194304u             /* 16 GB / 4 KB */
#define L 9                          /* 9 bits per level, 4 levels, 48-bit VA */
typedef struct pt { void *e[512]; } pt_t;          /* 4 KB table (512 x 8-byte pointers or PTEs) */
static size_t table_bytes = 0;

static pt_t *new_table(void) { table_bytes += sizeof(pt_t); return calloc(1, sizeof(pt_t)); }
static void map4(pt_t *root, uint64_t vpn, uint64_t frame)
{
    pt_t *t = root;
    for (int lvl = 3; lvl > 0; lvl--) {
        unsigned idx = (vpn >> (lvl * L)) & 511;
        if (!t->e[idx]) t->e[idx] = new_table();
        t = t->e[idx];
    }
    t->e[vpn & 511] = (void *)(uintptr_t)(frame + 1);   /* +1 so that frame 0 is not NULL */
}
static inline uint64_t walk4(pt_t *root, uint64_t vpn)
{
    pt_t *t = root;
    for (int lvl = 3; lvl > 0; lvl--) { t = t->e[(vpn >> (lvl * L)) & 511]; if (!t) return (uint64_t)-1; }
    return (uint64_t)(uintptr_t)t->e[vpn & 511] - 1;
}

/* inverted page table: one entry per physical frame + hash anchor table */
typedef struct { uint32_t pid, vpn, next, flags; } ipt_entry;   /* 16 bytes */
static ipt_entry *ipt; static uint32_t *anchor;
static inline uint32_t h(uint32_t pid, uint32_t vpn) { return (uint32_t)(((uint64_t)pid * 2654435761u + vpn * 40503u) % FRAMES); }
static void ipt_insert(uint32_t pid, uint32_t vpn, uint32_t frame)
{ ipt[frame].pid = pid; ipt[frame].vpn = vpn; uint32_t b = h(pid, vpn); ipt[frame].next = anchor[b]; anchor[b] = frame + 1; }
static inline int64_t ipt_lookup(uint32_t pid, uint32_t vpn, int *steps)
{
    uint32_t f = anchor[h(pid, vpn)]; int s = 0;
    while (f) { s++; ipt_entry *e = &ipt[f - 1]; if (e->pid == pid && e->vpn == vpn) { *steps += s; return f - 1; } f = e->next; }
    *steps += s; return -1;
}
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e9 + t.tv_nsec; }

int main(void)
{
    pt_t *root[NPROC]; uint32_t *perm = malloc(sizeof(uint32_t) * FRAMES);
    for (uint32_t i = 0; i < FRAMES; i++) perm[i] = i;
    srand(7); for (uint32_t i = FRAMES - 1; i > 0; i--) { uint32_t j = ((uint32_t)rand() << 8 ^ (uint32_t)rand()) % (i + 1); uint32_t t = perm[i]; perm[i] = perm[j]; perm[j] = t; }
    ipt = calloc(FRAMES, sizeof(ipt_entry)); anchor = calloc(FRAMES, sizeof(uint32_t));
    uint32_t next = 0;
    for (int p = 0; p < NPROC; p++) {
        root[p] = new_table();
        uint64_t base = 0x7f0000000ull + (uint64_t)p * 0x100000ull;      /* pages */
        for (uint32_t i = 0; i < NPAGES; i++) { uint32_t f = perm[next++]; map4(root[p], base + i, f); ipt_insert(p, (uint32_t)(base + i), f); }
    }
    double mt = table_bytes / 1048576.0, mi = (FRAMES * sizeof(ipt_entry) + FRAMES * sizeof(uint32_t)) / 1048576.0;
    printf("frames in use: %u of %u\n", next, FRAMES);
    printf("per-process 4-level tables, %d processes : %.1f MB  (%.1f KB per process)\n", NPROC, mt, table_bytes / 1024.0 / NPROC);
    printf("inverted page table (fixed by RAM size)  : %.1f MB  (16 B entry x %u frames + hash anchors)\n", mi, FRAMES);
    printf("with 1000 such processes the 4-level tables would need about %.1f MB; the inverted table stays %.1f MB\n", table_bytes / 1048576.0 / NPROC * 1000, mi);

    /* time: 20 million random translations */
    enum { Q = 20000000 };
    uint32_t *qp = malloc(sizeof(uint32_t) * 1000000), *qi = malloc(sizeof(uint32_t) * 1000000);
    for (int i = 0; i < 1000000; i++) { qp[i] = rand() % NPROC; qi[i] = rand() % NPAGES; }
    uint64_t sink = 0; int steps = 0; double t0 = now();
    for (int r = 0; r < Q / 1000000; r++) for (int i = 0; i < 1000000; i++) { uint64_t base = 0x7f0000000ull + (uint64_t)qp[i] * 0x100000ull; sink += walk4(root[qp[i]], base + qi[i]); }
    double t1 = now();
    for (int r = 0; r < Q / 1000000; r++) for (int i = 0; i < 1000000; i++) { uint64_t base = 0x7f0000000ull + (uint64_t)qp[i] * 0x100000ull; sink += (uint64_t)ipt_lookup(qp[i], (uint32_t)(base + qi[i]), &steps); }
    double t2 = now();
    printf("\n4-level walk       : %.1f ns per translation (4 dependent memory reads)\n", (t1 - t0) / Q);
    printf("inverted hash look-up: %.1f ns per translation (average chain length %.2f)\n", (t2 - t1) / Q, (double)steps / Q);
    printf("(checksum %llu)\n", (unsigned long long)sink);
    return 0;
}
