"""Ex1: one RAM dump -> queue CSV, graph and a short report summary."""
import argparse
import csv
from pathlib import Path
import struct

ROW = struct.Struct("<III")  # time_ms, raw_depth, filtered_depth


def read_log(path, queue_length=6):
    data = Path(path).read_bytes()
    if not data or len(data) % ROW.size:
        raise ValueError("Dump must contain complete 12-byte QueueLog records")
    rows = list(ROW.iter_unpack(data))
    for i, (time_ms, raw, filtered) in enumerate(rows):
        if max(raw, filtered) > queue_length:
            raise ValueError("Queue depth exceeds queue length")
        if i and time_ms < rows[i - 1][0]:
            raise ValueError("Timestamps must not go backwards")
    return rows


def uart_gaps(path):
    """Only count gaps between valid received lines, handling uint32 wrap."""
    previous = None
    gaps = []
    invalid = duplicates = backwards = 0
    for line in Path(path).read_text(encoding="utf-8-sig", errors="replace").splitlines():
        if not line.strip():
            continue
        try:
            row = [int(x.strip()) for x in line.split(",")]
            if len(row) != 4 or not 0 <= row[0] <= 0xFFFFFFFF:
                raise ValueError()
            if any(not -32768 <= x <= 32767 for x in row[1:]):
                raise ValueError()
        except ValueError:
            invalid += 1
            continue
        seq = row[0]
        if previous is not None:
            delta = (seq - previous) & 0xFFFFFFFF
            if delta == 0:
                duplicates += 1
            elif delta >= 0x80000000:
                backwards += 1
            elif delta > 1:
                gaps.append((previous, seq, delta - 1))
        previous = seq
    return gaps, invalid, duplicates, backwards


def write_csv(path, header, rows):
    with Path(path).open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(header)
        writer.writerows(rows)


def plot(rows, out, name, queue_length, synthetic):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.ticker import MaxNLocator

    fig, axes = plt.subplots(2, 1, sharex=True, figsize=(9, 5), layout="constrained")
    for axis, column, title in zip(axes, (1, 2), ("queue_raw", "queue_filtered")):
        axis.step([r[0] for r in rows], [r[column] for r in rows], where="post", marker=".")
        axis.set(title=title, ylabel="Items", ylim=(-0.2, queue_length + 0.5))
        axis.yaxis.set_major_locator(MaxNLocator(integer=True))
        axis.grid(alpha=0.3)
    axes[-1].set_xlabel("Time from logger start (ms)")
    fig.suptitle(("SYNTHETIC CHECK - " if synthetic else "") + name)
    fig.savefig(out / "queue_fill.png", dpi=180)
    plt.close(fig)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dump", type=Path, help="e.g. measurements/N16.bin")
    parser.add_argument("--uart-log", type=Path)
    parser.add_argument("--queue-length", type=int, default=6)
    parser.add_argument("--no-plots", action="store_true")
    parser.add_argument("--synthetic", action="store_true")
    args = parser.parse_args(argv)
    try:
        if args.queue_length < 1:
            raise ValueError("Queue length must be positive")
        rows = read_log(args.dump, args.queue_length)
        out = args.dump.parent / (args.dump.stem + "_analysis")
        out.mkdir(parents=True, exist_ok=True)
        write_csv(out / "depth.csv", ("time_ms", "raw_depth", "filtered_depth"), rows)
        summary = ["# Ex1 — Queue log", "",
                   "**SYNTHETIC: dữ liệu giả kiểm tra script.**" if args.synthetic else
                   "Nguồn: dump RAM người dùng cung cấp.", "",
                   "| Queue | Đỉnh quan sát | Lần đầu thấy đầy (ms) |",
                   "|---|---:|---:|"]
        for column, name in ((1, "raw"), (2, "filtered")):
            peak = max(r[column] for r in rows)
            first = next((r[0] for r in rows if r[column] == args.queue_length), None)
            summary.append(f"| {name} | {peak} | {first if first is not None else 'Chưa thấy'} |")
        if args.uart_log:
            gaps, invalid, duplicates, backwards = uart_gaps(args.uart_log)
            write_csv(out / "seq_gaps.csv", ("after_seq", "before_seq", "missing"), gaps)
            summary.extend(("", f"- Mẫu thiếu giữa các dòng UART hợp lệ: {sum(r[2] for r in gaps)}.",
                            f"- Dòng không hợp lệ: {invalid}; seq trùng: {duplicates}; "
                            f"seq lùi/reset: {backwards}."))
        summary.extend(("", "- Độ đầy lấy mẫu mỗi 2 ms có thể bỏ sót đỉnh ngắn.",
                        "- Lần đầu thấy đầy là thời điểm quan sát, không phải cạnh đầy chính xác.",
                        "- Seq thiếu chưa phân biệt lỗi firmware với lỗi thu UART; không đếm phần đuôi chưa nhận.",
                        "- Seq liên tục không chứng minh task lấy mẫu đúng chu kỳ.",
                        "- Chép bộ đếm measurement_errors từ Watch vào report riêng.", ""))
        (out / "summary.md").write_text("\n".join(summary), encoding="utf-8")
        if not args.no_plots:
            plot(rows, out, args.dump.stem, args.queue_length, args.synthetic)
        print(f"Saved: {out}")
    except ImportError:
        parser.exit(1, "Plot needs matplotlib. Use the documented uv command or --no-plots.\n")
    except (OSError, ValueError) as error:
        parser.exit(1, f"Error: {error}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
