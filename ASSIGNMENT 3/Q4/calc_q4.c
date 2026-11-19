/* calc_q4.c - effective access time (EAT) with page faults, maximum fault rate, frame allocation. */
#include <stdio.h>
int main(void)
{
    double m = 100.0, f = 8e6;     /* memory access 100 ns, page-fault service time 8 ms = 8,000,000 ns */
    double p[3] = {1e-3, 1e-5, 1e-6}; const char *lab[3] = {"1 in 1,000", "1 in 100,000", "1 in 1,000,000"};
    puts("EAT = (1 - p) x 100 ns + p x 8,000,000 ns");
    for (int i = 0; i < 3; i++) {
        double eat = (1 - p[i]) * m + p[i] * f;
        printf("p = %-16s: EAT = %12.4f ns   slow-down = %9.2f x   (%.1f%% extra)\n", lab[i], eat, eat / m, (eat - m) / m * 100);
    }
    double pmax = 0.10 * m / (f - m);
    printf("\nFor a 10%% slow-down: EAT <= 110 ns -> p <= 10 / (8,000,000 - 100) = %.3e  (about 1 fault in %.0f accesses)\n", pmax, 1 / pmax);
    puts("\nSame with the fault service time measured on this machine (about 110 us, from output_pressure.txt: time per access divided by fault rate at 768 and 1024 MB):");
    double fm = 110e3;
    for (int i = 0; i < 3; i++) { double eat = (1 - p[i]) * m + p[i] * fm; printf("p = %-16s: EAT = %10.3f ns\n", lab[i], eat); }
    printf("max p for 10%% slow-down with 110 us service time: %.3e (1 in %.0f)\n", 10.0 / (fm - m), (fm - m) / 10.0);

    puts("\n=== Alternative: 32 GB RAM, program of 24 GB ===");
    double pages = 24.0 * 1024 * 1024 / 4;      /* 24 GB in 4 KB pages */
    printf("24 GB = %.0f pages. With enough RAM every page is brought in once (cold start) and then never again.\n", pages);
    printf("  cold-start minor faults at 1.5 us (measured, anonymous memory)  : %.1f s once\n", pages * 1.5e-6);
    printf("  steady state EAT with 32 GB                                     : 100 ns (p = 0)\n");
    puts("  with 16 GB RAM the same program has p > 0 after the first 16 GB is used:");
    double pp[3] = {1e-3, 1e-5, 1e-6};
    for (int i = 0; i < 3; i++) printf("     p = %.0e : EAT = %9.1f ns (8 ms) -> %6.1f x slower than 32 GB\n", pp[i], (1 - pp[i]) * m + pp[i] * f, ((1 - pp[i]) * m + pp[i] * f) / m);
    puts("\n=== Frame allocation example: 62 free frames, process A = 10 pages, process B = 127 pages ===");
    printf("equal allocation       : A = %d frames, B = %d frames\n", 31, 31);
    printf("proportional allocation: A = %.0f frames, B = %.0f frames  (frames = size / total size x 62)\n", 10.0 / 137 * 62, 127.0 / 137 * 62);
    return 0;
}
