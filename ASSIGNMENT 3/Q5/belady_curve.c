/* belady_curve.c - FIFO and LRU faults against the number of frames for Belady's string 1 2 3 4 1 2 5 1 2 3 4 5.
 * Writes belady_curve.csv. FIFO goes UP when frames go from 3 to 4 (Belady's anomaly); LRU never does. */
#include <stdio.h>
#define MAXF 8
static int find(const int f[], int n, int p) { for (int i = 0; i < n; i++) if (f[i] == p) return i; return -1; }
static int fifo(const int ref[], int len, int frames)
{ int f[MAXF], used = 0, next = 0, faults = 0;
  for (int i = 0; i < len; i++) { if (find(f, used, ref[i]) >= 0) continue; faults++; if (used < frames) f[used++] = ref[i]; else { f[next] = ref[i]; next = (next + 1) % frames; } }
  return faults; }
static int lru(const int ref[], int len, int frames)
{ int f[MAXF], last[MAXF], used = 0, faults = 0;
  for (int i = 0; i < len; i++) { int k = find(f, used, ref[i]); if (k >= 0) { last[k] = i; continue; } faults++;
    if (used < frames) { f[used] = ref[i]; last[used++] = i; } else { int v = 0; for (int j = 1; j < frames; j++) if (last[j] < last[v]) v = j; f[v] = ref[i]; last[v] = i; } }
  return faults; }
int main(void)
{
    int b[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    FILE *o = fopen("belady_curve.csv", "w"); fprintf(o, "frames,fifo,lru\n");
    puts("frames  FIFO  LRU");
    for (int fr = 1; fr <= 7; fr++) { int x = fifo(b, 12, fr), y = lru(b, 12, fr); fprintf(o, "%d,%d,%d\n", fr, x, y); printf("%6d %5d %4d\n", fr, x, y); }
    fclose(o); return 0;
}
