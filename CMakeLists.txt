cmake_minimum_required(VERSION 3.20)

project(GPU_CPU_assignment LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCL REQUIRED)

add_executable(bifurcation_generator_GPU
    src/bifurcation_generator_GPU.cpp
)

target_link_libraries(bifurcation_generator_GPU PRIVATE OpenCL::OpenCL)
target_include_directories(bifurcation_generator_GPU PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)