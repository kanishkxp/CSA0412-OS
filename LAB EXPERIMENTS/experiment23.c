#include <stdio.h>
#include <stdlib.h>
int main()
{
    int req[50], n, head, size, i, j, temp, pos, seek = 0, cur;
    printf("Enter number of disk requests: ");
    scanf("%d", &n);
    printf("Enter request queue:\n");
    for(i = 0; i < n; i++)
        scanf("%d", &req[i]);
    printf("Enter initial head position: ");
    scanf("%d", &head);
    printf("Enter disk size: ");
    scanf("%d", &size);

    for(i = 0; i < n - 1; i++)
        for(j = 0; j < n - i - 1; j++)
            if(req[j] > req[j + 1])
            {
                temp = req[j];
                req[j] = req[j + 1];
                req[j + 1] = temp;
            }

    for(pos = 0; pos < n && req[pos] < head; pos++);

    cur = head;
    printf("\nHead movement: %d", cur);
    for(i = pos; i < n; i++)
    {
        seek += abs(req[i] - cur);
        cur = req[i];
        printf(" -> %d", cur);
    }
    seek += abs((size - 1) - cur);
    cur = size - 1;
    printf(" -> %d", cur);
    for(i = pos - 1; i >= 0; i--)
    {
        seek += abs(req[i] - cur);
        cur = req[i];
        printf(" -> %d", cur);
    }
    printf("\n\nTotal Seek Time = %d\n", seek);
    return 0;
}
