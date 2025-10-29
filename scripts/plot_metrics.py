#!/usr/bin/env python3
import csv
import sys
import os
from collections import defaultdict, OrderedDict

import matplotlib
matplotlib.use("Agg")  # non-interactive backend
import matplotlib.pyplot as plt


def ensure_dir(path: str):
    os.makedirs(path, exist_ok=True)


def read_scalars(path: str):
    """
    Read scalars.csv with columns: run,repetition,module,name,value
    Returns dicts keyed by run.
    """
    by_run = defaultdict(list)
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            # Basic sanitation
            try:
                val = float(row.get("value", ""))
            except ValueError:
                continue
            run = row.get("run", "")
            by_run[run].append({
                "module": row.get("module", ""),
                "name": row.get("name", ""),
                "value": val,
            })
    return by_run


def read_queue_vectors(path: str):
    """
    Read CSV-R vectors filtered for router queueLength.
    Expected header: run,type,module,name,attrname,attrvalue,vectime,vecvalue
    Returns dict: { run: [(t, v), ...] }
    """
    series = defaultdict(list)
    # Increase CSV field size limit to accommodate long runattr values
    try:
        csv.field_size_limit(10**7)
    except Exception:
        pass
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            t = row.get("vectime", "")
            v = row.get("vecvalue", "")
            if t and v:
                run = row.get("run", "")
                # vectime/vecvalue may contain whitespace-separated series
                tparts = t.strip().split()
                vparts = v.strip().split()
                if len(tparts) == 1 and len(vparts) == 1:
                    # Single sample case
                    try:
                        tt = float(tparts[0])
                        vv = float(vparts[0])
                    except ValueError:
                        continue
                    series[run].append((tt, vv))
                else:
                    # Sequence case
                    n = min(len(tparts), len(vparts))
                    pts = []
                    for i in range(n):
                        try:
                            tt = float(tparts[i])
                            vv = float(vparts[i])
                        except ValueError:
                            continue
                        pts.append((tt, vv))
                    series[run].extend(pts)
    # Sort by time for each run
    for r in series:
        series[r].sort(key=lambda x: x[0])
    return series


def scenario_name(run_id: str) -> str:
    # Run IDs look like: General-0-YYYYMMDD-hh:mm:ss-XXXXX
    if not run_id:
        return run_id
    return run_id.split("-")[0]


def plot_queue_length(series_by_run, out_path: str):
    plt.figure(figsize=(8, 4.8))
    # stable order
    order = ["Light", "General", "Mixed", "Heavy", "Congestion"]
    # collect all runs
    runs = list(series_by_run.keys())
    runs_sorted = sorted(runs, key=lambda r: order.index(scenario_name(r)) if scenario_name(r) in order else 999)
    for run in runs_sorted:
        points = series_by_run[run]
        if not points:
            continue
        x = [p[0] for p in points]
        y = [p[1] for p in points]
        plt.plot(x, y, label=scenario_name(run))
    plt.xlabel("Simulation Time (s)")
    plt.ylabel("Queue Length (packets)")
    plt.title("Router Queue Length Over Time")
    plt.legend(loc="best")
    plt.tight_layout()
    plt.savefig(out_path, dpi=160)
    plt.close()


def collect_scalar(by_run, name_predicate, module_predicate=lambda m: True):
    """
    Collect scalar values per run for items whose name and module match predicates.
    Returns OrderedDict of { run: {name: value, ...} }.
    If there are multiple matches for a given (run, name), take the last.
    """
    out = OrderedDict()
    for run, items in by_run.items():
        bucket = {}
        for it in items:
            n = it["name"]
            m = it["module"]
            if name_predicate(n) and module_predicate(m):
                bucket[n] = it["value"]
        out[run] = bucket
    return out


def plot_delay_by_type(by_run, out_path: str):
    def is_server(m):
        return ".server" in m

    def is_delay(n):
        return n in ("voiceAverageDelay", "videoAverageDelay", "dataAverageDelay")

    data = collect_scalar(by_run, is_delay, is_server)
    # Scenarios order
    order = ["Light", "General", "Mixed", "Heavy", "Congestion"]
    # Prepare bars
    runs = list(data.keys())
    runs_sorted = [r for r in sorted(runs, key=lambda r: order.index(scenario_name(r)) if scenario_name(r) in order else 999)]
    labels = [scenario_name(r) for r in runs_sorted]
    voice = [data[r].get("voiceAverageDelay", float("nan")) for r in runs_sorted]
    video = [data[r].get("videoAverageDelay", float("nan")) for r in runs_sorted]
    datad = [data[r].get("dataAverageDelay", float("nan")) for r in runs_sorted]

    x = list(range(len(labels)))
    w = 0.25
    plt.figure(figsize=(8, 4.8))
    plt.bar([xx - w for xx in x], voice, width=w, label="Voice")
    plt.bar(x, video, width=w, label="Video")
    plt.bar([xx + w for xx in x], datad, width=w, label="Data")
    plt.xticks(x, labels)
    plt.ylabel("Average Delay (s)")
    plt.title("Average End-to-End Delay by Traffic Class")
    plt.legend(loc="best")
    plt.tight_layout()
    plt.savefig(out_path, dpi=160)
    plt.close()


def plot_packet_loss(by_run, out_path: str):
    def is_router(m):
        return ".router" in m

    def is_loss(n):
        return n in ("packetsDropped", "packetsDropped:count", "lowPriorityDropped")

    data = collect_scalar(by_run, is_loss, is_router)
    order = ["Light", "General", "Mixed", "Heavy", "Congestion"]
    runs = list(data.keys())
    runs_sorted = [r for r in sorted(runs, key=lambda r: order.index(scenario_name(r)) if scenario_name(r) in order else 999)]
    labels = [scenario_name(r) for r in runs_sorted]
    total_loss = []
    low_loss = []
    for r in runs_sorted:
        # Prefer explicit count if present
        total = data[r].get("packetsDropped:count", data[r].get("packetsDropped", float("nan")))
        total_loss.append(total)
        low_loss.append(data[r].get("lowPriorityDropped", float("nan")))

    x = list(range(len(labels)))
    w = 0.35
    plt.figure(figsize=(8, 4.8))
    plt.bar([xx - w/2 for xx in x], total_loss, width=w, label="Total Dropped")
    plt.bar([xx + w/2 for xx in x], low_loss, width=w, label="Low-Priority Dropped")
    plt.xticks(x, labels)
    plt.ylabel("Packets")
    plt.title("Packet Loss Across Scenarios")
    plt.legend(loc="best")

    # If all values are zero or NaN, annotate the figure for clarity
    def _finite(vals):
        return [v for v in vals if (v == v)]  # filter out NaN

    finite_total = _finite(total_loss)
    finite_low = _finite(low_loss)
    if (sum(v for v in finite_total) == 0) and (sum(v for v in finite_low) == 0):
        plt.ylim(0, 1)
        plt.text(0.5, 0.5, "No packet drops observed in current runs",
                 ha="center", va="center", transform=plt.gca().transAxes,
                 fontsize=11, color="#555")

    plt.tight_layout()
    plt.savefig(out_path, dpi=160)
    plt.close()


def plot_throughput_by_type(by_run, out_path: str):
    def is_server(m):
        return ".server" in m

    def is_tput(n):
        return n in ("voiceThroughput", "videoThroughput", "dataThroughput")

    data = collect_scalar(by_run, is_tput, is_server)
    order = ["Light", "General", "Mixed", "Heavy", "Congestion"]
    runs = list(data.keys())
    runs_sorted = [r for r in sorted(runs, key=lambda r: order.index(scenario_name(r)) if scenario_name(r) in order else 999)]
    labels = [scenario_name(r) for r in runs_sorted]
    voice = [data[r].get("voiceThroughput", float("nan")) for r in runs_sorted]
    video = [data[r].get("videoThroughput", float("nan")) for r in runs_sorted]
    datad = [data[r].get("dataThroughput", float("nan")) for r in runs_sorted]

    x = list(range(len(labels)))
    w = 0.25
    plt.figure(figsize=(8, 4.8))
    plt.bar([xx - w for xx in x], voice, width=w, label="Voice")
    plt.bar(x, video, width=w, label="Video")
    plt.bar([xx + w for xx in x], datad, width=w, label="Data")
    plt.xticks(x, labels)
    plt.ylabel("Throughput (packets/s)")
    plt.title("Throughput by Traffic Class")
    plt.legend(loc="best")
    plt.tight_layout()
    plt.savefig(out_path, dpi=160)
    plt.close()


def main():
    base_results = os.path.join("results")
    scalars_csv = os.path.join(base_results, "scalars.csv")
    qlen_vectors_csv = os.path.join(base_results, "queue_length_vectors.csv")
    out_dir = os.path.join("figures")
    ensure_dir(out_dir)

    # Read inputs
    if not os.path.exists(scalars_csv):
        raise FileNotFoundError(f"Missing {scalars_csv}. Export scalars first using opp_scavetool.")

    scalars_by_run = read_scalars(scalars_csv)
    queue_series_by_run = {}
    if os.path.exists(qlen_vectors_csv):
        queue_series_by_run = read_queue_vectors(qlen_vectors_csv)
    else:
        # Fallback: try results/queueLength.csv if present (legacy multi-run format)
        legacy_csv = os.path.join(base_results, "queueLength.csv")
        if os.path.exists(legacy_csv):
            # Legacy format: header with labels; rows contain time,value pairs for each run
            with open(legacy_csv, newline="") as f:
                reader = csv.reader(f)
                header = next(reader, None)
                if header:
                    # Build mapping: every two columns constitute (time,value) for a run
                    # Extract scenario names from header entries
                    pairs = []
                    for i in range(0, len(header), 2):
                        label = header[i]
                        if not label:
                            continue
                        # Parse e.g., "queueLength AdaptivePriorityNetwork.router (#0 - Light-...)"
                        scenario = label.split("-")[0].split("(")[-1].strip()
                        pairs.append((i, i + 1, scenario))
                    # Read rows
                    series = defaultdict(list)
                    for row in reader:
                        for i, j, scenario in pairs:
                            try:
                                t = float(row[i])
                                v = float(row[j])
                            except (ValueError, IndexError):
                                continue
                            series[scenario].append((t, v))
                    # Sort and adopt
                    for sc in series:
                        series[sc].sort(key=lambda x: x[0])
                        # Use scenario as pseudo-run key
                        queue_series_by_run[sc] = series[sc]

    # Generate figures
    if queue_series_by_run:
        plot_queue_length(queue_series_by_run, os.path.join(out_dir, "queue_length_timeseries.png"))
    else:
        # No queue vectors found; skip this figure
        pass

    plot_delay_by_type(scalars_by_run, os.path.join(out_dir, "delay_by_type.png"))
    plot_packet_loss(scalars_by_run, os.path.join(out_dir, "packet_loss.png"))
    plot_throughput_by_type(scalars_by_run, os.path.join(out_dir, "throughput_by_type.png"))

    print("Figures written to:")
    for name in [
        "queue_length_timeseries.png",
        "delay_by_type.png",
        "packet_loss.png",
        "throughput_by_type.png",
    ]:
        path = os.path.join(out_dir, name)
        print(" -", path, "exists=" + str(os.path.exists(path)))


if __name__ == "__main__":
    main()