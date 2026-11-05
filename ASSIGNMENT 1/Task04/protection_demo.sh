#!/bin/bash
# Task 4: the operating system protects one patient's record from another user's program.
# Run as root (or with sudo) because it switches to a second user with setpriv.
mkdir -p /tmp/ward && cd /tmp/ward
echo "patient 101: diagnosis confidential" > record101.txt
chmod 600 record101.txt                       # only the owner may read or write
ls -l record101.txt
echo "--- owner reads the record:"
cat record101.txt
echo "--- another user (uid 65534, 'nobody') tries to read it:"
setpriv --reuid=65534 --regid=65534 --clear-groups cat record101.txt
echo "exit status: $?"
echo "--- the same user tries to read it with strace to see the failing system call:"
setpriv --reuid=65534 --regid=65534 --clear-groups strace -e trace=openat cat record101.txt 2>&1 | grep record101
rm -rf /tmp/ward
