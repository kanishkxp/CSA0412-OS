"""Task 2: availability of ONE big multiprocessor server versus a TWO-node cluster with fail-over.
All input numbers are assumptions (written in the report); change them to test other cases."""
HOURS_YEAR = 8760
mtbf = 2000.0        # hours between failures of one machine
mttr = 8.0           # hours to repair or replace a failed machine
failover_s = 10.0    # seconds for the standby node to take over

a = mtbf / (mtbf + mttr)                       # availability of one machine
single_down_h = (1 - a) * HOURS_YEAR           # single big server: every failure stops the service
failures_per_year = HOURS_YEAR / mtbf
cluster_failover_s = failures_per_year * failover_s          # short stop at every failure of the active node
both_down_h = ((1 - a) ** 2) * HOURS_YEAR                    # both nodes failed together
cluster_down_s = cluster_failover_s + both_down_h * 3600
print(f"availability of one machine           : {a*100:.3f} %")
print(f"single server downtime per year        : {single_down_h:.1f} hours")
print(f"failures per year per machine          : {failures_per_year:.2f}")
print(f"cluster fail-over downtime per year    : {cluster_failover_s:.1f} seconds")
print(f"cluster both-nodes-down per year       : {both_down_h*3600:.0f} seconds")
print(f"cluster total downtime per year        : {cluster_down_s:.0f} seconds = {cluster_down_s/60:.1f} minutes")
print(f"cluster availability                   : {(1 - cluster_down_s/(HOURS_YEAR*3600))*100:.4f} %")

# Throughput growth with processors (Amdahl's law), serial part 5 %
s = 0.05
print("\nprocessors  speed-up (Amdahl, 5% serial)")
for p in (1, 2, 4, 8, 16, 32, 64):
    print(f"{p:>10}  {1/(s + (1-s)/p):6.2f}")
