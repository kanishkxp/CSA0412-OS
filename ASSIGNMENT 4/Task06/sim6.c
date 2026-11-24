/* Task 6, Stage 5: simulation of the video-streaming storage design.
   5,000 videos, Zipf popularity (s = 1), 200,000 students, 10% watching at the same time.
   Chunks of each video are striped over all disks.
   Each request reads one 1 MB chunk at a random minute of a video (a student jumping to a minute).
   Time to first chunk = completion time - arrival time.
   HDD model: seek = 1.0 + 0.0009 ms per cylinder, rotation 0-8.33 ms (7200 rpm), transfer 180 MB/s.
   SSD model: 0.10 ms + 1 MB at 3 GB/s, FIFO over 8 drives.
   Compile: gcc -std=c11 -Wall -Wextra -O2 sim6.c -o sim6 -lm                                       */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define VIDEOS 5000
#define CYLS 10000
#define SIM_SECONDS 60.0
#define TOTAL_RATE 5555.0          /* requests per second for the whole system at the peak */
#define SSDS 8

typedef struct { double t; int rank, disk, cyl; } Req;

static double cumw[VIDEOS];        /* cumulative Zipf weights */
static double rnd(void) { return (rand() + 1.0) / (RAND_MAX + 2.0); }
static int cmp_time(const void *a, const void *b)
{
    double x = ((const Req *)a)->t, y = ((const Req *)b)->t;
    return (x > y) - (x < y);
}
static int cmp_dbl(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

static double hdd_service(int dist)
{
    double seek = dist ? 1.0 + 0.0009 * dist : 0.0;
    double rot = rnd() * 8.33;
    double transfer = 1.0 / 180.0 * 1000.0;               /* ms for 1 MB */
    return seek + rot + transfer;
}

/* run one design: H hard disks, first `cached` ranks live on the SSDs, scheduler 0 = FCFS, 1 = C-LOOK */
static void run(const char *name, int H, int cached, int sched)
{
    srand(2026);
    int cap = (int)(TOTAL_RATE * SIM_SECONDS * 1.05);
    Req *all = malloc(cap * sizeof *all);
    int n = 0;
    double t = 0;
    while (1) {
        t += -1000.0 / TOTAL_RATE * log(rnd());
        if (t > SIM_SECONDS * 1000.0 || n >= cap) break;
        double u = rnd() * cumw[VIDEOS - 1];
        int lo = 0, hi = VIDEOS - 1;
        while (lo < hi) { int mid = (lo + hi) / 2; if (cumw[mid] < u) lo = mid + 1; else hi = mid; }
        int rank = lo + 1;
        /* chunks of every video are striped over all disks, so the disk and the cylinder
           of a chunk look random; this avoids one disk becoming the hot spot of a popular video */
        int cyl = rand() % CYLS;
        all[n].t = t; all[n].rank = rank; all[n].disk = rand() % H; all[n].cyl = cyl; n++;
    }
    qsort(all, n, sizeof all[0], cmp_time);

    double *lat = malloc(n * sizeof *lat);
    int nl = 0, ssd_hits = 0, hdd_reqs = 0;
    double hdd_busy_total = 0, hdd_max_lat = 0, ssd_free[SSDS] = {0};

    /* SSD hits: FIFO on the least busy of 8 drives */
    for (int i = 0; i < n; i++) {
        if (all[i].rank > cached) continue;
        int best = 0;
        for (int s = 1; s < SSDS; s++) if (ssd_free[s] < ssd_free[best]) best = s;
        double start = all[i].t > ssd_free[best] ? all[i].t : ssd_free[best];
        double svc = 0.10 + 1.0 / 3000.0 * 1000.0;
        ssd_free[best] = start + svc;
        lat[nl++] = ssd_free[best] - all[i].t;
        ssd_hits++;
    }

    /* HDD misses: simulate every disk separately */
    for (int d = 0; d < H; d++) {
        int m = 0;
        static int list[400000];
        for (int i = 0; i < n; i++) if (all[i].rank > cached && all[i].disk == d) list[m++] = i;
        if (m == 0) continue;
        char *done = calloc(m, 1);
        double now = 0, busy = 0;
        int head = CYLS / 2, served = 0, lo = 0, hi = 0;
        while (served < m) {
            while (lo < m && done[lo]) lo++;
            while (hi < m && all[list[hi]].t <= now) hi++;
            if (hi == lo) { now = all[list[lo]].t; continue; }
            int pick = -1;
            if (sched == 0) pick = lo;
            else {                                              /* C-LOOK: next cylinder upwards, else the lowest */
                int low = -1;
                for (int i = lo; i < hi; i++) {
                    if (done[i]) continue;
                    int c = all[list[i]].cyl;
                    if (c >= head && (pick < 0 || c < all[list[pick]].cyl)) pick = i;
                    if (low < 0 || c < all[list[low]].cyl) low = i;
                }
                if (pick < 0) pick = low;
            }
            int c = all[list[pick]].cyl;
            double svc = hdd_service(abs(c - head));
            now += svc; busy += svc; head = c; done[pick] = 1; served++;
            double l = now - all[list[pick]].t;
            lat[nl++] = l;
            if (l > hdd_max_lat) hdd_max_lat = l;
        }
        hdd_busy_total += busy / now;
        hdd_reqs += m;
        free(done);
    }

    qsort(lat, nl, sizeof *lat, cmp_dbl);
    double sum = 0;
    for (int i = 0; i < nl; i++) sum += lat[i];
    printf("%-42s %5d %6.1f%% %9.1f %9.1f %9.1f %10.1f %7.0f%%\n", name, H,
           100.0 * ssd_hits / n, sum / nl, lat[(int)(0.95 * nl)], lat[(int)(0.99 * nl)], lat[nl - 1],
           H ? 100.0 * hdd_busy_total / H : 0.0);
    free(all); free(lat);
    (void)hdd_reqs; (void)hdd_max_lat;
}

int main(void)
{
    double s = 0;
    for (int i = 0; i < VIDEOS; i++) { s += 1.0 / (i + 1); cumw[i] = s; }
    printf("%-42s %5s %7s %9s %9s %9s %10s %8s\n", "design", "HDDs", "SSD hit", "mean ms", "p95 ms", "p99 ms", "max ms", "HDD busy");
    run("A: 96 HDD, FCFS, no SSD cache", 96, 0, 0);
    run("A2: 24 HDD, FCFS, no SSD cache", 24, 0, 0);
    run("A3: 24 HDD, C-LOOK, no SSD cache", 24, 0, 1);
    run("B2: 24 HDD, FCFS, SSD cache (top 1000)", 24, 1000, 0);
    run("B: 24 HDD, C-LOOK, SSD cache (top 1000)", 24, 1000, 1);
    return 0;
}
