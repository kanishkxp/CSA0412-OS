#!/bin/bash
# Build every Q1 binary. Compiler: gcc 13.3.0 on Ubuntu 24.04
set -e
CF="-std=c11 -Wall -Wextra -O2"
# 1. libraries (position independent code is required for shared libraries)
gcc $CF -fPIC -shared -o libencrypt.so encrypt_v1.c
gcc $CF -fPIC -shared -o libencrypt_v2.so encrypt_v2.c
# 2. dynamic build: bank and audit link against libencrypt.so at load time (rpath = current directory)
gcc $CF -o bank_dynamic bank.c -L. -lencrypt -Wl,-rpath,'$ORIGIN'
gcc $CF -o audit_dynamic audit.c -L. -lencrypt -Wl,-rpath,'$ORIGIN'
# 3. static build: library code and libc are copied INTO the executable
gcc $CF -static -o bank_static bank.c encrypt_v1.c
gcc $CF -static -o audit_static audit.c encrypt_v1.c
# 4. run-time loading with dlopen()
gcc $CF -o bank_dl bank_dl.c -ldl
echo "build finished"
