#!/usr/bin/env python3

"""
Orchestrates the execution of the TLB microbenchmark and generates
a logarithmic visualization of the results, ensuring the X-axis
utilizes the full width of the plot.
"""

import math
import os
import subprocess
import sys

import plotly.graph_objects as go

# --- Configuration ---
# Strategic sampling to catch L1 (usually 64) and L2 (usually 512/1024) TLB drops
PAGE_COUNTS = [
    1,
    2,
    4,
    8,
    16,
    32,
    48,
    64,
    80,
    96,
    128,
    256,
    384,
    512,
    640,
    768,
    896,
    1024,
    1536,
    2048,
    3072,
    4096,
    6144,
    8192,
    12288,
    16384,
    32768,
    65536,
    131072,
    163840,
    229376,
    294912,
    327680,
    393216,
    458752,
    589824,
]

BINARY = "./tlb"
CSV_FILE = "tlb_results.csv"


def run_benchmark():
    """Executes the C binary for different page counts and logs to CSV."""
    if not os.path.exists(BINARY):
        print(f"Error: {BINARY} not found. Compile it first!")
        sys.exit(1)
    # Presentation Table header
    print(f"{'Pages':<10} | {'Trials':<12} | {'Time (ns)':<10}")
    print("-" * 40)

    results = []

    for pages in PAGE_COUNTS:
        # High trials for low pages to overcome clock resolution;
        # lower trials for high pages to save time.
        if pages <= 64:
            trials = 1000000
        elif pages <= 1024:
            trials = 100000
        elif pages <= 16384:
            trials = 10000
        else:
            trials = 1000

        try:
            # Pin to Core 0 for consistency
            # cmd = ["taskset", "-c", "0", BINARY, str(pages), str(trials)]
            cmd = [BINARY, str(pages), str(trials)]
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)

            output = result.stdout.strip().split()
            if output:
                latency = float(output[-1])
                results.append((pages, latency))
                print(f"{pages:<10} | {trials:<12} | {latency:<10.2f}")
        except subprocess.CalledProcessError as e:
            print(f"Error running pages={pages}: {e}")

    # Write to CSV
    with open(CSV_FILE, "w") as f:
        f.write("Pages,Latency_ns\n")
        for p, ltc in results:
            f.write(f"{p},{ltc}\n")

    return results


def plot_results(data):
    """Generates a graph with a properly scaled logarithmic X-axis."""
    pages = [d[0] for d in data]
    latencies = [d[1] for d in data]

    fig = go.Figure()

    # Main Latency Trace
    fig.add_trace(
        go.Scatter(
            x=pages,
            y=latencies,
            mode="lines+markers",
            name="TLB Access Latency",
            line=dict(color="#ef4444", width=3),
            marker=dict(size=10, color="#1e293b", line=dict(width=1, color="white")),
            hovertemplate="<b>Pages:</b> %{x}<br><b>Latency:</b> %{y:.2f} ns<extra></extra>",
        )
    )

    # Calculate log range for the X-axis (log10 is the internal format for Plotly log axes)
    low_log = math.log10(pages[0])
    high_log = math.log10(pages[-1])
    # Add a little padding (5%) so markers aren't cut off at the edges
    padding = (high_log - low_log) * 0.05
    # Clean up the tick marks
    max_pow = math.ceil(math.log2(max(pages)))
    clean_ticks = [2**i for i in range(max_pow + 1)]

    fig.update_layout(
        title={
            "text": "<b>TLB Access Latency vs. Working Set Size</b>",
            "x": 0.5,
            "xanchor": "center",
            "font": {"size": 20},
        },
        xaxis=dict(
            title="Number of Pages (Log Scale)",
            type="log",
            tickmode="array",
            # tickvals=pages,
            # ticktext=[str(p) for p in pages],
            tickvals=clean_ticks,
            ticktext=[str(i) for i in clean_ticks],
            range=[low_log - padding, high_log + padding],
            # range=[low_log, high_log],
            gridcolor="#e5e7eb",
            zeroline=False,
        ),
        yaxis=dict(
            title="Latency (nanoseconds)",
            gridcolor="#e5e7eb",
            zeroline=True,
            zerolinecolor="#9ca3af",
            rangemode="tozero",
        ),
        template="plotly_white",
        # margin=dict(l=60, r=40, t=80, b=60),
        # width=1100,
        # height=600,
        hovermode="x",
    )

    # Architectural Zones based on your data and hardware specs
    zones = [
        (1, 64, "L1 TLB Hit", "rgba(34, 197, 94, 0.1)"),
        (64, 512, "L2 TLB Hit", "rgba(59, 130, 246, 0.1)"),
        (512, 1024, "L2 Walk", "rgba(249, 115, 22, 0.1)"),
        (1024, max(pages), "L3 / Pre-fetcher", "rgba(239, 68, 68, 0.1)"),
    ]

    for start, end, label, color in zones:
        fig.add_vrect(
            x0=start,
            x1=end,
            fillcolor=color,
            # opacity=0.08,
            layer="below",
            line_width=0,
            annotation_text=label,
            annotation_position="top left",
            annotation_font_size=12,
            annotation_font_color="black",
        )

    print("\nAnalysis complete. Opening interactive plot...")
    # fig.write_html("tlb_analysis.html")
    fig.show()


if __name__ == "__main__":
    # If CSV exists, you could optionally skip benchmarking and just plot
    # For now, we run fresh
    results = run_benchmark()
    if results:
        plot_results(results)
