#define _POSIX_C_SOURCE 199309L
/* Compares looking up one file name in
   (a) one flat directory with 12,000 entries (linear search, O(n)) and
   (b) the proposed tree /home/<dept>/<regno>/ (small directories, O(depth * small n)). */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define STUDENTS 12000
#define DEPTS 12
#define PER_DEPT (STUDENTS / DEPTS)
#define LOOKUPS 20000

static char flat[STUDENTS][24];
static char dept_names[DEPTS][8];
static char stud_names[DEPTS][PER_DEPT][12];

static double now_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

int main(void)
{
    for (int i = 0; i < STUDENTS; i++) sprintf(flat[i], "file_%05d.doc", i);
    for (int d = 0; d < DEPTS; d++) {
        sprintf(dept_names[d], "dept%02d", d);
        for (int s = 0; s < PER_DEPT; s++) sprintf(stud_names[d][s], "reg%05d", d * PER_DEPT + s);
    }

    static int target[LOOKUPS];
    srand(7);
    for (int i = 0; i < LOOKUPS; i++) target[i] = rand() % STUDENTS;

    long sink = 0;
    char name[32], dname[32], sname[32];

    /* (a) flat directory: compare names one by one */
    double t0 = now_ms();
    for (int i = 0; i < LOOKUPS; i++) {
        sprintf(name, "file_%05d.doc", target[i]);
        for (int j = 0; j < STUDENTS; j++)
            if (strcmp(flat[j], name) == 0) { sink += j; break; }
    }
    double flat_ms = now_ms() - t0;

    /* (b) tree: find the department (12 names), then the student (1000 names) */
    t0 = now_ms();
    for (int i = 0; i < LOOKUPS; i++) {
        int s = target[i];
        sprintf(dname, "dept%02d", s / PER_DEPT);
        sprintf(sname, "reg%05d", s);
        for (int d = 0; d < DEPTS; d++)
            if (strcmp(dept_names[d], dname) == 0) {
                for (int k = 0; k < PER_DEPT; k++)
                    if (strcmp(stud_names[d][k], sname) == 0) { sink += k; break; }
                break;
            }
    }
    double tree_ms = now_ms() - t0;

    printf("lookups: %d\n", LOOKUPS);
    printf("flat directory (12,000 entries): %.2f ms total, %.3f microseconds per lookup\n", flat_ms, flat_ms * 1000.0 / LOOKUPS);
    printf("tree (12 + 1000 entries)       : %.2f ms total, %.3f microseconds per lookup\n", tree_ms, tree_ms * 1000.0 / LOOKUPS);
    printf("(checksum %ld)\n", sink);
    return 0;
}
