#include <stdio.h>
#include <stdlib.h>
int main()
{
    int req[50], n, head, i, seek = 0, cur;
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
        seek += abs(req[i] - cur);
        cur = req[i];
        printf(" -> %d", cur);
    }
    printf("\n\nTotal Seek Time = %d", seek);
    printf("\nAverage Seek Time = %.2f\n", (float)seek / n);
    return 0;
}
