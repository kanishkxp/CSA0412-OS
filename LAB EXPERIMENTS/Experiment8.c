#include <stdio.h>

int main() {
    int p[20], bt[20], pri[20], wt[20], tat[20], i, j, n, total = 0, pos, temp;

    printf("Enter Total Number of Processes: ");
    scanf("%d", &n);

    printf("\nEnter Burst Time and Priority\n");
    for (i = 0; i < n; i++) {
        p[i] = i + 1;
        printf("P[%d] Burst Time: ", i + 1);
        scanf("%d", &bt[i]);
        printf("P[%d] Priority: ", i + 1);
        scanf("%d", &pri[i]);
    }

    // Sorting processes by priority (ascending order: lower number = higher priority)
    for (i = 0; i < n; i++) {
        pos = i;
        for (j = i + 1; j < n; j++) {
            if (pri[j] < pri[pos])
                pos = j;
        }

        temp = pri[i];
        pri[i] = pri[pos];
        pri[pos] = temp;

        temp = bt[i];
        bt[i] = bt[pos];
        bt[pos] = temp;

        temp = p[i];
        p[i] = p[pos];
        p[pos] = temp;
    }

    wt[0] = 0; // Waiting time for first process is zero

    for (i = 1; i < n; i++) {
        wt[i] = 0;
        for (j = 0; j < i; j++)
            wt[i] += bt[j];
        total += wt[i];
    }

    float avg_wt = (float)total / n;
    total = 0;

    printf("\nProcess\t    Priority\tBurst Time\tWaiting Time\tTurnaround Time");
    for (i = 0; i < n; i++) {
        tat[i] = bt[i] + wt[i];
        total += tat[i];
        printf("\nP[%d]\t\t%d\t\t%d\t\t%d\t\t%d", p[i], pri[i], bt[i], wt[i], tat[i]);
    }

    float avg_tat = (float)total / n;
    printf("\n\nAverage Waiting Time = %.2f", avg_wt);
    printf("\nAverage Turnaround Time = %.2f\n", avg_tat);

    return 0;
}