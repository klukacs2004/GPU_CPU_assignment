# pragma once

#include <shared/parameters.h>
#include <vector>
#include <chrono>

using namespace std;

//Type alias for the diagram matrix, to make the code more readable 
using DiagramMatrix = std::vector<int>;

//Helper functions for time measurement
auto tmark(){
    return chrono::high_resolution_clock::now();
}

template<typename T1, typename T2>
auto delta_time(T1&& t1, T2&& t2){
    return chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count() / 1e6;
}

inline DiaGenParameters initialize_parameters(T x_min_, T x_max_, T r_min_, T r_max_, int x_axis_, int y_axis_){
    DiaGenParameters params {};

    params.x_axis = x_axis_;
    params.y_axis = y_axis_;

    params.x_min = x_min_;
    params.x_max = x_max_;

    params.r_min = r_min_;
    params.r_max = r_max_;

    params.x_init = 0.25;

    params.stabilization_iterations = 1000;
    params.sample_iterations = 1000000;

    params.x_range_inv = 1.f / (x_max_ - x_min_);
    params.r_step = (r_max_ - r_min_) / (x_axis_ - 1);

    return params;
}







