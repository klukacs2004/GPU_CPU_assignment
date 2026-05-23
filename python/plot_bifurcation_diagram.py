from matplotlib.colors import LogNorm
import numpy as np
import matplotlib.pyplot as plt
import argparse
from pathlib import Path


def plot_bifurcation_diagram(filename, args):
    data = np.loadtxt(filename)

    output_dir = Path("results")
    output_dir.mkdir(exist_ok=True)

    output_filename = Path(filename).stem + ".png"
    output_path = output_dir / output_filename

    for j in range(data.shape[1]):
        nonzero_number = np.count_nonzero(data[:, j])
        if nonzero_number > 1:
            data[:, j] *= nonzero_number

    masked_data = np.ma.masked_where(data == 0, data)

    cmap = plt.cm.viridis_r.copy()
    cmap.set_bad(color="white")

    plt.figure(figsize=(9, 6))

    plt.imshow(
        masked_data,
        aspect="auto",
        cmap=cmap,
        extent=[args.r_min, args.r_max, args.x_min, args.x_max],

        #range for the gpu version
        #vmax=50000000,
        #vmin=1000,

        #for the cpu version
        vmax=3000000,
        vmin=10000,
        interpolation="bicubic",
    )

    plt.xlabel("Growth rate r")
    plt.ylabel("Population x")
    plt.title(f"Bifurcation diagram of the logistic map ({args.resolution}x{args.resolution})")

    #plt.colorbar(label="Intensity")
    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.show()

    print(f"Saved plot to: {output_path}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot a bifurcation diagram.")

    parser.add_argument("filename", help="Path to the data file.")

    parser.add_argument("--x_min", type=float, default=0.0)
    parser.add_argument("--x_max", type=float, default=1.0)
    parser.add_argument("--r_min", type=float, default=2.4)
    parser.add_argument("--r_max", type=float, default=4.0)
    parser.add_argument("--resolution", type=int, default=2048)

    args = parser.parse_args()

    plot_bifurcation_diagram(args.filename, args)