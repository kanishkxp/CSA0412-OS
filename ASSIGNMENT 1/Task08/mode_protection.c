#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

/* Shows the hardware and the OS stopping a user program.
   usage: ./mode_protection cli | kernel | null
     cli    : try to run the privileged instruction CLI (disable interrupts)
     kernel : try to read a kernel address
     null   : read address 0                                                    */
static void handler(int sig){
    const char m[] = "caught SIGSEGV: the CPU refused the access and the kernel killed the program\n";
    write(2, m, sizeof m - 1); _exit(sig);
}
int main(int argc, char **argv)
{
    if (argc != 2) return 1;
    signal(SIGSEGV, handler);
    if (!strcmp(argv[1], "cli")) {
        printf("user mode: trying the privileged instruction cli ...\n"); fflush(stdout);
#if defined(__x86_64__)
        __asm__ volatile("cli");
#endif
    } else if (!strcmp(argv[1], "kernel")) {
        printf("user mode: reading kernel address 0xffffffff81000000 ...\n"); fflush(stdout);
        volatile char *p = (char *)0xffffffff81000000UL; char c = *p; (void)c;
    } else {
        printf("user mode: reading address 0 ...\n"); fflush(stdout);
        volatile char *p = NULL; char c = *p; (void)c;
    }
    printf("this line should never print\n");
    return 0;
}
