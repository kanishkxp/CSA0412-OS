/* bank_dl.c - loads the encryption library only when it is needed, using dlopen()/dlsym().
 * Usage: ./bank_dl [library-path] [--no-encrypt]
 * Build: gcc -std=c11 -Wall -Wextra -O2 -o bank_dl bank_dl.c -ldl
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

typedef void (*encrypt_fn)(unsigned char *, size_t);
typedef const char *(*version_fn)(void);

int main(int argc, char **argv)
{
    const char *path = (argc > 1 && argv[1][0] != '-') ? argv[1] : "./libencrypt.so";
    int need_encrypt = !(argc > 1 && strcmp(argv[argc - 1], "--no-encrypt") == 0);

    if (!need_encrypt) { puts("Customer enquiry only - encryption library NOT loaded."); return 0; }

    void *h = dlopen(path, RTLD_NOW);
    if (!h) {                                   /* error handling for a missing library */
        fprintf(stderr, "bank_dl: cannot load %s: %s\n", path, dlerror());
        fputs("bank_dl: falling back - transaction REFUSED, encryption unavailable.\n", stderr);
        return 3;
    }
    encrypt_fn enc = (encrypt_fn)dlsym(h, "encrypt_buf");
    version_fn ver = (version_fn)dlsym(h, "encrypt_version");
    if (!enc || !ver) { fprintf(stderr, "bank_dl: symbol error: %s\n", dlerror()); dlclose(h); return 4; }

    unsigned char msg[] = "TRANSFER 5000 FROM ACC-1001 TO ACC-2002";
    printf("dlopen OK: %s\n", ver());
    enc(msg, sizeof msg - 1);
    printf("Encrypted bytes (hex): ");
    for (size_t i = 0; i < 12; i++) printf("%02x", msg[i]);
    printf(" ...\n");
    printf("encrypt_buf() found by dlsym at address %p\n", (void *)enc);
    puts("---- libencrypt mapping seen in /proc/self/maps ----");
    FILE *f = fopen("/proc/self/maps", "r"); char line[512];
    while (f && fgets(line, sizeof line, f)) if (strstr(line, "libencrypt")) fputs(line, stdout);
    if (f) fclose(f);
    dlclose(h);
    return 0;
}
