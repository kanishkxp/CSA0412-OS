#!/usr/bin/env bash
# Print a warning for every mounted file system that is fuller than the limit (default 80%).
set -euo pipefail
limit=${1:-80}
df -P -x tmpfs -x devtmpfs -x squashfs -x overlay | awk -v lim="$limit" 'NR > 1 {
    use = $5; sub("%", "", use)
    if (use + 0 >= lim) printf "WARNING: %s is %s%% full (%s)\n", $6, use, $1
}'
echo "disk check finished at $(date '+%F %T') (limit ${limit}%)"
