/* faults_frames.c - page faults against the number of frames (CSV), for FIFO, LRU and Optimal.
 * String A: the 20-reference textbook string. String B: 2000 references with locality (generated, fixed seed).
 * Output goes to faults_vs_frames.csv (used by make_plots.py). */
#include <stdio.h>
#include <stdlib.h>
#define MAXF 16
static int find(const int f[], int n, int page) { for (int i = 0; i < n; i++) if (f[i] == page) return i; return -1; }
static int fifo(const int ref[], int len, int frames)
{ int f[MAXF], used = 0, next = 0, faults = 0;
  for (int i = 0; i < len; i++) { if (find(f, used, ref[i]) >= 0) continue; faults++; if (used < frames) f[used++] = ref[i]; else { f[next] = ref[i]; next = (next + 1) % frames; } }
  return faults; }
static int lru(const int ref[], int len, int frames)
{ int f[MAXF], last[MAXF], used = 0, faults = 0;
  for (int i = 0; i < len; i++) { int k = find(f, used, ref[i]); if (k >= 0) { last[k] = i; continue; } faults++;
    if (used < frames) { f[used] = ref[i]; last[used++] = i; } else { int v = 0; for (int j = 1; j < frames; j++) if (last[j] < last[v]) v = j; f[v] = ref[i]; last[v] = i; } }
  return faults; }
static int optimal(const int ref[], int len, int frames)
{ int f[MAXF], used = 0, faults = 0;
  for (int i = 0; i < len; i++) { if (find(f, used, ref[i]) >= 0) continue; faults++; if (used < frames) { f[used++] = ref[i]; continue; }
    int victim = 0, farthest = -1; for (int j = 0; j < frames; j++) { int nx = len; for (int t = i + 1; t < len; t++) if (ref[t] == f[j]) { nx = t; break; } if (nx > farthest) { farthest = nx; victim = j; } }
    f[victim] = ref[i]; }
  return faults; }
int main(void)
{
    int a[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    static int b[2000]; srand(3); int base = 0;
    for (int i = 0; i < 2000; i++) { if (i % 100 == 0) base = rand() % 40; b[i] = (rand() % 100 < 90) ? base + rand() % 6 : rand() % 50; }
    FILE *o = fopen("faults_vs_frames.csv", "w"); fprintf(o, "string,frames,fifo,lru,optimal\n");
    printf("%-9s %6s %6s %6s %8s\n", "string", "frames", "FIFO", "LRU", "Optimal");
    for (int fr = 1; fr <= 8; fr++) {
        int x = fifo(a, 20, fr), y = lru(a, 20, fr), z = optimal(a, 20, fr);
        fprintf(o, "A,%d,%d,%d,%d\n", fr, x, y, z); printf("%-9s %6d %6d %6d %8d\n", "A (20)", fr, x, y, z);
    }
    for (int fr = 1; fr <= 16; fr++) {
        int x = fifo(b, 2000, fr), y = lru(b, 2000, fr), z = optimal(b, 2000, fr);
        fprintf(o, "B,%d,%d,%d,%d\n", fr, x, y, z); if (fr <= 8 || fr == 16) printf("%-9s %6d %6d %6d %8d\n", "B (2000)", fr, x, y, z);
    }
    fclose(o); return 0;
}
