#!/usr/bin/env bash
# Summarise a web-server access log: requests per status code and the busiest client.
set -euo pipefail

log=$(mktemp)
cat > "$log" <<'LOG'
10.0.0.5 - - [08/Oct/2026:10:01:02] "GET /index.html" 200 512
10.0.0.7 - - [08/Oct/2026:10:01:05] "GET /login" 200 1024
10.0.0.5 - - [08/Oct/2026:10:01:09] "POST /login" 302 0
10.0.0.9 - - [08/Oct/2026:10:02:11] "GET /admin" 403 128
10.0.0.5 - - [08/Oct/2026:10:02:15] "GET /report.pdf" 200 40960
10.0.0.7 - - [08/Oct/2026:10:03:01] "GET /missing" 404 64
10.0.0.5 - - [08/Oct/2026:10:03:30] "GET /index.html" 200 512
LOG

echo "Requests per status code:"
# field 7 is the status code; sort, count equal lines with uniq -c, sort by code, print "  200 4"
awk '{print $7}' "$log" | sort | uniq -c | sort -k2,2n | awk '{print "  " $2 " " $1}'

echo "Busiest client:"
# field 1 is the IP address; the biggest count comes first after sort -rn
awk '{print $1}' "$log" | sort | uniq -c | sort -rn | awk 'NR==1 {print "  " $2 " " $1}'

echo "Total bytes sent: $(awk '{s += $8} END {print s}' "$log")"
rm -f "$log"
