/* encrypt_v1.c - the bank's encryption library, version 1 (the one with the bug) */
#include <stddef.h>
const char *encrypt_version(void) { return "libencrypt v1.0 (old, weak key schedule)"; }

/* very simple XOR "cipher" - only for demonstration, NOT real encryption */
void encrypt_buf(unsigned char *buf, size_t n)
{
    unsigned char key = 0x5A;
    for (size_t i = 0; i < n; i++) { buf[i] ^= key; key = (unsigned char)(key + 7); }
}
