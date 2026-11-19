/* segv_demo.c - trigger two kinds of segmentation fault and show what the kernel tells us.
 * A SIGSEGV handler prints the faulting address (si_addr) and the reason (si_code):
 *   SEGV_MAPERR = address is not mapped in any segment (VMA)
 *   SEGV_ACCERR = address is mapped but the permission does not allow the access */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <sys/mman.h>

static sigjmp_buf env;
static void handler(int sig, siginfo_t *si, void *ctx)
{
    (void)ctx;
    char buf[200];
    int n = snprintf(buf, sizeof buf, "  -> signal %d (SIGSEGV) caught, si_addr=%p, si_code=%s\n", sig, si->si_addr,
                     si->si_code == SEGV_MAPERR ? "SEGV_MAPERR (no mapping)" : si->si_code == SEGV_ACCERR ? "SEGV_ACCERR (permission)" : "other");
    ssize_t w = write(1, buf, (size_t)n); (void)w;
    siglongjmp(env, 1);
}
int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);            /* unbuffered, so output and handler text stay in order */
    struct sigaction sa; memset(&sa, 0, sizeof sa);
    sa.sa_sigaction = handler; sa.sa_flags = SA_SIGINFO; sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);

    if (argc > 1 && strcmp(argv[1], "crash") == 0) {            /* no handler: default action */
        signal(SIGSEGV, SIG_DFL);
        puts("about to write to address 0 with the default SIGSEGV action...");
        fflush(stdout);
        volatile int *p = NULL; *p = 42;
        return 0;
    }
    if (sigsetjmp(env, 1) == 0) { puts("Test 1: write to NULL (page 0 is not mapped)"); volatile int *p = NULL; *p = 1; }
    if (sigsetjmp(env, 1) == 0) {
        puts("Test 2: write into a read-only page (like writing into the code segment)");
        char *ro = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        printf("  read-only page mapped at %p\n", (void *)ro); ro[10] = 'x';
    }
    if (sigsetjmp(env, 1) == 0) {
        puts("Test 3: access just beyond the end of a mapping (like offset > limit)");
        char *m = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        munmap(m + 4096, 4096);  /* make sure the next page is free */
        printf("  mapping = [%p, %p), accessing %p\n", (void *)m, (void *)(m + 4096), (void *)(m + 4096)); m[4096] = 'y';
    }
    puts("program survived because the handler recovered; without a handler the kernel kills it.");
    return 0;
}
