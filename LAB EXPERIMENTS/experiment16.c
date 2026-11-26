#include <stdio.h>
int main()
{
    int p[50], f[20], n, m, i, j;
    int faults = 0, hits = 0, pos = 0, found;
    printf("Enter number of pages: ");
    scanf("%d", &n);
    printf("Enter page reference string:\n");
    for(i = 0; i < n; i++)
        scanf("%d", &p[i]);
    printf("Enter number of frames: ");
    scanf("%d", &m);
    for(i = 0; i < m; i++)
        f[i] = -1;
    for(i = 0; i < n; i++)
    {
        found = 0;
        for(j = 0; j < m; j++)
            if(f[j] == p[i])
                found = 1;
        if(!found)
        {
            f[pos] = p[i];
            pos = (pos + 1) % m;
            faults++;
        }
        else
            hits++;
    }
    printf("\nPage Faults = %d", faults);
    printf("\nPage Hits = %d", hits);
    return 0;
}
