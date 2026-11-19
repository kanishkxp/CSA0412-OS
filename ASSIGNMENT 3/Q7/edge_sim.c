/* edge_sim.c - page-replacement simulation of the smart-factory edge device (2 GB RAM, no swap).
 * Frames: 2 GB = 524,288 pages of 4 KB; 256 MB are kept by the OS -> 458,752 frames for the tasks.
 * One global LRU list is used for all pages that the kernel is ALLOWED to reclaim (like Linux page reclaim):
 *   - file-backed pages (program code, memory-mapped model weights, tables) can be dropped and read again;
 *   - anonymous pages (stack, heap, buffers) cannot be reclaimed because there is no swap device;
 *   - pages locked with mlock() cannot be reclaimed at all.
 * Tasks:  RT   = real-time control task, 100 MB, 4 references per tick
 *         ML   = machine-learning task, 1.5 GB = 1.2 GB mapped weights (file-backed) + 300 MB activations (anonymous),
 *                100 references per tick, scanning the weights in a loop (one inference = one pass over all the weights)
 *         LOG  = logging task, 64 MB anonymous ring buffer, 2 sequential references per tick
 *         NEW  = optional 4th task, 500 MB anonymous, 50 random references per tick
 * The first 10,000 ticks are warm-up, the next 40,000 ticks are measured.
 * Usage: ./edge_sim            (prints all scenarios)                                        */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MB2P(mb) ((int)((mb) * 256))            /* MB -> 4 KB pages */
#define FRAMES 458752
#define MAXPAGES 800000
#define WARM 10000
#define MEASURE 40000

struct task { const char *name; int pages; int evictable; int locked; int refs; int seq; int base; int cursor; long nref, nfault; };

static int nxt[MAXPAGES], prv[MAXPAGES]; static char resident[MAXPAGES], evictable[MAXPAGES], locked[MAXPAGES];
static int head, tail, used_frames; static int oom;
static unsigned long rng = 88172645463325252ull;
static unsigned long rnd(void) { rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17; return rng; }

static void unlink_page(int p) { if (prv[p] >= 0) nxt[prv[p]] = nxt[p]; else head = nxt[p]; if (nxt[p] >= 0) prv[nxt[p]] = prv[p]; else tail = prv[p]; }
static void push_front(int p) { prv[p] = -1; nxt[p] = head; if (head >= 0) prv[head] = p; head = p; if (tail < 0) tail = p; }

/* returns 1 if the reference caused a page fault */
static int reference(int p)
{
    if (resident[p]) { if (evictable[p] && !locked[p]) { unlink_page(p); push_front(p); } return 0; }
    if (used_frames >= FRAMES) {                                  /* memory full: need a victim */
        if (tail < 0) { oom = 1; return 1; }                      /* nothing can be reclaimed -> OOM killer */
        int v = tail; unlink_page(v); resident[v] = 0; used_frames--;
    }
    resident[p] = 1; used_frames++;
    if (evictable[p] && !locked[p]) push_front(p);
    return 1;
}

static void run(const char *title, struct task *t, int n, int rt_locked)
{
    memset(resident, 0, sizeof resident); head = tail = -1; used_frames = 0; oom = 0; rng = 88172645463325252ull;
    int base = 0, total = 0;
    for (int i = 0; i < n; i++) {
        t[i].base = base; t[i].cursor = 0; t[i].nref = t[i].nfault = 0;
        for (int k = 0; k < t[i].pages; k++) { evictable[base + k] = (char)t[i].evictable; locked[base + k] = (char)t[i].locked; }
        base += t[i].pages; total += t[i].pages;
    }
    int nonevict = 0; for (int i = 0; i < n; i++) if (!t[i].evictable || t[i].locked) nonevict += t[i].pages;
    printf("\n--- %s ---\n", title);
    printf("pages needed in total: %d (%.0f MB), frames available: %d (%.0f MB), pages that can never be reclaimed: %d (%.0f MB)\n",
           total, total / 256.0, FRAMES, FRAMES / 256.0, nonevict, nonevict / 256.0);
    if (nonevict > FRAMES) { printf("RESULT: the pages that cannot be reclaimed alone are larger than RAM -> the OOM killer would have to kill a task.\n"); return; }
    /* start-up: anonymous buffers are allocated and initialised, and locked pages are brought in (mlockall + touching).
     * These first touches are not counted as faults of the measured run. */
    for (int i = 0; i < n; i++) if (!t[i].evictable || (rt_locked && t[i].locked)) for (int k = 0; k < t[i].pages; k++) reference(t[i].base + k);
    for (int tick = 0; tick < WARM + MEASURE && !oom; tick++) {
        for (int i = 0; i < n; i++)
            for (int r = 0; r < t[i].refs; r++) {
                int off = t[i].seq ? t[i].cursor++ % t[i].pages : (int)(rnd() % (unsigned long)t[i].pages);
                /* ML: the first MB2P(1200) pages are the weights (file-backed), the rest the activations; sequential scan covers all */
                int f = reference(t[i].base + off);
                if (tick >= WARM) { t[i].nref++; t[i].nfault += f; }
            }
    }
    if (oom) { puts("RESULT: out of memory during the run."); return; }
    printf("%-6s %9s %12s %12s %12s\n", "task", "pages", "references", "page faults", "fault rate");
    for (int i = 0; i < n; i++) printf("%-6s %9d %12ld %12ld %11.4f%%\n", t[i].name, t[i].pages, t[i].nref, t[i].nfault, t[i].nref ? 100.0 * (double)t[i].nfault / (double)t[i].nref : 0.0);
}

int main(void)
{
    /* The ML task is split in two entries so that weights are evictable (file-backed) and activations are not. */
    struct task A[] = {
        {"RT",  MB2P(100), 1, 1, 4, 0, 0, 0, 0, 0},
        {"ML-w", MB2P(1200), 1, 0, 100, 1, 0, 0, 0, 0},
        {"ML-a", MB2P(300), 0, 0, 20, 0, 0, 0, 0, 0},
        {"LOG", MB2P(64), 0, 0, 2, 1, 0, 0, 0, 0},
    };
    run("Scenario 1: three tasks, real-time task LOCKED", A, 4, 1);

    struct task B[] = {
        {"RT",  MB2P(100), 1, 0, 4, 0, 0, 0, 0, 0},
        {"ML-w", MB2P(1200), 1, 0, 100, 1, 0, 0, 0, 0},
        {"ML-a", MB2P(300), 0, 0, 20, 0, 0, 0, 0, 0},
        {"LOG", MB2P(64), 0, 0, 2, 1, 0, 0, 0, 0},
        {"NEW", MB2P(500), 0, 0, 50, 0, 0, 0, 0, 0},
    };
    run("Scenario 2: FOUR tasks (500 MB added), real-time task NOT locked (data is file-backed, so reclaimable)", B, 5, 0);

    struct task C[] = {
        {"RT",  MB2P(100), 1, 1, 4, 0, 0, 0, 0, 0},
        {"ML-w", MB2P(1200), 1, 0, 100, 1, 0, 0, 0, 0},
        {"ML-a", MB2P(300), 0, 0, 20, 0, 0, 0, 0, 0},
        {"LOG", MB2P(64), 0, 0, 2, 1, 0, 0, 0, 0},
        {"NEW", MB2P(500), 0, 0, 50, 0, 0, 0, 0, 0},
    };
    run("Scenario 3: FOUR tasks, real-time task LOCKED", C, 5, 1);

    struct task D[] = {
        {"RT",  MB2P(100), 1, 1, 4, 0, 0, 0, 0, 0},
        {"ML-w", MB2P(700), 1, 0, 100, 1, 0, 0, 0, 0},
        {"ML-a", MB2P(300), 0, 0, 20, 0, 0, 0, 0, 0},
        {"LOG", MB2P(64), 0, 0, 2, 1, 0, 0, 0, 0},
        {"NEW", MB2P(500), 0, 0, 50, 0, 0, 0, 0, 0},
    };
    run("Scenario 4: FOUR tasks, RT LOCKED, ML model reduced to 1.0 GB (700 MB weights + 300 MB activations)", D, 5, 1);

    struct task E[] = {
        {"RT",  MB2P(100), 1, 1, 4, 0, 0, 0, 0, 0},
        {"ML-w", MB2P(1200), 1, 0, 100, 1, 0, 0, 0, 0},
        {"ML-a", MB2P(300), 0, 0, 20, 0, 0, 0, 0, 0},
        {"LOG", MB2P(64), 0, 0, 2, 1, 0, 0, 0, 0},
        {"NEW", MB2P(1100), 0, 0, 50, 0, 0, 0, 0, 0},
    };
    run("Scenario 5 (limit test): a 1.1 GB fourth task instead of 500 MB, RT LOCKED", E, 5, 1);
    return 0;
}
