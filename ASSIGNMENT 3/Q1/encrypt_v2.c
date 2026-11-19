/* encrypt_v2.c - the bank's encryption library, version 2 (the security fix) */
#include <stddef.h>
const char *encrypt_version(void) { return "libencrypt v2.0 (security fix applied)"; }

void encrypt_buf(unsigned char *buf, size_t n)
{
    unsigned char key = 0xC3;
    for (size_t i = 0; i < n; i++) { buf[i] ^= key; key = (unsigned char)((key << 1) ^ (key >> 7) ^ 0x1D); }
}
