#include "shared/parameters.h"

#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

#include "bifurcation/logistic_map.h"

// Naive version
__kernel void gpu_generator(__constant DiaGenParameters* params, __global int* diagram_matrix){
    int icol = get_global_id(0);

    if (icol >= params->x_axis) return;

    T r = params->r_min + params->r_step * icol;
    T x = params->x_init;

    for (int k = 0; k < params->stabilization_iterations; k++) {
        x = logistic_map(x, r);
    }

    for (int k = 0; k < params->sample_iterations; k++) {
        x = logistic_map(x, r);

        if (x >= params->x_min && x <= params->x_max) {
            int j = (int)((x - params->x_min) * params->x_range_inv * (params->y_axis - 1));

            diagram_matrix[icol * params->y_axis + j]++;
        }
    }  
}

// Optimized version
__kernel void gpu_generator2(__constant DiaGenParameters* params, __global int* diagram_matrix){
    int icol   = get_global_id(0); // oszlop
    int worker = get_global_id(1); // worker az oszlopon belül

    int workers_per_column = get_global_size(1);

    T r = params->r_min + params->r_step * icol;

    // Every worker begin from different initial state
    T x = params->x_init + (T)(worker + 1) * (T)0.000001;

    // Stabilization
    for (int k = 0; k < params->stabilization_iterations; k++) {
        x = logistic_map(x, r);
    }

    // Distribute the iterations among worker-items 
    int samples_per_worker = params->sample_iterations / workers_per_column;

    //this is the current coulmn (a given r-value) 
    int base = icol * params->y_axis;

    for (int k = 0; k < samples_per_worker; k++) {
        x = logistic_map(x, r);

        if (x >= params->x_min && x <= params->x_max) {
            int j = (int)((x - params->x_min) * params->x_range_inv * (params->y_axis - 1));

            atomic_inc(&diagram_matrix[base + j]);
        }
    }
}