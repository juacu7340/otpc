import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import argparse

# Set up argument parser
parser = argparse.ArgumentParser(description='Plot benchmark times from a CSV file.')
parser.add_argument('csv_file', type=str, help='Path to the CSV file containing benchmark times')
args = parser.parse_args()

# Read the CSV file
csv_file = args.csv_file
df = pd.read_csv(csv_file, header=None)

# Assume that each row represents a function and each column represents a benchmark time for a different run/scenario
# The first column contains the function names
function_labels = df.iloc[:, 0].tolist()  # Get function names from the first column
benchmark_times = df.iloc[:, 1:]  # The rest are benchmark times

# Calculate average times
average_times = benchmark_times.mean(axis=1)

# Append average times to function labels
function_labels_with_avg = [f'{func} (avg: {avg:.2f})' for func, avg in zip(function_labels, average_times)]

num_functions = benchmark_times.shape[0]
num_runs = benchmark_times.shape[1]

# Generate run labels
run_labels = [f'Run {i+1}' for i in range(num_runs)]

# Plotting the data with lines and dots
plt.figure(figsize=(12, 8))

for i in range(num_functions):
    plt.plot(run_labels, benchmark_times.iloc[i], marker='o', label=function_labels_with_avg[i])

# Adding titles and labels
plt.title('Benchmark Times')
plt.xlabel('Run')
plt.ylabel('Time (seconds)')
plt.legend()

# Display the plot
plt.tight_layout()
plt.show()
