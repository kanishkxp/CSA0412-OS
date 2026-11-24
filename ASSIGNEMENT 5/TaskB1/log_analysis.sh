#!/usr/bin/env bash
# Analyse an nginx style access log: top URLs, top clients, errors per hour.
# Usage: ./log_analysis.sh access.log
set -euo pipefail
log=${1:?usage: $0 access.log}
echo "== Top 5 requested URLs =="
awk '{print $6}' "$log" | tr -d '"' | sort | uniq -c | sort -rn | awk 'NR<=5'
echo "== Top 5 clients =="
awk '{print $1}' "$log" | sort | uniq -c | sort -rn | awk 'NR<=5'
echo "== Server errors (5xx) and client errors (4xx) =="
awk '$7 ~ /^5/ {e5++} $7 ~ /^4/ {e4++} END {printf "4xx: %d   5xx: %d\n", e4, e5}' "$log"
echo "== Requests per hour =="
awk '{split($4, t, ":"); print t[2] ":00"}' "$log" | sort | uniq -c | awk '{print "  " $2 " " $1}'
