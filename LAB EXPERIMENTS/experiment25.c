#include <stdio.h>
int main()
{
    int p[50], f[20], last[20];
    int n, m, i, j, pos, found;
    int faults = 0, hits = 0;
    printf("Enter number of pages: ");
    scanf("%d", &n);
    printf("Enter page reference string:\n");
    for(i = 0; i < n; i++)
        scanf("%d", &p[i]);
    printf("Enter number of frames: ");
    scanf("%d", &m);
    for(i = 0; i < m; i++)
    {
        f[i] = -1;
        last[i] = -1;
    }
    for(i = 0; i < n; i++)
    {
        found = 0;
        for(j = 0; j < m; j++)
        {
            if(f[j] == p[i])
            {
                found = 1;
                last[j] = i;
                break;
            }
        }
        if(!found)
        {
            pos = -1;
            for(j = 0; j < m; j++)
                if(f[j] == -1)
                {
                    pos = j;
                    break;
                }
            if(pos == -1)
            {
                pos = 0;
                for(j = 1; j < m; j++)
                    if(last[j] < last[pos])
                        pos = j;
            }
            f[pos] = p[i];
            last[pos] = i;
            faults++;
        }
        else
            hits++;
    }
    printf("\nPage Faults = %d", faults);
    printf("\nPage Hits = %d", hits);
    return 0;
}


