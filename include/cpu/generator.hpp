#pragma once

#include "bifurcation/logistic_map.h"
#include "parameters.hpp"

void cpu_generator(int i_start, int i_end, const DiaGenParameters& params, DiagramMatrix& diagram_matrix) {
    for (int i = i_start; i < i_end; i++) {
        float r = params.r_min + params.r_step * i;           //calculate r value for this column
        float x = params.x_init;                    //initialize x to the same for each r

        //Stablization loop to let the system reach the attractor before sampling
        for (int k = 0; k < params.stabilization_iterations; k++) {
            x = logistic_map(x, r);
        }

        //Sampling loop to populate the diagram matrix
        for (int k = 0; k < params.sample_iterations; k++) {
            x = logistic_map(x, r);

            //Check if x is within the specified range 
            if (x >= params.x_min && x <= params.x_max) {
                int j = static_cast<int>( (x - params.x_min) * params.x_range_inv * (params.y_axis -1));
                
                // column-major order to improve cache performance when iterating over r values
                diagram_matrix[i * params.y_axis + j]++; 
            }
        }
    }
}