
# GPU CPU Assignment

## Overview

This project generates and visualizes the bifurcation diagram of the logistic map:

```math
x_{n+1} = r x_n (1 - x_n
```

The main computation is written in C++, while the plotting and visualization are done in Python. 


## Project structure

```bash
GPU_CPU_ASSIGNMENT/
├── include/
│   ├── bifurcation/
│   │   └── logistic_map.h
│   ├── cpu/
│   │   ├── generator.hpp
│   │   └── logistic_map.hpp
│   └── shared/
│       └── parameters.h
├── src/
│   ├── cpu/
│   │   └── bifurcation_generator_CPU.cpp
│   └── gpu/
│       ├── bifurcation_generator_GPU.cpp
│       └── bifurcation_generator.cl
├── python/
│   ├── plot_bifurcation_diagram.py
│   └── plot_time_measurements.py
├── data/
│   └── generated .txt files
├── results/
│   └── generated .png files
├── CMakeLists.txt
└── README.md
```

## Requirements

- CMake
- Ninja
- Clang or another C++ compiler
- Python 3
- NumPy
- Matplotlib

## Build

Configure the project:
```bash
cmake -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++
```
Build the executable:
```bash
cmake --build build
```
On Windows, I used the Clang compiler with the following flags:
```shell
/O2 /W4 /fp:fast /arch:AVX2
```
For convenience, I also included a CMakeLists.txt file that supports both Linux and Windows builds.
This is also necessary for the GPU implementation due to the OpenCL C style.

The kernel implementation is contained by the .cl file in the src/gpu/ directory, and the host side is .cpp file here. 

## Run

Run the CPU bifurcation generator:
```bash
.\build\bifurcation_generator_CPU.exe --xmin 0.0 --xmax 1.0 --rmin 2.5 --rmax 4.0 --nx 2048 --ny 2048   
```
Run the CPU bifurcation generator:
```bash
.\build\bifurcation_generator_GPU.exe --xmin 0.0 --xmax 1.0 --rmin 2.5 --rmax 4.0 --nx 2048 --ny 2048   
```
Here, the first two values define the lower and upper bounds of the population variable \(x\).  The next two values specify the lower and upper bounds of the parameter \(r\).  The last value sets the resolution of the generated image.

The command above produces a bifurcation diagram with a resolution of 2048×2048 pixels.

To see all available command-line options, run:
```bash
.\build\bifurcation_generator_CPU.exe --help
```

## Plot the Bifurcation Diagram
```bash
python .\python\plot_bifurcation_diagram.py data\bifurcation_diagram_2048x2048_CPU.txt --x_min 0.0 --x_max 1.0 --r_min 2.5 --r_max 4.0 --resolution 2048
```
```bash
python .\python\plot_bifurcation_diagram.py data\bifurcation_diagram_2048x2048_GPU.txt --x_min 0.0 --x_max 1.0 --r_min 2.5 --r_max 4.0 --resolution 2048
```

## Plot Runtime Measurement
```bash
python .\python\plot_time_measurements.py data\bifurcation_runtimes_2048x2048_CPU.txt 
```
## Reference

The implementation of the bifurcation diagram is based on the description available at:

https://commons.wikimedia.org/wiki/File:Logistic_Map_Bifurcation_Diagram,_Matplotlib.svg

## Notes

The main code performs 200 runtime measurements by default.  This value can be modified in the C++ source code. 