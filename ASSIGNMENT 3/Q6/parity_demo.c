/* parity_demo.c - RAID 5 / RAID 6 parity with real data.
 * 6 disks: 4 data blocks D0..D3 + P (XOR parity) + Q (Reed-Solomon over GF(2^8), RAID 6).
 *  1. build a stripe, destroy one data block, rebuild it from P (RAID 5 recovery)
 *  2. destroy TWO data blocks, rebuild both from P and Q (RAID 6 recovery)
 *  3. measure the CPU time to compute parity for a 4 KB block: RAID 5 (P only) against RAID 6 (P and Q).
 * Q = sum g^i * Di with generator g = 2 in GF(2^8), polynomial 0x11d (the one used by Linux md). */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#define BS 4096
#define ND 4
static uint8_t gf_exp[512], gf_log[256];
static void gf_init(void) { int x = 1; for (int i = 0; i < 255; i++) { gf_exp[i] = (uint8_t)x; gf_log[x] = (uint8_t)i; x <<= 1; if (x & 0x100) x ^= 0x11d; } for (int i = 255; i < 512; i++) gf_exp[i] = gf_exp[i - 255]; }
static uint8_t gmul(uint8_t a, uint8_t b) { return (a && b) ? gf_exp[gf_log[a] + gf_log[b]] : 0; }
static uint8_t ginv(uint8_t a) { return gf_exp[255 - gf_log[a]]; }
static uint8_t gpow2(int i) { return gf_exp[i % 255]; }
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e9 + t.tv_nsec; }

static void calc_p(uint8_t d[ND][BS], uint8_t *p) { memset(p, 0, BS); for (int i = 0; i < ND; i++) for (int k = 0; k < BS; k++) p[k] ^= d[i][k]; }
static void calc_pq(uint8_t d[ND][BS], uint8_t *p, uint8_t *q)
{ memset(p, 0, BS); memset(q, 0, BS);
  for (int i = 0; i < ND; i++) { uint8_t g = gpow2(i); for (int k = 0; k < BS; k++) { p[k] ^= d[i][k]; q[k] ^= gmul(g, d[i][k]); } } }

int main(void)
{
    gf_init(); static uint8_t d[ND][BS], orig[ND][BS], p[BS], q[BS], tmp[BS];
    srand(1); for (int i = 0; i < ND; i++) for (int k = 0; k < BS; k++) orig[i][k] = (uint8_t)rand();
    memcpy(d, orig, sizeof d); calc_pq(d, p, q);

    /* 1. RAID 5: lose D2, rebuild = P xor D0 xor D1 xor D3 */
    memset(d[2], 0, BS);                                     /* disk 2 failed */
    memcpy(tmp, p, BS); for (int i = 0; i < ND; i++) if (i != 2) for (int k = 0; k < BS; k++) tmp[k] ^= d[i][k];
    printf("RAID 5: disk holding D2 failed -> rebuilt from parity: %s\n", memcmp(tmp, orig[2], BS) == 0 ? "OK (identical to the original block)" : "FAILED");

    /* 2. RAID 6: lose D1 and D3. Solve  D1 + D3 = Pxy,  g^1 D1 + g^3 D3 = Qxy  */
    memcpy(d, orig, sizeof d);                                /* bring back D2 (step 1 had destroyed it) */
    int x = 1, y = 3; memset(d[x], 0, BS); memset(d[y], 0, BS);
    uint8_t g1 = gpow2(x), g3 = gpow2(y), denom = (uint8_t)(g1 ^ g3), invd = ginv(denom);
    int ok = 1;
    for (int k = 0; k < BS; k++) {
        uint8_t pxy = p[k], qxy = q[k];
        for (int i = 0; i < ND; i++) if (i != x && i != y) { pxy ^= d[i][k]; qxy ^= gmul(gpow2(i), d[i][k]); }
        uint8_t dx = gmul(invd, (uint8_t)(qxy ^ gmul(g3, pxy)));      /* D1 = (Q' + g3 P') / (g1 + g3) */
        uint8_t dy = (uint8_t)(pxy ^ dx);
        if (dx != orig[x][k] || dy != orig[y][k]) ok = 0;
    }
    printf("RAID 6: disks holding D1 and D3 failed together -> rebuilt from P and Q: %s\n", ok ? "OK (both blocks identical to the originals)" : "FAILED");

    /* 3. timing */
    memcpy(d, orig, sizeof d); int reps = 20000; double t0 = now();
    for (int r = 0; r < reps; r++) { calc_p(d, p); d[0][r % BS] ^= p[0]; }
    double t1 = now();
    for (int r = 0; r < reps; r++) { calc_pq(d, p, q); d[0][r % BS] ^= q[0]; }
    double t2 = now();
    printf("\nCPU time to compute the parity of one stripe (4 x 4 KB data blocks):\n  RAID 5 (P = XOR)         : %8.0f ns\n  RAID 6 (P and Q)         : %8.0f ns   (%.1f x more CPU work)\n", (t1 - t0) / reps, (t2 - t1) / reps, (t2 - t1) / (t1 - t0));
    puts("\nI/O needed to update ONE data block (small write):");
    puts("  RAID 5: read old data, read old P, write new data, write new P                   = 4 disk I/Os");
    puts("  RAID 6: read old data, read old P, read old Q, write new data, write new P and Q = 6 disk I/Os");
    puts("  RAID 10: write the block to both disks of the mirror                              = 2 disk writes");
    printf("  A disk I/O takes about 100 us (SSD) to 8,000 us (hard disk) so the parity CPU time of %.0f ns is small next to the extra I/O.\n", (t2 - t1) / reps);
    return 0;
}
