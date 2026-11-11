"""Turns tests-results/results.csv into speedup / efficiency / time-breakdown graphs.

Usage:  python scripts/plot_results.py [results.csv] [outDir]
Needs:  matplotlib  (pip install matplotlib)
Speedup S = T_serial / T_p, efficiency E = S / cores, cores = ranks * threads.
Every configuration is run several times -> the MEDIAN total time is used.
"""
import csv, statistics, sys, os
from collections import defaultdict
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

src = sys.argv[1] if len(sys.argv) > 1 else "tests-results/results.csv"
out = sys.argv[2] if len(sys.argv) > 2 else "tests-results"
os.makedirs(out, exist_ok=True)

runs = defaultdict(list)                      # (mode, ranks, threads, reduce) -> rows
with open(src, newline="") as f:
    for row in csv.DictReader(f):
        runs[(row["mode"], int(row["ranks"]), int(row["threads"]), row["reduce"])].append(row)

def med(rows, col): return statistics.median(float(r[col]) for r in rows)

serial = [k for k in runs if k[0] == "serial"]
if not serial: sys.exit("no 'serial' rows in the CSV - run the baseline first")
t_serial = med(runs[serial[0]], "total_s")

# verify correctness: every configuration must have produced the same checksum
sums = {r["checksum"] for rows in runs.values() for r in rows}
print("checksums:", "ALL EQUAL (correct)" if len(sums) == 1 else f"MISMATCH {sums}")

series = {   # label -> sorted list of (cores, total time)
    "OpenMP":                 sorted((k[2], med(v, "total_s")) for k, v in runs.items() if k[0] == "omp"),
    "MPI alltoall (1 thr)":   sorted((k[1], med(v, "total_s")) for k, v in runs.items() if k[0] == "mpi" and k[2] == 1 and k[3] == "alltoall"),
    "MPI gather (1 thr)":     sorted((k[1], med(v, "total_s")) for k, v in runs.items() if k[0] == "mpi" and k[2] == 1 and k[3] == "gather"),
    "Hybrid MPI+OpenMP":      sorted((k[1] * k[2], med(v, "total_s")) for k, v in runs.items() if k[0] == "mpi" and k[2] > 1),
}
series = {k: v for k, v in series.items() if v}

def plot(ylabel, fn, name, ideal=False):
    plt.figure(figsize=(6, 4.2))
    for label, pts in series.items():
        plt.plot([c for c, _ in pts], [fn(c, t) for c, t in pts], marker="o", label=label)
    if ideal:
        cmax = max(c for pts in series.values() for c, _ in pts)
        plt.plot([1, cmax], [1, cmax], "k--", label="ideal")
    plt.xlabel("cores (ranks x threads)"); plt.ylabel(ylabel); plt.grid(alpha=.3); plt.legend()
    plt.tight_layout(); plt.savefig(os.path.join(out, name), dpi=150); plt.close()

plot("speedup", lambda c, t: t_serial / t, "speedup.png", ideal=True)
plot("efficiency", lambda c, t: t_serial / t / c, "efficiency.png")

# stacked bars: where does the time go?
labels, parts = [], {"read_s": [], "count_s": [], "reduce_s": [], "setup_s": []}
for k, v in sorted(runs.items()):
    labels.append(f"{k[0]}\n{k[1]}x{k[2]}\n{k[3]}")
    for p in parts: parts[p].append(med(v, p))
plt.figure(figsize=(max(6, len(labels) * .7), 4.5)); bottom = [0] * len(labels)
for p, vals in parts.items():
    plt.bar(labels, vals, bottom=bottom, label=p); bottom = [b + x for b, x in zip(bottom, vals)]
plt.ylabel("seconds"); plt.legend(); plt.xticks(fontsize=7); plt.tight_layout()
plt.savefig(os.path.join(out, "time_breakdown.png"), dpi=150)
print("wrote speedup.png, efficiency.png, time_breakdown.png to", out)
