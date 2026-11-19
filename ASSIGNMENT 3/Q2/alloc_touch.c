/* alloc_touch.c - allocate N MB and touch one byte in every page; page faults happen on first touch. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char **argv)
{
    size_t mb = argc > 1 ? (size_t)atol(argv[1]) : 100, bytes = mb << 20, pg = (size_t)sysconf(_SC_PAGESIZE);
    char *p = malloc(bytes);
    if (!p) { perror("malloc"); return 1; }
    for (size_t i = 0; i < bytes; i += pg) p[i] = 1;
    printf("allocated and touched %zu MB = %zu pages of %zu bytes\n", mb, bytes / pg, pg);
    free(p); return 0;
}
