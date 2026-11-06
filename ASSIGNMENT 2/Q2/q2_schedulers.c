/* Q2 - CPU Scheduling: FCFS, SJF, SRTF, Priority, Round Robin and MLFQ
 * Name: Kanishk C   Reg No: 192511278
 *
 * Every algorithm is simulated one time unit at a time and writes into a
 * timeline tl[t] = index of the job that ran during [t, t+1) (-1 = CPU idle).
 * All results (Gantt chart, completion, turnaround, waiting and response time)
 * are then worked out from that timeline, so every algorithm is measured in
 * exactly the same way.
 *
 * Build: gcc -std=c11 -Wall -Wextra -o q2_schedulers q2_schedulers.c
 */
#include <stdio.h>
#include <string.h>

#define MAXJ 64
#define MAXT 6000

enum { FCFS, SJF, SRTF, PRIO, PRIO_P, PRIO_AGE, RR, MLFQ };
static const char *policy_name[] = {"FCFS", "SJF (non-preemptive)", "SRTF (preemptive SJF)",
    "Priority (non-preemptive)", "Priority (preemptive)", "Priority + ageing", "Round Robin", "MLFQ"};

typedef struct { char name[8]; int arr, burst, prio; int rem, fin, first; } Job;

typedef struct {
    int n;
    Job j[MAXJ];
    int tl[MAXT];
    int end;
    int switches;
    double avg_wt, avg_tat, avg_rt;
} Sim;

/* ---------- helpers ---------- */
static int better(Sim *s, int a, int b, int policy, int t, int age)
{
    /* returns 1 if job a should be preferred to job b */
    if (b < 0) return 1;
    Job *x = &s->j[a], *y = &s->j[b];
    double kx = 0, ky = 0;
    switch (policy) {
    case FCFS:     kx = x->arr;   ky = y->arr;   break;
    case SJF:      kx = x->burst; ky = y->burst; break;
    case SRTF:     kx = x->rem;   ky = y->rem;   break;
    case PRIO:
    case PRIO_P:   kx = x->prio;  ky = y->prio;  break;
    case PRIO_AGE: /* every 'age' time units of waiting improves the priority by 1 */
        kx = x->prio - (double)(t - x->arr - (x->burst - x->rem)) / age;
        ky = y->prio - (double)(t - y->arr - (y->burst - y->rem)) / age;
        break;
    }
    if (kx != ky) return kx < ky;
    if (x->arr != y->arr) return x->arr < y->arr;
    return a < b;
}

static void finish_metrics(Sim *s)
{
    for (int i = 0; i < s->n; i++) { s->j[i].fin = 0; s->j[i].first = -1; }
    for (int t = 0; t < s->end; t++) {
        int p = s->tl[t];
        if (p < 0) continue;
        if (s->j[p].first < 0) s->j[p].first = t;
        s->j[p].fin = t + 1;
    }
    s->avg_wt = s->avg_tat = s->avg_rt = 0;
    for (int i = 0; i < s->n; i++) {
        int tat = s->j[i].fin - s->j[i].arr;
        s->avg_tat += tat;
        s->avg_wt  += tat - s->j[i].burst;
        s->avg_rt  += s->j[i].first - s->j[i].arr;
    }
    s->avg_tat /= s->n; s->avg_wt /= s->n; s->avg_rt /= s->n;
    s->switches = 0;
    int prev = -1;
    for (int t = 0; t < s->end; t++) {
        int p = s->tl[t];
        if (p < 0) continue;
        if (prev >= 0 && p != prev) s->switches++;
        prev = p;
    }
}

/* ---------- simple policies (one decision per time unit) ---------- */
static void run_simple(Sim *s, int policy, int age)
{
    int t = 0, done = 0, cur = -1;
    for (int i = 0; i < s->n; i++) s->j[i].rem = s->j[i].burst;
    while (done < s->n) {
        int pick = -1;
        int nonpre = (policy == FCFS || policy == SJF || policy == PRIO);
        if (nonpre && cur >= 0) pick = cur;
        else
            for (int i = 0; i < s->n; i++)
                if (s->j[i].arr <= t && s->j[i].rem > 0 && better(s, i, pick, policy, t, age)) pick = i;
        if (pick < 0) { s->tl[t++] = -1; continue; }
        s->tl[t++] = pick;
        cur = pick;
        if (--s->j[pick].rem == 0) { done++; cur = -1; }
    }
    s->end = t;
    finish_metrics(s);
}

/* ---------- Round Robin (same rules as the given program) ---------- */
static void run_rr(Sim *s, int quantum)
{
    int queue[8192], head = 0, tail = 0, added[MAXJ] = {0}, t = 0, done = 0;
    for (int i = 0; i < s->n; i++) s->j[i].rem = s->j[i].burst;
    while (done < s->n) {
        for (int i = 0; i < s->n; i++)
            if (!added[i] && s->j[i].arr <= t) { queue[tail++] = i; added[i] = 1; }
        if (head == tail) { s->tl[t++] = -1; continue; }          /* CPU idle */
        int p = queue[head++];
        int run = s->j[p].rem < quantum ? s->j[p].rem : quantum;
        for (int k = 0; k < run; k++) s->tl[t++] = p;
        s->j[p].rem -= run;
        for (int i = 0; i < s->n; i++)
            if (!added[i] && s->j[i].arr <= t) { queue[tail++] = i; added[i] = 1; }
        if (s->j[p].rem > 0) queue[tail++] = p; else done++;
    }
    s->end = t;
    finish_metrics(s);
}

/* ---------- Multilevel feedback queue: 3 levels, quanta q0, q1, then FCFS ---------- */
static void run_mlfq(Sim *s, int q0, int q1, int boost)
{
    static int qa[3][8192];
    int qh[3] = {0, 0, 0}, qt[3] = {0, 0, 0};
    int lvl[MAXJ] = {0}, added[MAXJ] = {0};
    int quantum[3] = {q0, q1, 1 << 30};
    int t = 0, done = 0, cur = -1, used = 0;
    for (int i = 0; i < s->n; i++) s->j[i].rem = s->j[i].burst;
    while (done < s->n) {
        for (int i = 0; i < s->n; i++)
            if (!added[i] && s->j[i].arr <= t) { qa[0][qt[0]++] = i; added[i] = 1; lvl[i] = 0; }
        if (boost > 0 && t > 0 && t % boost == 0) {            /* ageing: lift everybody to the top queue */
            for (int l = 1; l < 3; l++)
                while (qh[l] < qt[l]) { int j = qa[l][qh[l]++]; lvl[j] = 0; qa[0][qt[0]++] = j; }
            if (cur >= 0) lvl[cur] = 0;
        }
        if (cur >= 0)                                          /* a higher queue got work -> preempt */
            for (int l = 0; l < lvl[cur]; l++)
                if (qh[l] < qt[l]) { qa[lvl[cur]][qt[lvl[cur]]++] = cur; cur = -1; break; }
        if (cur < 0) {
            for (int l = 0; l < 3; l++)
                if (qh[l] < qt[l]) { cur = qa[l][qh[l]++]; used = 0; break; }
            if (cur < 0) { s->tl[t++] = -1; continue; }
        }
        s->tl[t++] = cur;
        used++;
        if (--s->j[cur].rem == 0) { done++; cur = -1; }
        else if (used >= quantum[lvl[cur]]) {
            if (lvl[cur] < 2) lvl[cur]++;
            qa[lvl[cur]][qt[lvl[cur]]++] = cur;
            cur = -1;
        }
    }
    s->end = t;
    finish_metrics(s);
}

/* ---------- printing ---------- */
static void print_gantt(Sim *s, FILE *csv, const char *tag, const char *algo)
{
    printf("Gantt: ");
    int start = 0;
    for (int t = 1; t <= s->end; t++) {
        if (t == s->end || s->tl[t] != s->tl[start]) {
            if (s->tl[start] >= 0) {
                printf("%s(%d-%d) ", s->j[s->tl[start]].name, start, t);
                if (csv) fprintf(csv, "%s,%s,%s,%d,%d\n", tag, algo, s->j[s->tl[start]].name, start, t);
            } else printf("idle(%d-%d) ", start, t);
            start = t;
        }
    }
    printf("\n");
}

static void print_table(Sim *s)
{
    printf("%-8s %4s %5s %4s %5s %5s %5s %5s\n", "Process", "Arr", "Burst", "Pri", "CT", "TAT", "WT", "RT");
    for (int i = 0; i < s->n; i++) {
        Job *j = &s->j[i];
        int tat = j->fin - j->arr;
        printf("%-8s %4d %5d %4d %5d %5d %5d %5d\n", j->name, j->arr, j->burst, j->prio,
               j->fin, tat, tat - j->burst, j->first - j->arr);
    }
    printf("Average: TAT = %.2f, WT = %.2f, RT = %.2f   (context switches = %d)\n\n",
           s->avg_tat, s->avg_wt, s->avg_rt, s->switches);
}

static void set_job(Sim *s, int i, const char *name, int arr, int burst, int prio)
{
    strcpy(s->j[i].name, name);
    s->j[i].arr = arr; s->j[i].burst = burst; s->j[i].prio = prio;
}

/* average waiting time of the jobs whose burst is <= limit (short jobs) or > limit (long jobs) */
static double group_wt(Sim *s, int shorts, int limit, double *max_wait)
{
    double sum = 0; int c = 0; *max_wait = 0;
    for (int i = 0; i < s->n; i++) {
        int is_short = s->j[i].burst <= limit;
        if (is_short != shorts) continue;
        double w = s->j[i].fin - s->j[i].arr - s->j[i].burst;
        sum += w; c++;
        if (w > *max_wait) *max_wait = w;
    }
    return c ? sum / c : 0;
}

static double group_rt(Sim *s, int shorts, int limit)
{
    double sum = 0; int c = 0;
    for (int i = 0; i < s->n; i++) {
        if ((s->j[i].burst <= limit) != shorts) continue;
        sum += s->j[i].first - s->j[i].arr; c++;
    }
    return c ? sum / c : 0;
}

/* how many seconds of CPU job 0 (the long job) received before time 'limit' */
static int long_cpu(Sim *s, int limit)
{
    int c = 0;
    for (int t = 0; t < limit && t < s->end; t++) if (s->tl[t] == 0) c++;
    return c;
}

int main(void)
{
    static Sim s;
    FILE *csv = fopen("q2_gantt_data.csv", "w");
    FILE *qcsv = fopen("q2_quantum_data.csv", "w");
    fprintf(csv, "dataset,algorithm,process,start,end\n");
    fprintf(qcsv, "quantum,avg_wt,avg_tat,avg_rt,context_switches\n");

    /* ===== Part 1: the assignment's job set (same as the Complete-the-Code program) ===== */
    memset(&s, 0, sizeof s);
    s.n = 4;
    set_job(&s, 0, "P1", 0, 8, 2);      /* priority: smaller number = more important */
    set_job(&s, 1, "P2", 1, 4, 3);
    set_job(&s, 2, "P3", 2, 9, 1);
    set_job(&s, 3, "P4", 3, 5, 4);

    printf("################ PART 1: jobs P1..P4 ################\n\n");
    int pol[] = {FCFS, SJF, SRTF, PRIO, RR};
    for (int k = 0; k < 5; k++) {
        if (pol[k] == RR) run_rr(&s, 3); else run_simple(&s, pol[k], 10);
        printf("=== %s%s ===\n", policy_name[pol[k]], pol[k] == RR ? " (quantum = 3)" : "");
        print_gantt(&s, csv, "set1", policy_name[pol[k]]);
        print_table(&s);
    }

    /* ===== Part 2: effect of the time quantum ===== */
    printf("################ PART 2: Round Robin, effect of the quantum ################\n");
    printf("%8s %8s %8s %8s %10s\n", "quantum", "avg WT", "avg TAT", "avg RT", "switches");
    int qs[] = {1, 3, 5, 20};
    for (int k = 0; k < 4; k++) {
        run_rr(&s, qs[k]);
        printf("%8d %8.2f %8.2f %8.2f %10d\n", qs[k], s.avg_wt, s.avg_tat, s.avg_rt, s.switches);
        fprintf(qcsv, "%d,%.2f,%.2f,%.2f,%d\n", qs[k], s.avg_wt, s.avg_tat, s.avg_rt, s.switches);
    }

    /* ===== Part 3: video service: 2 long films (1200 s) and 8 thumbnails (2 s) ===== */
    memset(&s, 0, sizeof s);
    s.n = 10;
    set_job(&s, 0, "L1", 0, 1200, 2);
    set_job(&s, 1, "L2", 2, 1200, 2);
    for (int i = 0; i < 8; i++) {
        char nm[8]; snprintf(nm, sizeof nm, "S%d", i + 1);
        set_job(&s, 2 + i, nm, 3 + i * 5, 2, 1);        /* a thumbnail arrives every 5 s */
    }
    printf("\n################ PART 3: video service (long = 1200 s, short = 2 s) ################\n");
    printf("%-28s %8s %10s %10s %10s %10s %8s\n", "policy", "avg WT", "short WT", "long WT", "short RT", "max wait", "switches");
    struct { int pol; int q; const char *label; } runs[] = {
        {FCFS, 0, "FCFS"}, {SJF, 0, "SJF (non-preemptive)"}, {SRTF, 0, "SRTF"},
        {PRIO, 0, "Priority (non-preemptive)"}, {RR, 10, "Round Robin q=10"}, {RR, 100, "Round Robin q=100"},
        {MLFQ, 0, "MLFQ (2, 10, FCFS)"}, {MLFQ, 1, "MLFQ + boost every 300 s"} };
    for (int k = 0; k < 8; k++) {
        double mw, mw2;
        if (runs[k].pol == RR) run_rr(&s, runs[k].q);
        else if (runs[k].pol == MLFQ) run_mlfq(&s, 2, 10, runs[k].q ? 300 : 0);
        else run_simple(&s, runs[k].pol, 10);
        double sw = group_wt(&s, 1, 2, &mw), lw = group_wt(&s, 0, 2, &mw2);
        printf("%-28s %8.1f %10.1f %10.1f %10.1f %10.0f %8d\n", runs[k].label, s.avg_wt, sw, lw,
               group_rt(&s, 1, 2), mw > mw2 ? mw : mw2, s.switches);
        if (runs[k].pol == MLFQ && !runs[k].q) print_gantt(&s, NULL, "", "");
    }

    /* ===== Part 4: starvation demo: one 20 s job and a continuous stream of 2 s jobs ===== */
    memset(&s, 0, sizeof s);
    s.n = 31;
    set_job(&s, 0, "LONG", 0, 20, 2);
    for (int i = 0; i < 30; i++) {
        char nm[8]; snprintf(nm, sizeof nm, "S%d", i + 1);
        set_job(&s, 1 + i, nm, 1 + 2 * i, 2, 1);
    }
    printf("\n################ PART 4: starvation of the long job ################\n");
    printf("%-34s %14s %18s %16s\n", "policy", "LONG waits (s)", "LONG finishes at", "CPU by t=40 (s)");
    struct { int pol; const char *label; } st[] = {
        {SRTF, "SRTF"}, {PRIO_P, "Priority (preemptive)"}, {PRIO_AGE, "Priority + ageing (1 per 10 s)"} };
    for (int k = 0; k < 3; k++) {
        run_simple(&s, st[k].pol, 10);
        printf("%-34s %14d %18d %16d\n", st[k].label, s.j[0].fin - s.j[0].arr - s.j[0].burst, s.j[0].fin, long_cpu(&s, 40));
    }
    run_mlfq(&s, 2, 10, 0);
    printf("%-34s %14d %18d %16d\n", "MLFQ (no boost)", s.j[0].fin - s.j[0].arr - s.j[0].burst, s.j[0].fin, long_cpu(&s, 40));
    run_mlfq(&s, 2, 10, 30);
    printf("%-34s %14d %18d %16d\n", "MLFQ + boost every 30 s", s.j[0].fin - s.j[0].arr - s.j[0].burst, s.j[0].fin, long_cpu(&s, 40));

    fclose(csv); fclose(qcsv);
    return 0;
}
