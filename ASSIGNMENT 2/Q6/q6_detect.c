/* Q6 - Alternative concept: deadlock detection and recovery for the research-centre cluster
 * Name: Kanishk C   Reg No: 192511278
 *
 * Resources: R0 = GPU, R1 = simulation licence, R2 = fast storage (one unit of each).
 * Jobs J0..J3. Unlike the Banker's algorithm, detection needs no maximum demand in advance:
 * it only looks at what each job HOLDS (alloc) and what it is WAITING for (request).
 * A job that can get what it waits for may finish and release what it holds; jobs that can
 * never finish are deadlocked. Running time is O(m * n^2), same as the safety algorithm.
 *
 * Recovery: abort the deadlocked job that has done the least work (cheapest to lose), release
 * its resources and detect again.
 *
 * Build: gcc -std=c11 -Wall -Wextra -o q6_detect q6_detect.c
 */
#include <stdio.h>
#include <string.h>

#define N 4
#define M 3

static const char *rname[M] = {"GPU", "LICENCE", "STORAGE"};
static int avail[M]      = {0, 0, 0};
static int alloc[N][M]   = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0, 0, 0}};   /* what each job holds    */
static int request[N][M] = {{0, 1, 0}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}};   /* what each job waits for */
static int minutes_done[N] = {40, 15, 90, 5};                               /* work done so far (minutes) */
static int aborted[N];

/* fills dead[] (1 = deadlocked) and returns how many jobs are deadlocked */
static int detect(int dead[N])
{
    int work[M], finish[N];
    memcpy(work, avail, sizeof work);
    for (int p = 0; p < N; p++) {
        int holds = 0;
        for (int r = 0; r < M; r++) holds |= alloc[p][r];
        finish[p] = !holds || aborted[p];            /* a job holding nothing cannot be part of a deadlock */
    }
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int p = 0; p < N; p++) {
            if (finish[p]) continue;
            int ok = 1;
            for (int r = 0; r < M; r++) if (request[p][r] > work[r]) { ok = 0; break; }
            if (ok) {
                for (int r = 0; r < M; r++) work[r] += alloc[p][r];
                finish[p] = 1; changed = 1;
            }
        }
    }
    int n = 0;
    for (int p = 0; p < N; p++) { dead[p] = !finish[p]; n += dead[p]; }
    return n;
}

int main(void)
{
    int dead[N];
    int n = detect(dead);
    printf("detection run 1: %d deadlocked job(s):", n);
    for (int p = 0; p < N; p++) if (dead[p]) printf(" J%d", p);
    printf("\n");
    for (int p = 0; p < N; p++) if (dead[p]) {
        printf("   J%d holds", p);
        for (int r = 0; r < M; r++) if (alloc[p][r]) printf(" %s", rname[r]);
        printf(", waits for");
        for (int r = 0; r < M; r++) if (request[p][r]) printf(" %s", rname[r]);
        printf(", work done = %d min\n", minutes_done[p]);
    }

    int victim = -1;
    for (int p = 0; p < N; p++)
        if (dead[p] && (victim < 0 || minutes_done[p] < minutes_done[victim])) victim = p;
    printf("recovery: abort J%d (least work done), %d minutes of work are lost\n", victim, minutes_done[victim]);
    aborted[victim] = 1;
    for (int r = 0; r < M; r++) { avail[r] += alloc[victim][r]; alloc[victim][r] = 0; request[victim][r] = 0; }

    n = detect(dead);
    printf("detection run 2: %d deadlocked job(s)%s\n", n, n == 0 ? " -> the cluster is running again" : "");
    return 0;
}
