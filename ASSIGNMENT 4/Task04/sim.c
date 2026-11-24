/* Task 4.2: disk scheduling simulation.
   10,000 random requests on a disk with cylinders 0-199 and three loads.
   Service time of a request = 2.0 ms (rotation + transfer) + 0.05 ms per cylinder moved.
   Algorithms: FCFS, SSTF, SCAN, C-SCAN, LOOK, C-LOOK.
   Compile: gcc -std=c11 -Wall -Wextra sim.c -o sim -lm                                  */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define NREQ 10000
#define MAXCYL 199
#define FIXED_MS 2.0
#define SEEK_MS 0.05

enum { FCFS, SSTF, SCAN, CSCAN, LOOK, CLOOK, NALGO };
const char *algo_name[NALGO] = {"FCFS", "SSTF", "SCAN", "C-SCAN", "LOOK", "C-LOOK"};

static double arr[NREQ];
static int cyl[NREQ];
static char done[NREQ];

static double rnd(void) { return (rand() + 1.0) / (RAND_MAX + 2.0); }

static void make_requests(double mean_gap_ms)
{
    srand(12345);                                   /* same requests for every algorithm */
    double t = 0;
    for (int i = 0; i < NREQ; i++) {
        t += -mean_gap_ms * log(rnd());             /* exponential gaps = Poisson arrivals */
        arr[i] = t;
        cyl[i] = rand() % (MAXCYL + 1);
    }
}

typedef struct { double avg_move, avg_wait, max_wait, util, sd_wait, edge_wait, mid_wait; } Result;

static Result run(int algo)
{
    memset(done, 0, sizeof done);
    double t = 0, wait_sum = 0, wait_sq = 0, wait_max = 0, busy = 0, edge_sum = 0, mid_sum = 0;
    long edge_n = 0, mid_n = 0;
    long moved = 0;
    int head = 100, dir = +1, served = 0, lo = 0, hi = 0;   /* hi = how many requests have arrived */

    while (served < NREQ) {
        while (lo < NREQ && done[lo]) lo++;
        while (hi < NREQ && arr[hi] <= t) hi++;
        if (hi == lo) { t = arr[lo]; continue; }             /* nothing waiting: wait for an arrival */

        int pick = -1;
        if (algo == FCFS) {
            pick = lo;                                       /* oldest pending request */
        } else if (algo == SSTF) {
            for (int i = lo; i < hi; i++)
                if (!done[i] && (pick < 0 || abs(cyl[i] - head) < abs(cyl[pick] - head))) pick = i;
        } else {
            /* nearest pending request in the current direction (SCAN, LOOK) or upwards (C-SCAN, C-LOOK) */
            int d = (algo == CSCAN || algo == CLOOK) ? +1 : dir;
            for (int i = lo; i < hi; i++) {
                if (done[i]) continue;
                int ahead = (d > 0) ? cyl[i] - head : head - cyl[i];
                if (ahead >= 0 && (pick < 0 || ahead < ((d > 0) ? cyl[pick] - head : head - cyl[pick]))) pick = i;
            }
            if (pick < 0) {                                  /* nothing ahead of the head */
                int travel = 0;
                if (algo == SCAN) {                          /* go to the end of the disk, then reverse */
                    travel = (dir > 0) ? MAXCYL - head : head;
                    head = (dir > 0) ? MAXCYL : 0;
                    dir = -dir;
                } else if (algo == CSCAN) {                  /* go to the last cylinder, jump to 0 */
                    travel = (MAXCYL - head) + MAXCYL;
                    head = 0;
                } else if (algo == LOOK) {                   /* just reverse, no trip to the end */
                    dir = -dir;
                } else {                                     /* C-LOOK: jump to the lowest pending request */
                    int low = MAXCYL + 1;
                    for (int i = lo; i < hi; i++) if (!done[i] && cyl[i] < low) low = cyl[i];
                    travel = head - low;
                    head = low;
                }
                moved += travel;
                t += travel * SEEK_MS;
                busy += travel * SEEK_MS;
                continue;                                    /* decide again from the new position */
            }
        }
        int dist = abs(cyl[pick] - head);
        double service = FIXED_MS + SEEK_MS * dist;
        double w = t - arr[pick];
        wait_sum += w;
        wait_sq += w * w;
        if (cyl[pick] < 20 || cyl[pick] > 179) { edge_sum += w; edge_n++; }      /* edge cylinders */
        if (cyl[pick] >= 80 && cyl[pick] < 120) { mid_sum += w; mid_n++; }       /* middle cylinders */
        if (w > wait_max) wait_max = w;
        moved += dist;
        busy += service;
        t += service;
        head = cyl[pick];
        done[pick] = 1;
        served++;
    }
    double mean = wait_sum / NREQ;
    Result r = {(double)moved / NREQ, mean, wait_max, busy / t, sqrt(wait_sq / NREQ - mean * mean),
                edge_sum / edge_n, mid_sum / mid_n};
    return r;
}

int main(void)
{
    const char *load_name[3] = {"light", "medium", "heavy"};
    double gap[3] = {40.0, 8.0, 5.0};                        /* mean ms between arrivals */
    printf("load,mean_gap_ms,algorithm,avg_head_movement,avg_wait_ms,max_wait_ms,disk_busy_percent,std_wait_ms,edge_wait_ms,middle_wait_ms\n");
    for (int l = 0; l < 3; l++) {
        make_requests(gap[l]);
        for (int a = 0; a < NALGO; a++) {
            Result r = run(a);
            printf("%s,%.0f,%s,%.1f,%.1f,%.1f,%.0f,%.1f,%.1f,%.1f\n", load_name[l], gap[l], algo_name[a],
                   r.avg_move, r.avg_wait, r.max_wait, r.util * 100.0, r.sd_wait, r.edge_wait, r.mid_wait);
        }
    }
    return 0;
}
