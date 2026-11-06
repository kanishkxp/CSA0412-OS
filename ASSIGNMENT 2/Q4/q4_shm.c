/* Q4 - The same weather pipeline using shared memory + semaphores instead of a pipe
 * Name: Kanishk C   Reg No: 192511278
 *
 * An anonymous shared mapping is created BEFORE fork(), so parent and child see
 * the same physical memory. A small ring buffer holds the readings. Two
 * unnamed semaphores (placed inside the shared memory) make sure the writer
 * never overwrites a reading that was not read yet (empty) and the reader never
 * reads a slot that is not filled yet (full). A pipe needs none of this because
 * the kernel does the waiting for us.
 *
 * Build: gcc -std=c11 -Wall -Wextra -pthread -o q4_shm q4_shm.c
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define SLOTS 4
#define END_MARK (-1)          /* readings are never negative, so -1 means "no more data" */

struct shared {
    sem_t empty, full;
    int buf[SLOTS];
};

int main(void)
{
    struct shared *sh = mmap(NULL, sizeof *sh, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sh == MAP_FAILED) { perror("mmap"); return 1; }
    sem_init(&sh->empty, 1, SLOTS);      /* second argument 1 = shared between processes */
    sem_init(&sh->full, 1, 0);

    pid_t pid = fork();
    if (pid == 0) {                      /* analyser */
        int out = 0, count = 0, sum = 0, max = 0;
        for (;;) {
            sem_wait(&sh->full);
            int r = sh->buf[out];
            out = (out + 1) % SLOTS;
            sem_post(&sh->empty);
            if (r == END_MARK) break;
            count++; sum += r;
            if (count == 1 || r > max) max = r;
        }
        printf("analyser: %d readings, average %.1f, max %d\n", count, count ? (double)sum / count : 0.0, max);
        fflush(stdout);
        _exit(0);
    }

    int readings[] = {31, 33, 36, 34, 41, 38, END_MARK};
    int in = 0;
    for (int i = 0; i < 7; i++) {        /* sensor reader */
        sem_wait(&sh->empty);
        sh->buf[in] = readings[i];
        in = (in + 1) % SLOTS;
        sem_post(&sh->full);
    }
    waitpid(pid, NULL, 0);
    printf("reader: done\n");
    sem_destroy(&sh->empty);
    sem_destroy(&sh->full);
    munmap(sh, sizeof *sh);
    return 0;
}
