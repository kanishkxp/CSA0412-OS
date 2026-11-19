/* bank.c - branch software. Links against libencrypt (static or dynamic, decided at build time).
 * Usage: ./bank            -> encrypt a sample transaction
 *        ./bank maps       -> also print /proc/self/maps
 *        ./bank hold N     -> stay alive N seconds (so other tools can look at the process)
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "encrypt.h"

static void print_file(const char *path)
{
    FILE *f = fopen(path, "r"); char line[512];
    if (!f) { perror(path); return; }
    while (fgets(line, sizeof line, f)) fputs(line, stdout);
    fclose(f);
}

int main(int argc, char **argv)
{
    unsigned char msg[] = "TRANSFER 5000 FROM ACC-1001 TO ACC-2002";
    printf("Using %s\n", encrypt_version());
    encrypt_buf(msg, sizeof msg - 1);
    printf("Encrypted bytes (hex): ");
    for (size_t i = 0; i < 12; i++) printf("%02x", msg[i]);
    printf(" ...\n");
    if (argc > 1 && strcmp(argv[1], "maps") == 0) { puts("---- /proc/self/maps ----"); print_file("/proc/self/maps"); }
    if (argc > 2 && strcmp(argv[1], "hold") == 0) { fflush(stdout); sleep(atoi(argv[2])); }
    return 0;
}
