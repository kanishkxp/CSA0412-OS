#include <stdio.h>
int main()
{
    int b[20], p[20], m, n, i, j, worst;
    printf("Enter number of blocks: ");
    scanf("%d", &m);
    printf("Enter block sizes:\n");
    for(i = 0; i < m; i++)
        scanf("%d", &b[i]);
    printf("Enter number of processes: ");
    scanf("%d", &n);
    printf("Enter process sizes:\n");
    for(i = 0; i < n; i++)
        scanf("%d", &p[i]);
    for(i = 0; i < n; i++)
    {
        worst = -1;
        for(j = 0; j < m; j++)
        {
            if(b[j] >= p[i])
            {
                if(worst == -1 || b[j] > b[worst])
                    worst = j;
            }
        }
        if(worst != -1)
        {
            printf("Process %d allocated to Block %d\n",
                   i + 1, worst + 1);
            b[worst] -= p[i];
        }
        else
            printf("Process %d Not Allocated\n", i + 1);
    }
    return 0;
}
