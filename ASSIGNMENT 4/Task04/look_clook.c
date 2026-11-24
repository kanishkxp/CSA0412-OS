/* Task 4.1 extra: LOOK and C-LOOK for the queue 98 183 37 122 14 124 65 67, head at 53.
   LOOK goes down first (like the SCAN program), C-LOOK goes up first (like C-SCAN). */
#include <stdio.h>
#include <stdlib.h>

#define N 8
int req[N] = {98, 183, 37, 122, 14, 124, 65, 67};
int cmp(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

int main(void)
{
    int head = 53, s[N], order[N], n = 0, total = 0, pos = head;
    for (int i = 0; i < N; i++) s[i] = req[i];
    qsort(s, N, sizeof s[0], cmp);

    /* LOOK: down to the lowest request, then up to the highest */
    for (int i = N - 1; i >= 0; i--) if (s[i] < head) order[n++] = s[i];
    for (int i = 0; i < N; i++) if (s[i] > head) order[n++] = s[i];
    printf("LOOK   %d", head);
    for (int i = 0; i < n; i++) { printf(" %d", order[i]); total += abs(order[i] - pos); pos = order[i]; }
    printf("  -> %d cylinders\n", total);

    /* C-LOOK: up to the highest request, jump to the lowest, then up again */
    n = 0; total = 0; pos = head;
    for (int i = 0; i < N; i++) if (s[i] > head) order[n++] = s[i];
    int up = 0, p = head;
    for (int i = 0; i < n; i++) { up += abs(order[i] - p); p = order[i]; }
    int jump = abs(p - s[0]);                         /* from the highest to the lowest request */
    int low = 0; p = s[0];
    for (int i = 0; i < N; i++) if (s[i] < head) { low += abs(s[i] - p); p = s[i]; }
    printf("C-LOOK %d", head);
    for (int i = 0; i < n; i++) printf(" %d", order[i]);
    for (int i = 0; i < N; i++) if (s[i] < head) printf(" %d", s[i]);
    printf("  -> %d cylinders (+%d for the jump = %d)\n", up + low, jump, up + low + jump);
    return 0;
}
