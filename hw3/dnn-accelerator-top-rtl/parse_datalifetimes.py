#!/usr/bin/env python3
import re
import csv
import sys
from collections import defaultdict

def parse_gainsight_line(line):
    pattern = re.compile(
        r'^Gainsight\s+'  # Prefix
        r'([\w.]+)\s+'  # Buffer (e.g., conv_tb.conv_inst.ifmap_double_buffer.ram)
        r'(Write|Read)\s+to\s+address\s+(\d+)\s+'  # Operation, Address
        r'at\s+time\s+stamp:\s+(\d+)\s*ns\s+with\s+data\s+(\S+)'  # Timestamp, Data
    )
    match = pattern.match(line.strip())
    if match:
        buffer_str = match.group(1)  # Full buffer path
        operation = match.group(2)   # "Write" or "Read"
        address   = int(match.group(3))  # Address
        timestamp = int(match.group(4))  # Timestamp
        return (buffer_str, operation, address, timestamp)
    return None

def load_and_sort_events(filename):
    events = []
    with open(filename, 'r') as f:
        for line in f:
            if not line.startswith("Gainsight"):
                continue
            parsed = parse_gainsight_line(line)
            if parsed is None:
                continue
            (buffer_str, operation, address, timestamp) = parsed
            events.append((timestamp, operation, buffer_str, address))
    
    # Sort events by timestamp
    events.sort(key=lambda x: x[0])
    return events

def compute_lifetimes(events):
    bufferaddr_events = defaultdict(list)
    
    # 1) Group events by (buffer, address)
    for (ts, op, buffer_str, addr) in events:
        bufferaddr_events[(buffer_str, addr)].append((ts, op))
    
    # 2) Compute lifetimes
    lifetime_map = {}
    for (bufferaddr, ev_list) in bufferaddr_events.items():
        lifetimes = []
        current_write_time = None
        last_read_after_write = None
        
        for (ts, op) in ev_list:
            if op == "Write":
                if current_write_time is not None and last_read_after_write is not None:
                    if last_read_after_write > current_write_time:
                        lifetimes.append(last_read_after_write - current_write_time)
                current_write_time = ts
                last_read_after_write = None
            elif op == "Read":
                if current_write_time is not None and ts > current_write_time:
                    last_read_after_write = ts
        
        # Finalize last write
        if current_write_time is not None and last_read_after_write is not None:
            if last_read_after_write > current_write_time:
                lifetimes.append(last_read_after_write - current_write_time)
        
        lifetime_map[bufferaddr] = lifetimes
    
    return lifetime_map

def main():
    if len(sys.argv) != 3:
        print("Usage: python script.py <log_file> <output_csv>")
        sys.exit(1)
    
    log_file = sys.argv[1]
    events = load_and_sort_events(log_file)
    lifetime_map = compute_lifetimes(events)
    
    sorted_keys = sorted(lifetime_map.keys(), key=lambda x: (x[0], x[1]))
    print("Data Lifetimes (time = last-read - write), grouped by (buffer, address):")
    
    for (buffer, addr) in sorted_keys:
        lifetimes = lifetime_map[(buffer, addr)]
        if lifetimes:
            print(f"Buffer={buffer}, Addr={addr}: lifetimes = {lifetimes}")
    
    # Write to CSV
    csv_filename = sys.argv[2]
    with open(csv_filename, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Buffer", "Address", "Lifetime"])
        for (buffer, addr) in sorted_keys:
            lifetimes = lifetime_map[(buffer, addr)]
            if lifetimes:
                for lt in lifetimes:
                    writer.writerow([buffer, addr, lt])
    
    print(f"\nLifetimes saved to '{csv_filename}'.")

if __name__ == "__main__":
    main()
