/* audit.c - a second branch program (audit report). Also uses libencrypt and libc. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "encrypt.h"
int main(int argc, char **argv)
{
    unsigned char rec[] = "AUDIT-LOG-2026-11-14 BRANCH-CHENNAI OK";
    encrypt_buf(rec, sizeof rec - 1);
    printf("audit: %s, first byte %02x\n", encrypt_version(), rec[0]);
    if (argc > 2 && strcmp(argv[1], "hold") == 0) { fflush(stdout); sleep(atoi(argv[2])); }
    return 0;
}
