#!/bin/bash
# runs all Q2 programs and saves the output
cd "$(dirname "$0")"
./page_translate > out_page_translate_1024.txt
./page_translate_2048 > out_page_translate_2048.txt
{
echo '$ ./boundary_test'; ./boundary_test
echo; echo '$ ./calc_q2'; ./calc_q2
echo; echo '$ ./frag_pages'; ./frag_pages
echo; echo '$ getconf PAGESIZE'; getconf PAGESIZE
echo; echo '$ ../tools/mytime ./alloc_touch 100'; ../tools/mytime ./alloc_touch 100
echo; echo '$ ../tools/mytime /bin/ls  (small program)'; ../tools/mytime /bin/ls > /dev/null
echo; echo '$ ./pt_compare'; ./pt_compare
} > output_q2.txt 2>&1
