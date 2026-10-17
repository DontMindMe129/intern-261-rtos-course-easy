#!/usr/bin/env python3
"""Analyze a full-sample PulseView CSV (Dedup rows OFF, Time column ON).

Only standard-library modules are needed. Run from the repository root:

    python week1/ex3/tools/analyze_timing.py \
        week1/ex3/measurements/cfg1/data.csv --sample-rate 1000000

Every rising AND falling edge is a task-iteration marker. By default, use
the first 501 edges, giving 500 intervals. Time is reconstructed from sample
indices and the explicitly supplied acquisition sample rate, avoiding integer
rounding in PulseView's exported microseconds column. This is NOT suitable for
deduplicated CSV data. The sample rate is the analyzer's rate, not the CPU or
FreeRTOS tick rate.
"""

import argparse
import csv
import json
import math
import sys
from pathlib import Path


def positive_float(value):
    number = float(value)
    if not math.isfinite(number) or number <= 0:
        raise argparse.ArgumentTypeError("must be a positive, finite number")
    return number


def positive_int(value):
    number = int(value)
    if number <= 0:
        raise argparse.ArgumentTypeError("must be a positive integer")
    return number


def nonnegative_int(value):
    number = int(value)
    if number < 0:
        raise argparse.ArgumentTypeError("must be zero or greater")
    return number


def analyze(path, sample_rate, samples=500, period_ms=23.0, skip_edges=0, bit=0):
    """Read CSV in one pass; retain only the selected interval boundaries."""
    selected_edges = []
    edge_count = 0
    row_count = 0
    previous_level = None
    previous_time = None
    sample_us = 1_000_000.0 / sample_rate

    with Path(path).open("r", encoding="utf-8-sig", newline="") as source:
        # PulseView's optional metadata comments start with ';'.
        rows = csv.reader(
            line for line in source if line.strip() and not line.lstrip().startswith(";")
        )
        header = next(rows, None)
        if header is None:
            raise ValueError("CSV is empty")
        columns = [column.strip().lower() for column in header]
        if "microseconds" not in columns or "logic" not in columns:
            raise ValueError(
                "Expected 'microseconds,logic' columns. Export with Output header "
                "and Time column ON, Dedup rows OFF."
            )
        time_column = columns.index("microseconds")
        logic_column = columns.index("logic")

        for row in rows:
            try:
                time_us = float(row[time_column])
                logic = int(row[logic_column].strip(), 10)
            except (ValueError, IndexError) as exc:
                raise ValueError(f"Invalid data row {row_count + 1}: {row!r}") from exc
            if not math.isfinite(time_us) or logic < 0:
                raise ValueError(f"Invalid time/logic at data row {row_count + 1}")
            if previous_time is not None and time_us < previous_time:
                raise ValueError(f"Time decreases at data row {row_count + 1}")
            # Allow one sample of export rounding (observed with integer us).
            # Large gaps indicate deduplication, missing rows or a wrong rate.
            expected_us = row_count * sample_us
            if abs(time_us - expected_us) > max(1.0, sample_us) + 0.01:
                raise ValueError(
                    f"Time/sample mismatch at data row {row_count + 1}: "
                    f"CSV={time_us:g} us, expected={expected_us:g} us. "
                    "Check --sample-rate and re-export the entire capture "
                    "with Dedup rows OFF."
                )
            level = (logic >> bit) & 1
            if previous_level is not None and level != previous_level:
                if edge_count >= skip_edges and len(selected_edges) < samples + 1:
                    selected_edges.append(row_count)
                edge_count += 1
            previous_level = level
            previous_time = time_us
            row_count += 1

    if len(selected_edges) != samples + 1:
        raise ValueError(
            f"Need {samples + 1} edges after skipping {skip_edges}; "
            f"found only {len(selected_edges)}. Total detected edges: {edge_count}."
        )

    intervals = [b - a for a, b in zip(selected_edges, selected_edges[1:])]
    elapsed_ms = (selected_edges[-1] - selected_edges[0]) * 1000.0 / sample_rate
    return {
        "csv": str(Path(path)),
        "sample_rate_hz": sample_rate,
        "sample_resolution_us": sample_us,
        "logic_bit": bit,
        "capture_samples": row_count,
        "capture_duration_s": row_count / sample_rate,
        "total_edges": edge_count,
        "skipped_edges": skip_edges,
        "selected_edges": len(selected_edges),
        "interval_count": samples,
        "first_edge_sample_index": selected_edges[0],
        "last_edge_sample_index": selected_edges[-1],
        "first_edge_time_s": selected_edges[0] / sample_rate,
        "last_edge_time_s": selected_edges[-1] / sample_rate,
        "requested_period_ms": period_ms,
        "min_ms": min(intervals) * 1000.0 / sample_rate,
        "max_ms": max(intervals) * 1000.0 / sample_rate,
        "mean_ms": elapsed_ms / samples,
        "jitter_us": (max(intervals) - min(intervals)) * sample_us,
        "elapsed_ms": elapsed_ms,
        "reference_elapsed_ms": samples * period_ms,
        "drift_ms": elapsed_ms - samples * period_ms,
        "time_basis": "sample index / acquisition sample rate; both edge polarities",
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("csv", type=Path)
    parser.add_argument("--sample-rate", type=positive_float, required=True,
                        help="acquisition rate in Hz, e.g. 1000000 (NOT tick rate)")
    parser.add_argument("--samples", type=positive_int, default=500,
                        help="number of toggle intervals (default: 500)")
    parser.add_argument("--period-ms", type=positive_float, default=23.0,
                        help="requested reference period for drift (default: 23 ms)")
    parser.add_argument("--skip-edges", type=nonnegative_int, default=0,
                        help="discard this many initial edges (default: 0)")
    parser.add_argument("--bit", type=nonnegative_int, default=0,
                        help="bit in exported decimal logic values (default: 0)")
    parser.add_argument("--json", action="store_true", help="print JSON instead of a summary")
    args = parser.parse_args()
    try:
        result = analyze(args.csv, args.sample_rate, args.samples, args.period_ms,
                         args.skip_edges, args.bit)
    except (OSError, ValueError, csv.Error) as exc:
        parser.exit(1, f"Error: {exc}\n")

    if args.json:
        print(json.dumps(result, indent=2))
        return

    print(f"CSV: {result['csv']}")
    print(f"Acquisition: {result['sample_rate_hz']:g} Hz; "
          f"resolution {result['sample_resolution_us']:g} us; "
          f"{result['capture_samples']} samples; {result['capture_duration_s']:g} s")
    print(f"Logic bit: {result['logic_bit']}; total edges: {result['total_edges']}")
    print(f"Selection: skip {args.skip_edges} edges, use {args.samples + 1} edges "
          f"for {args.samples} intervals (rising AND falling)")
    print(f"Edge sample indices: {result['first_edge_sample_index']} .. "
          f"{result['last_edge_sample_index']}")
    print(f"Time range: {result['first_edge_time_s']:.9f} .. "
          f"{result['last_edge_time_s']:.9f} s")
    print()
    print("| min (ms) | max (ms) | mean (ms) | jitter (us) | drift (ms) |")
    print("|---|---|---|---|---|")
    print(f"| {result['min_ms']:.6f} | {result['max_ms']:.6f} | "
          f"{result['mean_ms']:.6f} | {result['jitter_us']:.3f} | "
          f"{result['drift_ms']:.6f} |")
    print()
    print(f"Drift reference: {args.samples} x {args.period_ms:g} ms. "
          "Negative means earlier than the requested schedule.")
    print("CSV/capture files are not modified. This tool does not identify firmware mode.")


if __name__ == "__main__":
    main()
