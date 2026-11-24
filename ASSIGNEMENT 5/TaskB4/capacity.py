"""Task B4.2 - capacity estimate for result day, and the cost model used in A5 / B4.3.
All prices are example list prices in USD (assumptions, check the provider's calculator)."""
import math

students, hours = 150_000, 2
requests_per_login = 10        # assumption: login page, result page, static files, API calls
peak_factor = 5                # assumption: most students log in during the first 15-20 minutes
measured_rps = 3190            # measured: ab, gunicorn 2 workers, 1 vCPU, simple endpoint (TaskB3)
derate = 10                    # assumption: real portal with database work is 10 times slower
rps_per_1cpu_container = measured_rps / derate

avg_logins = students / (hours * 3600)
avg_rps = avg_logins * requests_per_login
peak_rps = avg_rps * peak_factor
need = math.ceil(peak_rps / rps_per_1cpu_container)
with_headroom = math.ceil(need * 1.5)

print(f"average logins per second      : {avg_logins:.1f}")
print(f"average requests per second    : {avg_rps:.0f}")
print(f"peak requests per second       : {peak_rps:.0f}")
print(f"one 1-CPU container handles    : {rps_per_1cpu_container:.0f} requests/s (measured {measured_rps} / {derate})")
print(f"containers needed at the peak  : {need}")
print(f"with 50% headroom              : {with_headroom}")
print(f"2-vCPU virtual machines needed : {math.ceil(peak_rps / (2 * rps_per_1cpu_container))} (+1 spare)")

print("\n--- yearly cost model (USD, assumptions) ---")
node = 0.096        # 2 vCPU / 8 GB instance per hour
hours_year = 8760
fixed = with_headroom * node * hours_year
base_nodes, peak_nodes, peak_hours = 2, 4, 4 * 2           # 2 result days a year, 4 hours each
auto = base_nodes * node * hours_year + (peak_nodes - base_nodes) * node * peak_hours
cp = 0.10 * hours_year                                       # managed Kubernetes control plane
db = 2 * 0.07 * hours_year                                   # managed database, two zones
lb = 0.025 * hours_year
print(f"fixed VMs sized for result day ({with_headroom} VMs all year) : {fixed:10.0f}")
print(f"autoscaled nodes (2 always, 4 for 8 h)           : {auto:10.0f}")
print(f"+ control plane {cp:.0f}, database {db:.0f}, load balancer {lb:.0f}")
print(f"public cloud total (autoscaled)                  : {auto + cp + db + lb:10.0f}")
own = 2 * 4000 / 5 + 2 * 250                                  # 2 servers over 5 years + power
print(f"own 2 servers (4,000 each over 5 years + power)  : {own:10.0f}  (+ staff time)")
print(f"idle share of fixed VMs: they are busy {peak_hours} of {hours_year} hours = {100*peak_hours/hours_year:.2f}% of the year")
