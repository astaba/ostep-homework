#!/usr/bin/env python
"""
This script orchestrates the execution of relocation.py multiple
times to analyze the success rate of address translation for a 16-byte
block access, sweeping the Limit register in increments of 16 from
the whole virtual memory address space.
"""

# Homework 15.5: What fraction of randomly-generated virtual addresses
# are valid, as a function of the value of the bounds register? Make
# a graph from running with different random seeds, with limit values
# ranging from 0 up to the maximum size of the address space.

import random
import subprocess
import sys
from optparse import OptionParser

import plotly.graph_objects as go
from plotly.offline import plot

# --- Configuration ---
SIMULTION_EXE = "relocation.py"
ADDRESS_SPACE_SIZE = 1024
NUM_ADDRESSES = 100       # Fixed number of virtual addresses to test per run (-n 100)
# Use a step of 16, as requested, for "System Programming Lore"
LIMIT_STEP = 16
LIMIT_RANGE = range(0, ADDRESS_SPACE_SIZE + 1, LIMIT_STEP)

# --- Command Line Parsing ---
parser = OptionParser()
parser.add_option('-s', '--analysis-seed', default=42, help='the random seed for the *overall* analysis sequence (defaults to 42)',
                  action='store', type='int', dest='analysis_seed')
(options, args) = parser.parse_args()

# --- Utility Functions ---

def run_simulation_and_analyze(limit_value, seed):
    """
    Runs the relocation.py script as a subprocess and parses its output.
    Returns the fraction of successful address translations (0.0 to 1.0)
    for a 16-byte access.
    """

    # Construct the command: python relocation.py -s <seed> -n 100 -l
    # <lbounds> -c
    command = [
        sys.executable,
        SIMULTION_EXE,
        '-s', str(seed),
        '-n', str(NUM_ADDRESSES),
        '-l', str(limit_value),
        '-c'
    ]

    try:
        # Run the subprocess, capture stdout, and decode it
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            check=True,
            encoding='utf-8'
        )
    except subprocess.CalledProcessError as e:
        print(f"Error running relocation.py (Limit={limit_value}, Seed={seed}): {e.stderr}", file=sys.stderr)
        return None
    except FileNotFoundError:
        print("Error: 'relocation.py' not found. Ensure it is in the same directory and is executable.", file=sys.stderr)
        sys.exit(1)

    # Analyze the output
    success_count = 0

    # We are looking for lines containing '--> VALID:'
    for line in result.stdout.splitlines():
        if '--> VALID:' in line:
            success_count += 1

    # The fraction is (Successful Translations / Total Addresses
    # Generated)
    return success_count / float(NUM_ADDRESSES)


def main():
    print("--- Running Relocation Analysis Simulation (16-byte Access, Plotly Output) ---")
    print(f"Total Addresses per Run (-n): {NUM_ADDRESSES}")
    print(f"Address Space Size (asize): {ADDRESS_SPACE_SIZE} bytes")
    print(f"Limit Register (-l) Range: 0 to {ADDRESS_SPACE_SIZE} (Step: {LIMIT_STEP})")
    print("-" * 60)

    limits = []
    fractions = []

    random.seed(options.analysis_seed) # Fixed seed for overall reproducibility

    for lbounds in LIMIT_RANGE:
        seed = random.randint(1000, 100000)
        fraction = run_simulation_and_analyze(lbounds, seed)

        if fraction is not None:
            limits.append(lbounds)
            fractions.append(fraction)
            print(f"Limit (-l={lbounds:4d}) | Seed (-s={seed:6d}) | Success Rate: {fraction * 100.0:.1f}%")
        else:
            print(f"Limit (-l={lbounds:4d}) | Simulation Failed, skipping point.")


    print("-" * 60)
    print("Analysis complete. Generating Plotly results...")

    # Convert fractions to percentages for plotting
    percentages = [int(f * 100) for f in fractions]

    # --- Plotting with Plotly ---

    fig = go.Figure()

    # Add the main trace
    fig.add_trace(go.Scatter(
        x=limits,
        y=percentages,
        mode='lines+markers',
        name='16-byte Block Success Rate',
        marker=dict(size=6, color='#C85C50'),
        line=dict(width=3, shape='linear')
    ))

    # Add reference lines (ASIZE Boundary and Minimum Limit)
    fig.add_vline(
        x=ADDRESS_SPACE_SIZE,
        line_dash="dash",
        line_color="red",
        annotation_text=f"ASIZE Boundary ({ADDRESS_SPACE_SIZE})",
        annotation_position="top right"
    )

    fig.add_vline(
        x=LIMIT_STEP,
        line_dash="dot",
        line_color="green",
        annotation_text=f"Min Limit for 16-byte Access ({LIMIT_STEP})",
        annotation_position="bottom right"
    )

    title_line_1 = f'<b>Analysis Seed: {options.analysis_seed}, Runs: {SIMULTION_EXE} -s <ever_diff> -n {NUM_ADDRESSES} -l <0 -- {ADDRESS_SPACE_SIZE}> -c</b>'
    title_line_2 = f'Success Rate of 16-byte Access vs. Limit Register Size (Segment Granularity)'
    title = f'{title_line_1}<br>{title_line_2}'

    # Update layout for aesthetics and clarity
    fig.update_layout(
        title={
            'text': title,
            'x': 0.5,
            'xanchor': 'center'
        },
        xaxis_title=f'<b>Limit Register Value (-l)</b><br>Ranging from 0 to {ADDRESS_SPACE_SIZE} - Stepped by {LIMIT_STEP} bytes',
        yaxis_title='Percentage of 16-byte Blocks Within Bounds (%)',
        yaxis=dict(
            tickmode='linear',
            dtick=10,
            range=[0, 100],
            gridcolor='#e0e0e0'
        ),
        xaxis=dict(
            gridcolor='#e0e0e0'
        ),
        template='plotly_white', # Clean, professional theme
        hovermode="x unified"
    )

    # The plot is generated in an interactive HTML file which opens in
    # the browser.
    print("\n--- Plotly HTML file generated. Check your output viewer. ---")
    fig.show()


if __name__ == '__main__':
    main()
