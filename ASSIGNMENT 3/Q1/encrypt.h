#ifndef ENCRYPT_H
#define ENCRYPT_H
#include <stddef.h>
const char *encrypt_version(void);
void encrypt_buf(unsigned char *buf, size_t n);
#endif
