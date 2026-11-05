#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/* Copies a file in two ways to compare the number of system calls:
   mode "sys"   : open/read/write with a buffer of b bytes
   mode "stdio" : fopen/fread/fwrite (the C library adds its own 4096 byte buffer)
   usage: ./stdio_vs_syscall sys|stdio src dst b                                  */
int main(int argc, char **argv)
{
    if (argc != 5) { fprintf(stderr, "usage: %s sys|stdio src dst b\n", argv[0]); return 1; }
    size_t b = (size_t)atol(argv[4]);
    char *buf = malloc(b);
    if (argv[1][1] == 'y') {                       /* "sys" */
        int in = open(argv[2], O_RDONLY);
        int out = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        ssize_t n;
        while ((n = read(in, buf, b)) > 0) write(out, buf, (size_t)n);
        close(in); close(out);
    } else {                                       /* "stdio" */
        FILE *in = fopen(argv[2], "rb");
        FILE *out = fopen(argv[3], "wb");
        size_t n;
        while ((n = fread(buf, 1, b, in)) > 0) fwrite(buf, 1, n, out);
        fclose(in); fclose(out);
    }
    free(buf);
    return 0;
}
