/* calc_q2.c - page-table sizes and effective memory-access time (EMAT) */
#include <stdio.h>
#include <math.h>
int main(void)
{
    puts("=== Page table size, 4 KB pages (single-level table) ===");
    double pg = 4096;
    double e32 = pow(2, 32) / pg, e48 = pow(2, 48) / pg;
    printf("32-bit space: %.0f entries x 4 bytes = %.0f bytes = %.1f MB\n", e32, e32 * 4, e32 * 4 / 1048576.0);
    printf("48-bit space: %.0f entries x 8 bytes = %.0f bytes = %.0f GB (per process!)\n", e48, e48 * 8, e48 * 8 / 1073741824.0);
    printf("   48-bit as 4-level tree, 512 entries/table: one fully used leaf table = %d bytes\n", 512 * 8);

    puts("\n=== Effective memory-access time ===");
    double m = 100.0, t = 10.0;   /* memory access 100 ns, TLB lookup 10 ns (assumed) */
    printf("assumptions: memory access = %.0f ns, TLB lookup = %.0f ns, single-level page table\n", m, t);
    printf("without TLB            : %.1f ns (page-table access + data access)\n", 2 * m);
    double hr[] = {0.80, 0.95, 0.99};
    for (int i = 0; i < 3; i++) {
        double h = hr[i];
        double emat = h * (t + m) + (1 - h) * (t + 2 * m);
        printf("with TLB, hit ratio %2.0f%%: %.1f ns   (slow-down vs. no paging: %.1f%%)\n", h * 100, emat, (emat - m) / m * 100);
    }
    puts("with 4-level table, miss costs 4 table reads + data:");
    for (int i = 0; i < 3; i++) {
        double h = hr[i];
        double emat = h * (t + m) + (1 - h) * (t + 5 * m);
        printf("   hit ratio %2.0f%%: %.1f ns\n", h * 100, emat);
    }
    return 0;
}
