/* Q4 - Growth investigation: time to move 1 MB, 10 MB and 100 MB between two processes
 * Name: Kanishk C   Reg No: 192511278
 *
 * Methods:
 *   pipe : producer write()s 64 KB chunks, consumer read()s them (data is copied
 *          user -> kernel pipe buffer -> user, i.e. twice)
 *   shm  : producer fills slots of a shared ring buffer directly, consumer reads
 *          them in place (no copy through the kernel), 2 semaphores for sync
 *   mq   : POSIX message queue, 8 KB messages (copied twice as well)
 * Both sides touch every byte (the producer generates it, the consumer adds it
 * to a checksum), so the comparison is fair. The checksum is compared at the end.
 *
 * Build: gcc -std=gnu11 -Wall -Wextra -O2 -pthread -o q4_bench q4_bench.c -lrt
 * Run  : ./q4_bench pipe|shm|mq <MB>
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>

#define CHUNK (64 * 1024)
#define SLOTS 16
#define MQ_MSG 8192

struct shared {
    sem_t empty, full;
    size_t len[SLOTS];
    unsigned char data[SLOTS][CHUNK];
};

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void fill(unsigned char *p, size_t n, size_t offset)
{
    for (size_t i = 0; i < n; i++) p[i] = (unsigned char)((offset + i) * 31 + 7);
}

static uint64_t sum(const unsigned char *p, size_t n)
{
    uint64_t s = 0;
    for (size_t i = 0; i < n; i++) s += p[i];
    return s;
}

int main(int argc, char **argv)
{
    if (argc != 3) { fprintf(stderr, "usage: %s pipe|shm|mq MB\n", argv[0]); return 1; }
    size_t total = (size_t)atoi(argv[2]) * 1024 * 1024;
    int is_pipe = !strcmp(argv[1], "pipe"), is_shm = !strcmp(argv[1], "shm");

    /* one shared 8-byte cell in which the consumer reports its checksum */
    uint64_t *consumer_sum = mmap(NULL, sizeof *consumer_sum, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int fd[2];
    struct shared *sh = NULL;
    mqd_t mq = (mqd_t)-1;
    if (is_pipe) {
        if (pipe(fd) == -1) { perror("pipe"); return 1; }
    } else if (is_shm) {
        sh = mmap(NULL, sizeof *sh, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (sh == MAP_FAILED) { perror("mmap"); return 1; }
        sem_init(&sh->empty, 1, SLOTS);
        sem_init(&sh->full, 1, 0);
    } else {
        struct mq_attr at = {0};
        at.mq_maxmsg = 10; at.mq_msgsize = MQ_MSG;
        mq_unlink("/q4bench");
        mq = mq_open("/q4bench", O_CREAT | O_RDWR, 0600, &at);
        if (mq == (mqd_t)-1) { perror("mq_open"); return 1; }
    }

    double t0 = now();
    pid_t pid = fork();
    if (pid == 0) {                                   /* ---------- consumer ---------- */
        uint64_t checksum = 0;
        size_t got = 0;
        if (is_pipe) {
            close(fd[1]);
            unsigned char *buf = malloc(CHUNK);
            ssize_t r;
            while ((r = read(fd[0], buf, CHUNK)) > 0) { checksum += sum(buf, (size_t)r); got += (size_t)r; }
            free(buf);
        } else if (is_shm) {
            int out = 0;
            while (got < total) {
                sem_wait(&sh->full);
                checksum += sum(sh->data[out], sh->len[out]);
                got += sh->len[out];
                out = (out + 1) % SLOTS;
                sem_post(&sh->empty);
            }
        } else {
            unsigned char *buf = malloc(MQ_MSG);
            while (got < total) {
                ssize_t r = mq_receive(mq, (char *)buf, MQ_MSG, NULL);
                if (r < 0) { perror("mq_receive"); break; }
                checksum += sum(buf, (size_t)r); got += (size_t)r;
            }
            free(buf);
        }
        *consumer_sum = checksum;
        _exit(got == total ? 0 : 1);
    }

    /* ---------- producer (parent) ---------- */
    uint64_t checksum = 0;
    if (is_pipe) {
        close(fd[0]);
        unsigned char *buf = malloc(CHUNK);
        for (size_t off = 0; off < total; off += CHUNK) {
            fill(buf, CHUNK, off); checksum += sum(buf, CHUNK);
            if (write(fd[1], buf, CHUNK) != CHUNK) { perror("write"); break; }
        }
        close(fd[1]);
        free(buf);
    } else if (is_shm) {
        int in = 0;
        for (size_t off = 0; off < total; off += CHUNK) {
            sem_wait(&sh->empty);
            fill(sh->data[in], CHUNK, off); checksum += sum(sh->data[in], CHUNK);
            sh->len[in] = CHUNK;
            in = (in + 1) % SLOTS;
            sem_post(&sh->full);
        }
    } else {
        unsigned char *buf = malloc(MQ_MSG);
        for (size_t off = 0; off < total; off += MQ_MSG) {
            fill(buf, MQ_MSG, off); checksum += sum(buf, MQ_MSG);
            if (mq_send(mq, (char *)buf, MQ_MSG, 0) < 0) { perror("mq_send"); break; }
        }
        free(buf);
    }
    int status;
    waitpid(pid, &status, 0);
    double t1 = now();
    printf("%-5s %4s MB : %8.2f ms  (%7.1f MB/s)   data check: %s\n", argv[1], argv[2],
           (t1 - t0) * 1000.0, atof(argv[2]) / (t1 - t0),
           (WIFEXITED(status) && WEXITSTATUS(status) == 0 && *consumer_sum == checksum) ? "OK" : "MISMATCH");
    if (mq != (mqd_t)-1) { mq_close(mq); mq_unlink("/q4bench"); }
    return 0;
}
