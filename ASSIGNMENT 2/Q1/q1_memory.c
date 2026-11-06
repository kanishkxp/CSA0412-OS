/* Q1 - Memory used by N live sessions (processes vs threads)
 * Name: Kanishk C   Reg No: 192511278
 *
 * Every session allocates and touches 256 KB (its own working data) and then
 * stays alive for a few seconds. While they are all alive we compare the
 * system's MemAvailable (from /proc/meminfo) before and after.
 *
 * Usage: ./q1_memory <process|thread> <N>
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define WORK_BYTES (256 * 1024)

static long mem_available_kb(void)
{
    FILE *f = fopen("/proc/meminfo", "r");
    char line[128]; long v = -1;
    while (f && fgets(line, sizeof line, f))
        if (sscanf(line, "MemAvailable: %ld kB", &v) == 1) break;
    if (f) fclose(f);
    return v;
}

static void session(void)
{
    volatile char *buf = malloc(WORK_BYTES);
    for (size_t i = 0; i < WORK_BYTES; i += 4096) buf[i] = 1;   /* touch every page */
    sleep(3);                 /* session stays alive, like a student logged in */
}

static void *thread_main(void *a) { (void)a; session(); return NULL; }

int main(int argc, char **argv)
{
    if (argc != 3) { fprintf(stderr, "usage: %s process|thread N\n", argv[0]); return 1; }
    int n = atoi(argv[2]);
    long before = mem_available_kb();

    pthread_t *t = malloc(sizeof(pthread_t) * n);
    for (int i = 0; i < n; i++) {
        if (strcmp(argv[1], "process") == 0) {
            pid_t pid = fork();
            if (pid < 0) { perror("fork"); return 1; }
            if (pid == 0) { session(); _exit(0); }
        } else {
            if (pthread_create(&t[i], NULL, thread_main, NULL) != 0) { perror("pthread_create"); return 1; }
        }
    }
    sleep(1);                                  /* all sessions are alive now */
    long during = mem_available_kb();
    printf("%s x %d : memory used = %ld KB total, %.1f KB per session\n",
           argv[1], n, before - during, (double)(before - during) / n);

    if (strcmp(argv[1], "process") == 0) while (wait(NULL) > 0) ;
    else for (int i = 0; i < n; i++) pthread_join(t[i], NULL);
    free(t);
    return 0;
}
