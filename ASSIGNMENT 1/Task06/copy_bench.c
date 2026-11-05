#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/resource.h>

/* Benchmark for the backup program.
   usage: ./copy_bench MODE src dst [bufsize]
   MODE = sys   : read()/write() with a buffer of bufsize bytes
          stdio : fread()/fwrite() with the same buffer size
          mmap  : mmap the source and write the whole thing with one write()  */
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }
static double tv(struct timeval t){ return t.tv_sec + t.tv_usec/1e6; }

int main(int argc, char **argv)
{
    if (argc < 4) { fprintf(stderr, "usage: %s sys|stdio|mmap src dst [bufsize]\n", argv[0]); return 1; }
    size_t b = argc > 4 ? (size_t)atol(argv[4]) : 4096;
    long reads = 0;
    double t0 = now();
    if (argv[1][0] == 's' && argv[1][1] == 'y') {
        char *buf = malloc(b);
        int in = open(argv[2], O_RDONLY);
        int out = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        ssize_t n;
        while (1) { n = read(in, buf, b); reads++; if (n <= 0) break; if (write(out, buf, (size_t)n) != n) return 2; }
        close(in); close(out); free(buf);
    } else if (argv[1][0] == 's') {
        char *buf = malloc(b);
        FILE *in = fopen(argv[2], "rb"), *out = fopen(argv[3], "wb");
        size_t n;
        while ((n = fread(buf, 1, b, in)) > 0) { reads++; fwrite(buf, 1, n, out); }
        fclose(in); fclose(out); free(buf);
    } else {
        int in = open(argv[2], O_RDONLY);
        struct stat st; fstat(in, &st);
        void *p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, in, 0);
        int out = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        size_t done = 0;
        while (done < (size_t)st.st_size) { ssize_t w = write(out, (char*)p + done, st.st_size - done); if (w < 0) return 2; done += w; }
        munmap(p, st.st_size); close(in); close(out);
    }
    double t1 = now();
    struct rusage ru; getrusage(RUSAGE_SELF, &ru);
    printf("%s b=%zu reads=%ld real=%.4f user=%.4f sys=%.4f\n", argv[1], b, reads, t1 - t0, tv(ru.ru_utime), tv(ru.ru_stime));
    return 0;
}
