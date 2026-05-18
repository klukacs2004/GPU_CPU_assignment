typedef struct {
    int x_axis;
    int y_axis;

    float x_min;
    float x_max;

    float r_min;
    float r_max;

    float x_init;

    int stabilization_iterations;
    int sample_iterations;

    float x_range_inv;
    float r_step;
} DiaGenParameters;

float logistic_map(float x, float r) {
    return r * x * (1.0f - x);
}
__kernel void calculate_columns(__constant DiaGenParameters* params, __global int* diagram_matrix, int resolution){
    //int thx = get_global_id(0);
    int icol = get_global_id(0);

    if(icol < params->x_axis ){
        float r = params->r_min + params->r_step * icol;
        float x = params->x_init;

        for (int k = 0; k < params->stabilization_iterations; k++) {
            x = logistic_map(x, r);
        }
        for (int k = 0; k < params->sample_iterations; k++) {
            x = logistic_map(x, r);

            //Check if x is within the specified range 
            if (x >= params->x_min && x <= params->x_max) {
                int j = (int)( (x - params->x_min) * params->x_range_inv * (params->y_axis -1));
                
                // column-major order to improve cache performance when iterating over r values
                diagram_matrix[icol * params->y_axis + j]++; 
            }
        }
        
    }   
}