#!/usr/bin/env python3
"""shared_pages.py PID1 PID2 [name-fragment]
Reads /proc/<pid>/maps and /proc/<pid>/pagemap (root needed to see frame numbers) and counts how many
file-backed read-only/executable pages of the two processes live in the SAME physical frame."""
import sys, struct

def regions(pid, frag):
    out = []
    for line in open(f"/proc/{pid}/maps"):
        parts = line.split()
        if len(parts) < 6 or frag not in parts[5]: continue
        if parts[1][0] != 'r' or parts[1][1] == 'w': continue      # read-only / executable only
        a, b = (int(x, 16) for x in parts[0].split('-'))
        out.append((a, b, parts[5]))
    return out

def frames(pid, regs):
    s = set(); resident = 0
    with open(f"/proc/{pid}/pagemap", "rb") as f:
        for a, b, _ in regs:
            for va in range(a, b, 4096):
                f.seek(va // 4096 * 8)
                e = struct.unpack("Q", f.read(8))[0]
                if e >> 63 & 1:                       # page present
                    resident += 1
                    s.add(e & ((1 << 55) - 1))        # frame number
    return s, resident

p1, p2 = sys.argv[1], sys.argv[2]
frag = sys.argv[3] if len(sys.argv) > 3 else "libc"
r1, r2 = regions(p1, frag), regions(p2, frag)
f1, n1 = frames(p1, r1); f2, n2 = frames(p2, r2)
common = f1 & f2
print(f"filter '{frag}': process {p1} has {n1} resident read-only/exec pages, process {p2} has {n2}")
print(f"physical frames used by BOTH processes: {len(common)}")
print(f"=> {len(common)} of {n1} pages ({100.0*len(common)/max(n1,1):.0f}%) are shared")
