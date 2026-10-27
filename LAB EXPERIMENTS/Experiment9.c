#include <stdio.h>

int main() {
    int i, n, sum = 0, count = 0, y, quant, wt = 0, tat = 0, at[10], bt[10], temp[10];
    float avg_wt, avg_tat;

    printf("Total number of processes in system: ");
    scanf("%d", &n);
    y = n;

    for (i = 0; i < n; i++) {
        printf("\nEnter the Burst Time of Process[%d]: ", i + 1);
        scanf("%d", &bt[i]);
        at[i] = 0; // Assuming arrival time is 0 for simplicity
        temp[i] = bt[i];
    }

    printf("Enter the Time Quantum: ");
    scanf("%d", &quant);

    printf("\nProcess No \t Burst Time \t TAT \t Waiting Time");
    for (sum = 0, i = 0; y != 0;) {
        if (temp[i] <= quant && temp[i] > 0) {
            sum += temp[i];
            temp[i] = 0;
            count = 1;
        } else if (temp[i] > 0) {
            temp[i] -= quant;
            sum += quant;
        }
        if (temp[i] == 0 && count == 1) {
            y--;
            printf("\nProcess[%d] \t\t %d \t\t %d \t\t %d", i + 1, bt[i], sum - at[i], sum - at[i] - bt[i]);
            wt += sum - at[i] - bt[i];
            tat += sum - at[i];
            count = 0;
        }
        if (i == n - 1)
            i = 0;
        else if (at[i + 1] <= sum)
            i++;
        else
            i = 0;
    }

    avg_wt = (float)wt / n;
    avg_tat = (float)tat / n;
    printf("\n\nAverage Waiting Time: %.2f", avg_wt);
    printf("\nAverage Turnaround Time: %.2f\n", avg_tat);

    return 0;
}