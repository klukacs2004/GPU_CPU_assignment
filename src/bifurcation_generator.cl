#include "shared/parameters.h"

#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

#include "bifurcation/logistic_map.h"


__kernel void gpu_generator(__constant DiaGenParameters* params, __global int* diagram_matrix){
    int icol = get_global_id(0);

    if(icol < params->x_axis ){
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
}