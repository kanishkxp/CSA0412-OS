#!/bin/bash
# Runs every Q1 experiment and prints the results (saved to output_q1.txt by the caller)
cd "$(dirname "$0")"
sync; sleep 2        # flush freshly compiled files so their pages are clean in the page cache
T=../tools
echo "################ 1. Sizes of the executables ################"
ls -l bank_static bank_dynamic libencrypt.so | awk '{printf "%-18s %10d bytes\n",$9,$5}'
echo; file bank_static bank_dynamic | cut -c1-150
echo; echo "################ 2. ldd (shared libraries) ################"
echo "--- ldd bank_dynamic"; ldd ./bank_dynamic
echo "--- ldd bank_static";  ldd ./bank_static 2>&1
echo; echo "################ 3. Normal run of both versions ################"
./bank_static; ./bank_dynamic
echo; echo "################ 4. Addresses (/proc/self/maps) - dynamic version ################"
./bank_dynamic maps
echo; echo "################ 4b. Addresses - static version ################"
./bank_static maps
echo; echo "################ 5. dlopen()/dlsym() at run time ################"
./bank_dl ./libencrypt.so
echo "--- same program, customer enquiry (library not needed):"
./bank_dl ./libencrypt.so --no-encrypt
echo; echo "################ 6. Start-up time (2000 runs each) ################"
for r in 1 2 3; do echo "round $r"; $T/bench_exec 2000 ./bank_static; $T/bench_exec 2000 ./bank_dynamic; $T/bench_exec 2000 ./bank_dl ./libencrypt.so; $T/bench_exec 2000 ./bank_dl ./libencrypt.so --no-encrypt; done
echo; echo "################ 7. Memory use of one process ################"
echo "--- static";  $T/mytime ./bank_static  2>&1 >/dev/null | grep -E "Maximum resident|Minor"
echo "--- dynamic"; $T/mytime ./bank_dynamic 2>&1 >/dev/null | grep -E "Maximum resident|Minor"
for v in static dynamic; do ./bank_$v hold 3 >/dev/null & P=$!; sleep 1
  echo "--- $v: /proc/$P/smaps_rollup"; grep -E "^(Rss|Pss|Shared_Clean|Private_Clean|Private_Dirty)" /proc/$P/smaps_rollup; wait $P; done
echo; echo "################ 8. Replace the shared library WITHOUT recompiling ################"
./bank_dynamic
cp libencrypt.so libencrypt_v1.so.bak
cp libencrypt_v2.so libencrypt.so        # drop in the fixed library, nothing else changes
./bank_dynamic; ./audit_dynamic
echo "--- static version is unchanged (still has the old code inside):"; ./bank_static
cp libencrypt_v1.so.bak libencrypt.so; rm libencrypt_v1.so.bak
echo; echo "################ 9. Two different programs running together - which pages are shared? ################"
echo "=== DYNAMIC: bank_dynamic + audit_dynamic"
./bank_dynamic hold 4 >/dev/null & A=$!; ./audit_dynamic hold 4 >/dev/null & B=$!; sleep 1
python3 shared_pages.py $A $B libc; python3 shared_pages.py $A $B libencrypt; wait $A $B
echo "=== STATIC: bank_static + audit_static (libc code is inside each file)"
./bank_static hold 4 >/dev/null & A=$!; ./audit_static hold 4 >/dev/null & B=$!; sleep 1
python3 shared_pages.py $A $B bank_static >/dev/null 2>&1
python3 - <<PY
import re
def pages(pid):
    n=0
    for l in open(f"/proc/{pid}/maps"):
        p=l.split()
        if len(p)>=6 and ('static' in p[5]) and p[1][0]=='r' and p[1][1]!='w':
            a,b=[int(x,16) for x in p[0].split('-')]; n+=(b-a)//4096
    return n
print("static: read-only/exec pages mapped by bank_static =",pages($A)," by audit_static =",pages($B))
PY
python3 shared_pages.py $A $B static; wait $A $B
echo "=== Same binary twice (two copies of bank_dynamic) - code pages come from one page-cache copy"
./bank_dynamic hold 4 >/dev/null & A=$!; ./bank_dynamic hold 4 >/dev/null & B=$!; sleep 1
python3 shared_pages.py $A $B bank_dynamic; python3 shared_pages.py $A $B libc; wait $A $B
echo; echo "################ 10. Shared library missing ################"
mv libencrypt.so libencrypt.so.hidden
echo "--- bank_dynamic (linked at load time):"; ./bank_dynamic; echo "exit status = $?"
echo "--- bank_static (nothing to load):"; ./bank_static | head -1; echo "exit status = $?"
echo "--- bank_dl (dlopen with error handling):"; ./bank_dl ./libencrypt.so; echo "exit status = $?"
echo "--- bank_dl, customer enquiry still works:"; ./bank_dl ./libencrypt.so --no-encrypt; echo "exit status = $?"
mv libencrypt.so.hidden libencrypt.so
