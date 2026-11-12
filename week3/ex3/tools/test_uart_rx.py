"""Send numbered Ex3 commands while collecting UART replies concurrently.

Install: python -m pip install pyserial
Example: python test_uart_rx.py --port COM5 --count 100 --interval-ms 20
No serial port is opened when this module is imported.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import queue
import re
import threading
import time


def summarize(expected, lines):
    """A reply counts only if both sequence and echoed command match."""
    replies = Counter(line[3:] for line in lines if line.startswith("OK "))
    expected_set = set(expected)
    correct = [cmd for cmd in expected if replies[cmd] > 0]
    return {
        "sent": len(expected),
        "confirmed": len(correct),
        "missing_sequences": [int(cmd.rsplit("#", 1)[1])
                              for cmd in expected if replies[cmd] == 0],
        "duplicate_replies": sum(max(0, replies[cmd] - 1) for cmd in expected),
        "unexpected_replies": [cmd for cmd in replies if cmd not in expected_set],
        "error_replies": [line for line in lines if line.startswith("ERR ")],
    }


class Reader:
    def __init__(self, port):
        self.port = port
        self.lines = queue.Queue()
        self.stop = threading.Event()
        self.error = None
        self.thread = threading.Thread(target=self.run, daemon=True)

    def run(self):
        pending = bytearray()
        try:
            while not self.stop.is_set():
                pending.extend(self.port.read(self.port.in_waiting or 1))
                while b"\n" in pending:
                    line, _, pending = pending.partition(b"\n")
                    self.lines.put(line.decode("ascii", errors="replace").strip())
                if len(pending) > 4096:
                    raise RuntimeError("UART reply exceeds 4096 bytes without newline")
        except Exception as exc:
            self.error = str(exc)

    def collect(self, duration):
        result = []
        deadline = time.monotonic() + duration
        while time.monotonic() < deadline:
            try:
                result.append(self.lines.get(timeout=min(0.05, max(0.001, deadline - time.monotonic()))))
            except queue.Empty:
                if self.error:
                    raise RuntimeError(self.error)
        return result


def get_stats(port, reader, attempts=3):
    """Query-only protocol command; no demo-task side effects."""
    for _ in range(attempts):
        port.write(b"?\r")
        port.flush()
        lines = reader.collect(0.5)
        stats = {}
        for line in lines:
            if line.startswith("ST "):
                stats.update({key: int(value) for key, value in
                              re.findall(r"(\w+)=(\d+)", line)})
        if {"rx", "drop", "uart", "rearm", "ok", "bad", "long",
            "txdrop", "txerr", "buf"} <= stats.keys():
            return stats
    raise RuntimeError("No complete ST reply. Check firmware, wiring, baud and READY state.")


def run(args):
    try:
        import serial
    except ImportError as exc:
        raise RuntimeError("Install pyserial: python -m pip install pyserial") from exc

    expected = [f"{args.command}#{i}" for i in range(1, args.count + 1)]
    if max(map(len, expected)) > 31:
        raise ValueError("Numbered command exceeds 31 payload bytes (MAX_CMD_LEN - 1)")

    with serial.Serial(args.port, args.baud, timeout=0.05, write_timeout=5) as port:
        # Allow boot if the USB adapter/reset wiring resets the board on open.
        time.sleep(args.boot_wait)
        port.reset_input_buffer()
        reader = Reader(port)
        reader.thread.start()
        try:
            before = get_stats(port, reader)
            started = time.monotonic()
            for command in expected:
                payload = (command + "\r").encode("ascii")
                if port.write(payload) != len(payload):
                    raise RuntimeError("Incomplete host serial write")
                if args.interval_ms:
                    time.sleep(args.interval_ms / 1000)
            port.flush()  # Wait for the host's transmit queue to drain.
            # Replies are collected concurrently by Reader even while sending.
            wire_seconds = sum(len(c) + 1 for c in expected) * 10 / args.baud
            lines = reader.collect(max(args.drain_seconds, wire_seconds * 2))
            elapsed = time.monotonic() - started
            after = get_stats(port, reader)
        finally:
            reader.stop.set()
            reader.thread.join(timeout=1)

    delta = {key: (after[key] - before[key]) & 0xFFFFFFFF
             for key in before if key != "buf"}
    result = summarize(expected, lines)
    result.update({"port": args.port, "baud": args.baud, "command": args.command,
                   "interval_ms": args.interval_ms, "elapsed_seconds": elapsed,
                   "stream_buffer_bytes": after["buf"], "before": before,
                   "after": after, "delta": delta,
                   "expected_command_bytes": sum(len(c) + 1 for c in expected),
                   "note": "delta.rx includes 2 bytes for the final ?\\r query. "
                           "Unconfirmed commands do not by themselves prove RX byte loss.",
                   "replies": lines})
    clean = (result["confirmed"] == args.count
             and not result["duplicate_replies"] and not result["unexpected_replies"]
             and not result["error_replies"]
             and delta["ok"] == args.count
             and all(delta[k] == 0 for k in ("drop", "uart", "rearm", "bad", "long", "txdrop", "txerr"))
             and delta["rx"] == result["expected_command_bytes"] + 2)
    result["clean_run"] = clean
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(f"Sent: {args.count}; confirmed: {result['confirmed']}; clean: {clean}")
    print(f"Missing: {result['missing_sequences']}")
    print(f"Counter delta: {delta}")
    print(f"Saved: {args.output}")
    return 0 if clean else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--count", type=int, default=100)
    parser.add_argument("--command", default="P10", help="Base P/S/T command, without CR or #")
    parser.add_argument("--interval-ms", type=float, default=20)
    parser.add_argument("--boot-wait", type=float, default=1)
    parser.add_argument("--drain-seconds", type=float, default=2)
    parser.add_argument("--output", type=Path, default=Path("uart_rx_result.json"))
    args = parser.parse_args()
    if not 1 <= args.count <= 100000 or args.baud <= 0:
        parser.error("count must be 1..100000 and baud must be positive")
    if min(args.interval_ms, args.boot_wait, args.drain_seconds) < 0:
        parser.error("time intervals cannot be negative")
    if not re.fullmatch(r"(?:P[1-9][0-9]*|S[0-4]|T[01])", args.command):
        parser.error("command must be P<positive ms>, S0..S4, T0 or T1")
    try:
        return run(args)
    except (OSError, ValueError, RuntimeError) as exc:
        parser.exit(2, f"Error: {exc}\n")


if __name__ == "__main__":
    raise SystemExit(main())
