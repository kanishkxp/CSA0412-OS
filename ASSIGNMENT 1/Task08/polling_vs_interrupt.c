#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <poll.h>
#include <stdint.h>
#include <sys/eventfd.h>

/* A "sensor" thread raises an abnormal reading at random times. The "monitor"
   notices it in one of three ways:
     busy  : polls in a tight loop (no sleeping)
     p10   : polls every 10 ms
     intr  : sleeps in poll() on an eventfd and is woken by the kernel
             (this is the closest user-space picture of an interrupt)
   We measure the delay between the reading and the moment it is noticed, and the CPU
   time used by the monitor thread.
   usage: ./polling_vs_interrupt busy|p10|intr                              */
#define EVENTS 40
static volatile int flag = 0;
static volatile double t_event = 0;
static int efd;
static int mode;                 /* 0 busy, 1 p10, 2 intr */

static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }

static void *sensor(void *a)
{
    (void)a;
    srand(7);
    for (int i = 0; i < EVENTS; i++) {
        usleep(20000 + rand() % 30000);          /* 20 to 50 ms between abnormal readings */
        t_event = now();
        flag = 1;
        if (mode == 2) { uint64_t one = 1; write(efd, &one, 8); }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2) return 1;
    mode = !strcmp(argv[1], "busy") ? 0 : !strcmp(argv[1], "p10") ? 1 : 2;
    efd = eventfd(0, 0);
    pthread_t s;
    struct timespec cpu0, cpu1;
    double maxd = 0, sum = 0;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu0);
    double wall0 = now();
    pthread_create(&s, NULL, sensor, NULL);
    for (int got = 0; got < EVENTS; got++) {
        if (mode == 0) { while (!flag) ; }
        else if (mode == 1) { while (!flag) usleep(10000); }
        else { struct pollfd p = { efd, POLLIN, 0 }; poll(&p, 1, -1); uint64_t v; read(efd, &v, 8); }
        double d = now() - t_event;
        flag = 0;
        sum += d; if (d > maxd) maxd = d;
    }
    double wall = now() - wall0;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu1);
    double cpu = (cpu1.tv_sec - cpu0.tv_sec) + (cpu1.tv_nsec - cpu0.tv_nsec) / 1e9;
    pthread_join(s, NULL);
    printf("%-5s average delay %8.1f us   worst delay %8.1f us   monitor CPU %.3f s of %.3f s (%.1f%%)\n",
           argv[1], sum / EVENTS * 1e6, maxd * 1e6, cpu, wall, cpu / wall * 100);
    return 0;
}
