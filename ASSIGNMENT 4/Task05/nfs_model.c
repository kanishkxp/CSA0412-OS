/* Task 5 alternative concept: one big NFS server versus a replicated distributed file system.
   Simple M/M/1 queue model per server: response time T = 1 / (mu - lambda).
   Assumptions (mine): each server has a 10 Gbit/s link = 1250 MB/s, every request moves 4 MB,
   so one server completes mu = 312.5 requests per second. Each user sends one request every 2 s.
   The distributed system has 6 servers and 3 replicas: reads go to any one replica (load is
   shared by 6 servers), a write must be done on 3 servers (3 times the work).
   Compile: gcc -std=c11 -Wall -Wextra nfs_model.c -o nfs_model -lm                              */
#include <stdio.h>
#include <math.h>

int main(void)
{
    double mu = 1250.0 / 4.0;                 /* requests per second one server can finish */
    int users[] = {100, 300, 500, 600, 1000, 3000, 6000};
    printf("%6s %10s %16s %16s %16s\n", "users", "req/s", "NFS 1 server ms", "DFS read ms", "DFS write ms");
    for (int i = 0; i < 7; i++) {
        double lambda = users[i] / 2.0;
        double t_nfs = (lambda < mu) ? 1000.0 / (mu - lambda) : -1;
        double lr = lambda / 6.0;                         /* read load per server */
        double lw = lambda * 3.0 / 6.0;                   /* write load per server (3 copies) */
        double t_r = (lr < mu) ? 1000.0 / (mu - lr) : -1;
        double t_w = (lw < mu) ? 1000.0 / (mu - lw) : -1;
        printf("%6d %10.0f ", users[i], lambda);
        if (t_nfs < 0) printf("%16s ", "overloaded"); else printf("%16.1f ", t_nfs);
        if (t_r < 0) printf("%16s ", "overloaded"); else printf("%16.1f ", t_r);
        if (t_w < 0) printf("%16s\n", "overloaded"); else printf("%16.1f\n", t_w);
    }
    double a = 0.995;                                    /* one server is up 99.5% of the time */
    printf("\nAvailability of one server        : %.4f %%  (down about %.1f hours a year)\n", a * 100, (1 - a) * 8760);
    double a3 = 1 - pow(1 - a, 3);
    printf("Availability with 3 replicas      : %.6f %%  (down about %.4f hours a year)\n", a3 * 100, (1 - a3) * 8760);
    printf("Storage for 100 TB of data        : NFS 1 copy = 100 TB, 3 replicas = 300 TB\n");
    return 0;
}
