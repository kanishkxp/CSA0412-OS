#!/bin/bash
# runs all Q3 programs and saves the output
cd "$(dirname "$0")"
{
echo '$ ./seg_translate'; ./seg_translate
echo; echo '$ ./segv_demo'; ./segv_demo
echo; echo '$ ./segv_demo crash   (no handler: default action of the kernel)'; ./segv_demo crash; echo "shell reports exit status = $?  (128 + 11 = SIGSEGV)"
echo; echo '$ ./seg_frag'; ./seg_frag
echo; echo '$ ./table_sizes'; ./table_sizes
echo; echo '$ ./show_segments 3 &   then   cat /proc/<pid>/maps'
./show_segments 3 > /tmp/seg_out.txt & P=$!; sleep 1; cat /tmp/seg_out.txt; echo; cat /proc/$P/maps | cut -c1-150; wait $P
} > output_q3.txt 2>&1
