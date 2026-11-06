/* Q1 - Processes and process states
 * Name: Kanishk C   Reg No: 192511278
 *
 * The parent creates 4 child processes with fork(). Each child sleeps for a
 * while (so it shows up as state S = waiting) and one child keeps the CPU busy
 * (state R = running/ready). While they are alive, the parent runs "ps" and
 * reads /proc/<pid>/status so we can see the process control block (PCB) data
 * that the kernel keeps for every process.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define CHILDREN 4

static void show_proc_status(pid_t pid)
{
    char path[64], line[256];
    snprintf(path, sizeof path, "/proc/%d/status", (int)pid);
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); return; }
    while (fgets(line, sizeof line, f)) {
        /* only print the fields that look like PCB entries */
        if (line[0] == 'N' && line[1] == 'a') fputs(line, stdout);          /* Name   */
        else if (line[0] == 'S' && line[1] == 't' && line[2] == 'a') fputs(line, stdout); /* State */
        else if (line[0] == 'P' && line[1] == 'i') fputs(line, stdout);     /* Pid    */
        else if (line[0] == 'P' && line[1] == 'P') fputs(line, stdout);     /* PPid   */
        else if (line[0] == 'V' && line[1] == 'm' && line[2] == 'R') fputs(line, stdout); /* VmRSS */
        else if (line[0] == 'V' && line[1] == 'm' && line[2] == 'S' && line[3] == 't') fputs(line, stdout); /* VmStk */
        else if (line[0] == 'v' || line[0] == 'n') fputs(line, stdout);     /* context switches */
    }
    fclose(f);
}

int main(void)
{
    pid_t kids[CHILDREN];
    printf("parent pid = %d\n", (int)getpid());

    for (int i = 0; i < CHILDREN; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return 1; }
        if (pid == 0) {
            /* child */
            if (i == 0) {
                /* child 0 burns CPU for 3 seconds -> RUNNING / READY */
                time_t start = time(NULL);
                volatile unsigned long x = 0;
                while (time(NULL) - start < 3) x++;
            } else {
                /* other children just wait (like waiting for I/O) */
                sleep(3);
            }
            _exit(i);
        }
        kids[i] = pid;
    }

    struct timespec half_sec = {0, 500000000L};
    nanosleep(&half_sec, NULL);   /* give the children time to start */

    printf("\n--- ps output while children are alive ---\n");
    char cmd[128];
    snprintf(cmd, sizeof cmd, "ps -o pid,ppid,stat,wchan:12,comm --ppid %d", (int)getpid());
    fflush(stdout);
    system(cmd);

    printf("\n--- /proc/%d/status (child 0, busy) ---\n", (int)kids[0]);
    show_proc_status(kids[0]);
    printf("\n--- /proc/%d/status (child 1, sleeping) ---\n", (int)kids[1]);
    show_proc_status(kids[1]);

    /* let the children finish but do NOT wait yet, so they become zombies */
    sleep(4);
    printf("\n--- ps output after children exited, before wait() (zombies = Z) ---\n");
    fflush(stdout);
    system(cmd);

    for (int i = 0; i < CHILDREN; i++) {
        int status;
        waitpid(kids[i], &status, 0);
        printf("child %d (pid %d) terminated with exit code %d\n",
               i, (int)kids[i], WEXITSTATUS(status));
    }
    return 0;
}
