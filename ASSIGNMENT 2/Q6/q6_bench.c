/* Q6 - Measuring the running time of the Banker's safety algorithm
 * Name: Kanishk C   Reg No: 192511278
 *
 * PREDICTION (made before measuring): the safety algorithm repeats "scan all n processes,
 * and for each unfinished one compare m numbers". If only ONE process can finish per scan
 * (worst case) there are n scans of n processes with m comparisons each, so T = O(m * n^2).
 * If all processes can finish in the first scan (best case) it is one scan: O(m * n).
 *
 * The worst case is built like a chain: process p can only finish after process p+1 has
 * released its resources. The "chain" resource is the LAST column so the comparison loop
 * cannot stop early.
 *
 * Build: gcc -std=c11 -Wall -Wextra -O2 -o q6_bench q6_bench.c
 * Run  : ./q6_bench
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct { int n, m; int *avail, *maxm, *alloc; } State;      /* flat arrays: [p*m + r] */

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static State make_state(int n, int m, int worst)
{
    State s = {n, m, calloc(m, sizeof(int)), calloc((size_t)n * m, sizeof(int)), calloc((size_t)n * m, sizeof(int))};
    for (int p = 0; p < n; p++) {
        for (int r = 0; r < m; r++) {
            s.alloc[p * m + r] = (r == m - 1) ? 1 : 0;
            if (worst) s.maxm[p * m + r] = (r == m - 1) ? 1 + (n - 1 - p) : 0;   /* need = n-1-p units of chain resource */
            else       s.maxm[p * m + r] = s.alloc[p * m + r];                   /* need = 0: everybody can finish at once */
        }
    }
    s.avail[m - 1] = 0;
    return s;
}

static void free_state(State *s) { free(s->avail); free(s->maxm); free(s->alloc); }

/* the safety algorithm, same logic as q6_banker.c but for any n and m; returns 1 if safe */
static int is_safe(const State *s, int *seq, int *work, char *finish)
{
    int n = s->n, m = s->m, count = 0;
    memcpy(work, s->avail, sizeof(int) * m);
    memset(finish, 0, n);
    while (count < n) {
        int found = 0;
        for (int p = 0; p < n; p++) {
            if (finish[p]) continue;
            int ok = 1;
            for (int r = 0; r < m; r++)
                if (s->maxm[p * m + r] - s->alloc[p * m + r] > work[r]) { ok = 0; break; }
            if (ok) {
                for (int r = 0; r < m; r++) work[r] += s->alloc[p * m + r];
                finish[p] = 1; seq[count++] = p; found = 1;
            }
        }
        if (!found) return 0;
    }
    return 1;
}

static double time_safety(int n, int m, int worst, int reps)
{
    State s = make_state(n, m, worst);
    int *seq = malloc(sizeof(int) * n), *work = malloc(sizeof(int) * m);
    char *finish = malloc(n);
    int ok = 1;
    double t0 = now();
    for (int i = 0; i < reps; i++) ok &= is_safe(&s, seq, work, finish);
    double t = (now() - t0) / reps;
    if (!ok) fprintf(stderr, "state was not safe!\n");
    free(seq); free(work); free(finish); free_state(&s);
    return t;
}

int main(void)
{
    printf("== Safety algorithm, m = 3 resource types ==\n");
    printf("%6s %16s %16s %12s\n", "n", "worst case (us)", "best case (us)", "worst/n^2 (ns)");
    int ns[] = {10, 100, 1000, 2000};
    double prev = 0;
    for (int k = 0; k < 4; k++) {
        int n = ns[k];
        int reps = n <= 100 ? 20000 : (n == 1000 ? 20 : 5);
        double w = time_safety(n, 3, 1, reps), b = time_safety(n, 3, 0, reps * 5);
        printf("%6d %16.2f %16.2f %12.2f", n, w * 1e6, b * 1e6, w * 1e9 / ((double)n * n));
        if (prev > 0) printf("   (x%.1f from previous row)", w / prev);
        printf("\n");
        prev = w;
    }

    printf("\n== Safety algorithm, n = 500, worst case, growing number of resource types m ==\n");
    printf("%6s %16s\n", "m", "time (us)");
    int ms[] = {2, 4, 8, 16, 32};
    for (int k = 0; k < 5; k++) printf("%6d %16.2f\n", ms[k], time_safety(500, ms[k], 1, 50) * 1e6);

    /* cost of ONE resource request: validate, tentatively allocate, (optionally) run the safety
     * algorithm, then roll back so that every repetition starts from the same state */
    printf("\n== Cost of ONE resource request, m = 3, worst-case layout (chain), grows with n ==\n");
    printf("%6s %22s %22s\n", "n", "with Banker's (us)", "without check (us)");
    int rn[] = {10, 100, 1000};
    for (int k = 0; k < 3; k++) {
        int n = rn[k], m = 3, reps = n <= 100 ? 20000 : 20;
        State s = make_state(n, m, 1);
        s.avail[m - 1] = 1;                                   /* one spare unit so that process 0 can ask for it */
        int *seq = malloc(sizeof(int) * n), *work = malloc(sizeof(int) * m);
        char *finish = malloc(n);
        int req[3] = {0, 0, 1};
        long granted = 0;
        for (int check = 1; check >= 0; check--) {
            double t0 = now();
            for (int i = 0; i < reps; i++) {
                int ok = 1;
                for (int r = 0; r < m; r++)
                    if (req[r] > s.maxm[r] - s.alloc[r] || req[r] > s.avail[r]) ok = 0;      /* p = 0 */
                if (!ok) continue;
                for (int r = 0; r < m; r++) { s.avail[r] -= req[r]; s.alloc[r] += req[r]; }
                if (!check || is_safe(&s, seq, work, finish)) granted++;
                for (int r = 0; r < m; r++) { s.avail[r] += req[r]; s.alloc[r] -= req[r]; }  /* roll back */
            }
            double t = (now() - t0) / reps;
            if (check) printf("%6d %22.3f", n, t * 1e6); else printf(" %22.3f\n", t * 1e6);
        }
        if (granted != 2L * reps) fprintf(stderr, "unexpected number of granted requests: %ld\n", granted);
        free(seq); free(work); free(finish); free_state(&s);
    }
    return 0;
}
