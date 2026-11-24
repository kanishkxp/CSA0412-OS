/* Task 4 extra: starvation of cylinder 183 under SSTF when a stream of requests near
   cylinder 60 arrives every millisecond. Compared with LOOK on the same input.
   Service time = 1.0 ms + 0.05 ms per cylinder moved.                              */
#include <stdio.h>
#include <stdlib.h>

#define MAXR 5000
static int cyl[MAXR];
static double arr[MAXR];
static char done[MAXR];
static int n;

static void add(double t, int c) { arr[n] = t; cyl[n] = c; n++; }

static double run(int use_sstf, int *served_before)
{
    for (int i = 0; i < n; i++) done[i] = 0;
    int head = 53, dir = +1, left = n, target = -1;
    double t = 0;
    for (int i = 0; i < n; i++) if (cyl[i] == 183) target = i;
    *served_before = 0;
    while (left > 0) {
        int pick = -1;
        for (int i = 0; i < n; i++) {
            if (done[i] || arr[i] > t) continue;
            if (use_sstf) {
                if (pick < 0 || abs(cyl[i] - head) < abs(cyl[pick] - head)) pick = i;
            } else {
                int ahead = (dir > 0) ? cyl[i] - head : head - cyl[i];
                if (ahead >= 0 && (pick < 0 || ahead < ((dir > 0) ? cyl[pick] - head : head - cyl[pick]))) pick = i;
            }
        }
        if (pick < 0) {
            if (!use_sstf) { dir = -dir; continue; }   /* LOOK: reverse when nothing is ahead */
            t += 0.1; continue;                        /* idle */
        }
        t += 1.0 + 0.05 * abs(cyl[pick] - head);
        head = cyl[pick];
        done[pick] = 1; left--;
        if (pick == target) return t - arr[target];
        (*served_before)++;
    }
    return -1;
}

int main(void)
{
    int original[] = {98, 183, 37, 122, 14, 124, 65, 67};
    for (int i = 0; i < 8; i++) add(0.0, original[i]);
    int stream_ms[] = {0, 100, 500, 2000};
    printf("%-22s %-14s %-14s\n", "stream length (ms)", "SSTF wait (ms)", "LOOK wait (ms)");
    for (int s = 0; s < 4; s++) {
        n = 8;
        for (int ms = 1; ms <= stream_ms[s]; ms++) add((double)ms, 58 + (ms % 5));   /* cylinders 58..62 */
        int a, b;
        double w1 = run(1, &a), w2 = run(0, &b);
        printf("%-22d %-14.1f %-14.1f   (requests served before 183: SSTF %d, LOOK %d)\n",
               stream_ms[s], w1, w2, a, b);
    }
    return 0;
}
