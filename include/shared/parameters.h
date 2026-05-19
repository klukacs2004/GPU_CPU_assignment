#ifndef PARAMETERS_H
#define PARAMETERS_H

#ifndef T
typedef float T; 
#endif

typedef struct {
    int x_axis;
    int y_axis;

    T x_min;
    T x_max;

    T r_min;
    T r_max;

    T x_init;

    int stabilization_iterations;
    int sample_iterations;

    T x_range_inv;
    T r_step;
} DiaGenParameters;

#endif