#include <stdio.h>
#include <stdlib.h>
int main()
{
    int req[50], done[50] = {0}, n, head, i, j, idx, min, seek = 0, cur;
    printf("Enter number of disk requests: ");
    scanf("%d", &n);
    printf("Enter request queue:\n");
    for(i = 0; i < n; i++)
        scanf("%d", &req[i]);
    printf("Enter initial head position: ");
    scanf("%d", &head);

    cur = head;
    printf("\nHead movement: %d", cur);
    for(i = 0; i < n; i++)
    {
        min = 9999;
        idx = -1;
        for(j = 0; j < n; j++)
        {
            if(!done[j] && abs(req[j] - cur) < min)
            {
                min = abs(req[j] - cur);
                idx = j;
            }
        }
        done[idx] = 1;
        seek += min;
        cur = req[idx];
        printf(" -> %d", cur);
    }
    printf("\n\nTotal Seek Time = %d", seek);
    printf("\nAverage Seek Time = %.2f\n", (float)seek / n);
    return 0;
}
