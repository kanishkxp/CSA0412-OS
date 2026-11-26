#include <stdio.h>
int main()
{
    int disk[100] = {0}, nf, i, j, start, len, ok;
    printf("Enter number of files: ");
    scanf("%d", &nf);
    for(i = 1; i <= nf; i++)
    {
        printf("\nEnter starting block and length of file %d: ", i);
        scanf("%d %d", &start, &len);
        ok = 1;
        for(j = start; j < start + len; j++)
            if(j >= 100 || disk[j] == 1)
                ok = 0;
        if(ok)
        {
            for(j = start; j < start + len; j++)
                disk[j] = 1;
            printf("File %d allocated: blocks ", i);
            for(j = start; j < start + len; j++)
                printf("%d ", j);
            printf("\n");
        }
        else
            printf("File %d NOT allocated (block already in use)\n", i);
    }
    return 0;
}
