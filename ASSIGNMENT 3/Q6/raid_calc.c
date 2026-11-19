/* raid_calc.c - usable capacity of 6 disks x 4 TB at each RAID level, and probability of data loss.
 * Model 1 (one year): every disk fails independently with probability p during the year (AFR). Data is lost when more
 *   disks fail than the level can tolerate (RAID 1 = a mirrored pair, RAID 10 = 3 mirrored pairs striped).
 * Model 2 (rebuild window): after the first failure, data is lost if another disk (a second one for RAID 5, the mirror
 *   partner for RAID 1/10) fails before the rebuild finishes. */
#include <stdio.h>
#include <math.h>

static double C(int n, int k) { double r = 1; for (int i = 1; i <= k; i++) r = r * (n - k + i) / i; return r; }
/* probability that at least t of n disks fail */
static double at_least(int n, int t, double p) { double s = 0; for (int k = t; k <= n; k++) s += C(n, k) * pow(p, k) * pow(1 - p, n - k); return s; }

int main(void)
{
    int n = 6; double tb = 4.0;
    puts("=== Usable capacity, 6 disks of 4 TB (raw 24 TB) ===");
    printf("RAID 0  (striping)                  : %5.1f TB  (%3.0f%%)  survives 0 failures\n", n * tb, 100.0);
    printf("RAID 1  (all 6 disks mirror)        : %5.1f TB  (%3.0f%%)  survives 5 failures\n", tb, 100.0 / n);
    printf("RAID 1  (3 mirrored pairs, no stripe): %4.1f TB  (%3.0f%%)  survives 1 per pair\n", n / 2 * tb, 50.0);
    printf("RAID 5  (single parity)             : %5.1f TB  (%3.0f%%)  survives 1 failure\n", (n - 1) * tb, 100.0 * (n - 1) / n);
    printf("RAID 6  (double parity)             : %5.1f TB  (%3.0f%%)  survives 2 failures\n", (n - 2) * tb, 100.0 * (n - 2) / n);
    printf("RAID 10 (3 mirrored pairs, striped) : %5.1f TB  (%3.0f%%)  survives 1 per pair (at least 1, at most 3)\n", n / 2 * tb, 50.0);

    double p = 0.02;                 /* assumed annual failure rate of one disk: 2% */
    puts("\n=== Model 1: probability of data loss within one year (disk failure probability p = 2% per year, independent) ===");
    printf("RAID 0           : %.6f  (%.4f%%)  any failure\n", at_least(6, 1, p), 100 * at_least(6, 1, p));
    printf("RAID 1 (2 disks) : %.8f  (%.4f%%)  both disks of the mirror fail = p^2\n", p * p, 100 * p * p);
    printf("RAID 5 (6 disks) : %.8f  (%.4f%%)  2 or more of 6 fail\n", at_least(6, 2, p), 100 * at_least(6, 2, p));
    printf("RAID 6 (6 disks) : %.10f (%.5f%%) 3 or more of 6 fail\n", at_least(6, 3, p), 100 * at_least(6, 3, p));
    double r10 = 1 - pow(1 - p * p, 3);
    printf("RAID 10 (3 pairs): %.8f  (%.4f%%)  both disks of any pair fail = 1-(1-p^2)^3\n", r10, 100 * r10);

    double hours = 24.0, pw = p * hours / 8760.0;      /* chance that one given disk fails inside the rebuild window */
    puts("\n=== Model 2: after one disk has failed, a 24-hour rebuild window (AFR 2% -> per-disk window failure probability) ===");
    printf("probability that one given disk fails in 24 h = %.3e\n", pw);
    printf("RAID 1 (2 disks) : the single partner fails in the window      : %.3e\n", pw);
    printf("RAID 5 (6 disks) : any of the 5 remaining disks fails          : %.3e\n", 1 - pow(1 - pw, 5));
    printf("RAID 6 (6 disks) : 2 more of the 5 remaining fail              : %.3e\n", at_least(5, 2, pw));
    printf("RAID 10          : the mirror partner of the failed disk fails : %.3e\n", pw);
    printf("expected first failures per year in a 6-disk array: %.2f\n", 6 * p);
    printf("-> RAID 5 data-loss events per year ~ 6p x P(window) = %.3e (once in %.0f years)\n", 6 * p * (1 - pow(1 - pw, 5)), 1 / (6 * p * (1 - pow(1 - pw, 5))));
    printf("-> RAID 1/10 per year ~ 6p x P(partner) = %.3e (once in %.0f years)\n", 6 * p * pw, 1 / (6 * p * pw));
    printf("-> RAID 6 per year ~ 6p x P(2 more) = %.3e (once in %.0f years)\n", 6 * p * at_least(5, 2, pw), 1 / (6 * p * at_least(5, 2, pw)));
    return 0;
}
