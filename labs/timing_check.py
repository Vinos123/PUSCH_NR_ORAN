"""Offline timing exercise. All times must already use the same reference, in us."""
import csv
import math
import sys


def analyze(row):
    for field in ("cell", "ue", "frame", "slot"):
        if int(row[field]) < 0:
            raise ValueError(f"negative {field}")
    names = ("cfg_us", "iq_us", "enqueue_us", "start_us", "end_us", "report_us", "deadline_us")
    times = {key: float(row[key]) for key in names}
    if not all(math.isfinite(value) for value in times.values()):
        raise ValueError("non-finite time")
    ready = max(times["cfg_us"], times["iq_us"])
    if not ready <= times["enqueue_us"] <= times["start_us"] <= times["end_us"] <= times["report_us"]:
        raise ValueError("events are inconsistent: check clock domains or producer/consumer timing")
    return {
        "ready_to_enqueue_us": times["enqueue_us"] - ready,
        "queue_us": times["start_us"] - times["enqueue_us"],
        "compute_us": times["end_us"] - times["start_us"],
        "report_delay_us": times["report_us"] - times["end_us"],
        "margin_us": times["deadline_us"] - times["report_us"],
    }


def main(path):
    misses = 0
    with open(path, newline="", encoding="utf-8") as stream:
        for number, row in enumerate(csv.DictReader(stream), start=2):
            result = analyze(row)
            status = "DEADLINE_MISS" if result["margin_us"] < 0 else "WITHIN_DEADLINE"
            misses += result["margin_us"] < 0
            print(f"row={number}, cell={row['cell']}, ue={row['ue']}, frame={row['frame']}, slot={row['slot']}, {status}, {result}")
    return 1 if misses else 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: python3 timing_check.py events.csv", file=sys.stderr)
        sys.exit(2)
    try:
        sys.exit(main(sys.argv[1]))
    except (ValueError, KeyError, OSError) as error:
        print(error, file=sys.stderr)
        sys.exit(2)
