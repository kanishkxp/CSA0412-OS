#!/bin/bash
# runs the Q6 commands and programs and saves the output (fdisk may not be installed; lsblk and /proc/partitions are used too)
cd "$(dirname "$0")"
[ -f /tmp/q4_data.bin ] || dd if=/dev/urandom of=/tmp/q4_data.bin bs=1M count=1024 status=none
{
echo '$ lsblk -o NAME,SIZE,TYPE,ROTA,FSTYPE,MOUNTPOINTS'; lsblk -o NAME,SIZE,TYPE,ROTA,FSTYPE,MOUNTPOINTS
echo; echo '$ cat /proc/partitions'; cat /proc/partitions
echo; echo '$ fdisk -l   (fdisk is not installed on this machine)'; (fdisk -l 2>&1 || true)
echo; echo '$ blkid /dev/vda'; blkid /dev/vda 2>&1
echo; echo '$ findmnt /'; findmnt /
echo; echo '$ cat /proc/swaps'; cat /proc/swaps
echo; echo '$ free -h'; free -h
echo; echo '$ cat /sys/block/vda/queue/rotational  (1 = reported as spinning disk, set by the virtual disk driver)'; cat /sys/block/vda/queue/rotational
echo; echo '$ ./raid_calc'; ./raid_calc
echo; echo '$ ./parity_demo'; ./parity_demo
echo; echo '$ ./read_speed /tmp/q4_data.bin  (run 3 times)'; for i in 1 2 3; do ./read_speed /tmp/q4_data.bin; echo; done
} > output_q6.txt 2>&1
