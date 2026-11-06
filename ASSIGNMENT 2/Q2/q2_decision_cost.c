/* Q2 - Cost of ONE scheduling decision: SJF versus Round Robin / MLFQ
 * Name: Kanishk C   Reg No: 192511278
 *
 * n jobs are ready.
 *   SJF  : the scheduler scans the whole ready list to find the job with the shortest burst  -> O(n)
 *   SJF with a binary min-heap : takes the top and fixes the heap                              -> O(log n)
 *   RR / MLFQ : takes the job at the head of the (highest non-empty) queue, puts it at the tail -> O(1)
 * Every method does DECISIONS decisions on a ready list of n jobs and the average time per
 * decision is printed. A running total of the chosen values stops the compiler from removing the work.
 *
 * Build: gcc -std=c11 -Wall -Wextra -O2 -o q2_decision_cost q2_decision_cost.c
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DECISIONS 200000

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static unsigned long long rng = 88172645463325252ULL;
static unsigned next_rand(void) { rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17; return (unsigned)(rng >> 16); }

/* ---- SJF with a linear scan ---- */
static long sjf_scan(int n, int *burst)
{
    long sum = 0;
    for (int d = 0; d < DECISIONS; d++) {
        int best = 0;
        for (int i = 1; i < n; i++) if (burst[i] < burst[best]) best = i;
        sum += burst[best];
        burst[best] = (int)(next_rand() % 1000) + 1;          /* the job comes back with a new burst */
    }
    return sum;
}

/* ---- SJF with a heap ---- */
static void sift_down(int *h, int n, int i)
{
    for (;;) {
        int l = 2 * i + 1, r = l + 1, m = i;
        if (l < n && h[l] < h[m]) m = l;
        if (r < n && h[r] < h[m]) m = r;
        if (m == i) return;
        int t = h[i]; h[i] = h[m]; h[m] = t; i = m;
    }
}

static long sjf_heap(int n, int *h)
{
    for (int i = n / 2 - 1; i >= 0; i--) sift_down(h, n, i);
    long sum = 0;
    for (int d = 0; d < DECISIONS; d++) {
        sum += h[0];
        h[0] = (int)(next_rand() % 1000) + 1;                 /* replace the root and fix the heap */
        sift_down(h, n, 0);
    }
    return sum;
}

/* ---- Round Robin: circular queue, take head, put it at the tail ---- */
static long rr_queue(int n, int *q)
{
    int head = 0;
    long sum = 0;
    for (int d = 0; d < DECISIONS; d++) {
        sum += q[head];
        q[head] = (int)(next_rand() % 1000) + 1;
        head = (head + 1) % n;                                /* the job that ran is now at the tail */
    }
    return sum;
}

int main(void)
{
    int sizes[] = {10, 100, 1000, 10000};
    printf("%8s %18s %20s %18s\n", "n ready", "SJF scan (ns)", "SJF heap (ns)", "RR / MLFQ (ns)");
    long total = 0;
    for (int k = 0; k < 4; k++) {
        int n = sizes[k];
        int *a = malloc(sizeof(int) * n), *b = malloc(sizeof(int) * n), *c = malloc(sizeof(int) * n);
        for (int i = 0; i < n; i++) a[i] = b[i] = c[i] = (int)(next_rand() % 1000) + 1;
        double t0 = now(); total += sjf_scan(n, a);  double t1 = now();
        total += sjf_heap(n, b);                     double t2 = now();
        total += rr_queue(n, c);                     double t3 = now();
        printf("%8d %18.1f %20.1f %18.1f\n", n, (t1 - t0) * 1e9 / DECISIONS, (t2 - t1) * 1e9 / DECISIONS,
               (t3 - t2) * 1e9 / DECISIONS);
        free(a); free(b); free(c);
    }
    if (total == 42) printf("(never printed)\n");
    return 0;
}
