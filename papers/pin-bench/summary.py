# Summary of results.csv: median seconds and median peak RSS per program and variant,
# and the ratios that the paper uses. Run: python3 -I summary.py results.csv
import csv, sys, statistics as st
rows = list(csv.DictReader(open(sys.argv[1])))
V = ["c", "lua54", "luajit", "off", "unchecked", "on"]
P = []
for r in rows:
    if r["program"] not in P: P.append(r["program"])
bad = [r for r in rows if r["exit"] != "0"]
if bad: print("NONZERO EXIT:", bad)
def med (p, v, k):
    xs = [float(r[k]) for r in rows if r["program"] == p and r["variant"] == v]
    return st.median(xs) if xs else float("nan")
print("| program | size | " + " | ".join(V) + " | on/off | on/unchecked | on/lua54 |")
print("|---|---|" + "---|"*len(V) + "---|---|---|")
for p in P:
    size = next(r["size"] for r in rows if r["program"] == p)
    t = {v: med(p, v, "seconds") for v in V}
    print(f"| {p} | {size} | " + " | ".join(f"{t[v]:.3f}" for v in V) +
          f" | {t['on']/t['off']:.2f} | {t['on']/t['unchecked']:.2f} | {t['on']/t['lua54']:.2f} |")
print()
print("Peak RSS, MB (median)")
print("| program | " + " | ".join(V) + " |")
print("|---|" + "---|"*len(V))
for p in P:
    print(f"| {p} | " + " | ".join(f"{med(p, v, 'peak_rss_kb')/1024:.1f}" for v in V) + " |")
print()
print("Spread: max/min of the 5 runs of `on`, per program")
for p in P:
    xs = [float(r["seconds"]) for r in rows if r["program"] == p and r["variant"] == "on"]
    print(f"- {p}: {max(xs)/min(xs):.3f} over {len(xs)} runs")
print()
print("Best of the runs (min), seconds: less sensitive to a noisy machine")
print("| program | " + " | ".join(V) + " | on/off | on/lua54 |")
print("|---|" + "---|"*len(V) + "---|---|")
for p in P:
    m = {v: min(float(r["seconds"]) for r in rows if r["program"] == p and r["variant"] == v) for v in V}
    print(f"| {p} | " + " | ".join(f"{m[v]:.3f}" for v in V) + f" | {m['on']/m['off']:.2f} | {m['on']/m['lua54']:.2f} |")

if rows and "cpu_seconds" in rows[0]:
    print()
    print("Best of the runs (min), CPU seconds (user + system): the figure the paper uses")
    print("| program | " + " | ".join(V) + " | on/off | on/lua54 | off/lua54 |")
    print("|---|" + "---|"*len(V) + "---|---|---|")
    for p in P:
        m = {v: min(float(r["cpu_seconds"]) for r in rows if r["program"] == p and r["variant"] == v) for v in V}
        print(f"| {p} | " + " | ".join(f"{m[v]:.3f}" for v in V) + f" | {m['on']/m['off']:.2f} | {m['on']/m['lua54']:.2f} | {m['off']/m['lua54']:.2f} |")
