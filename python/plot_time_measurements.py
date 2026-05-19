import numpy as np
import matplotlib.pyplot as plt
import argparse
from pathlib import Path

def plot_time_measurement(filename):
    data = np.loadtxt(filename) / 1e6

    output_dir = Path("results")
    output_dir.mkdir(exist_ok=True)
    output_filename = Path(filename).stem + ".png"

    plt.hist(data, bins=20, edgecolor='black')

    plt.xlabel('Time (seconds)')
    plt.ylabel('Frequency')
    plt.title('Time Measurement Distribution')

    plt.tight_layout()
    plt.savefig(output_dir / output_filename)
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot time measurement data.")
    parser.add_argument("filename", help="Path to the time measurement file.")
    args = parser.parse_args()

    plot_time_measurement(args.filename)