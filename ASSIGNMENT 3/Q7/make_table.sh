#!/bin/bash
# creates the 64 MB calibration table used by rt_lock_test and drops it from the page cache
dd if=/dev/urandom of=/tmp/q7_table.bin bs=1M count=64 status=none && sync
python3 -c "import os; fd=os.open('/tmp/q7_table.bin',os.O_RDONLY); os.posix_fadvise(fd,0,0,os.POSIX_FADV_DONTNEED)"
echo "table created: $(ls -l /tmp/q7_table.bin | awk '{print $5}') bytes"
