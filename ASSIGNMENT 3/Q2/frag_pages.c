/* frag_pages.c - internal fragmentation of 100 processes of random size for 1 KB, 4 KB and 2 MB pages.
 * Sizes are log-uniform between 8 KB and 50 MB (many small programs, a few big ones). Fixed seed -> repeatable. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
int main(void)
{
    enum { N = 100 };
    double size[N]; srand(2026);
    for (int i = 0; i < N; i++) {
        double u = rand() / (RAND_MAX + 1.0);
        size[i] = floor(exp(log(8192.0) + u * (log(50.0 * 1048576) - log(8192.0))));
    }
    double total = 0; for (int i = 0; i < N; i++) total += size[i];
    printf("100 processes, total size %.1f MB\n", total / 1048576);
    double mn = size[0], mx = size[0];
    for (int i = 1; i < N; i++) { if (size[i] < mn) mn = size[i]; if (size[i] > mx) mx = size[i]; }
    printf("smallest process = %.1f KB, largest process = %.1f MB\n\n", mn / 1024, mx / 1048576);
    double ps[] = {1024, 4096, 2 * 1048576};
    const char *name[] = {"1 KB", "4 KB", "2 MB"};
    printf("%-8s %12s %14s %16s %12s %14s\n", "page", "total pages", "page-table(8B)", "internal frag", "avg/process", "waste % of mem");
    for (int k = 0; k < 3; k++) {
        double pages = 0, waste = 0;
        for (int i = 0; i < N; i++) { double p = ceil(size[i] / ps[k]); pages += p; waste += p * ps[k] - size[i]; }
        printf("%-8s %12.0f %11.1f KB %13.2f MB %9.1f KB %13.2f %%\n", name[k], pages, pages * 8 / 1024, waste / 1048576, waste / N / 1024, 100.0 * waste / (total + waste));
    }
    return 0;
}
