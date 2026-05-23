cmake_minimum_required(VERSION 3.20)

project(GPU_CPU_assignment LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCL REQUIRED)

add_executable(bifurcation_generator_GPU
    src/gpu/bifurcation_generator_GPU.cpp
)

add_executable(bifurcation_generator_CPU
    src/cpu/bifurcation_generator_CPU.cpp
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MSVC)
    target_compile_options(bifurcation_generator_GPU PRIVATE /clang:-O3 /W4 /fp:fast /arch:AVX2)
    target_compile_options(bifurcation_generator_CPU PRIVATE /clang:-O3 /W4 /fp:fast /arch:AVX2)

elseif(MSVC)
    target_compile_options(bifurcation_generator_GPU PRIVATE /O2 /W4 /fp:fast)
    target_compile_options(bifurcation_generator_CPU PRIVATE /O2 /W4 /fp:fast)

else()
    target_compile_options(bifurcation_generator_GPU PRIVATE -O3 -Wall -Wextra -march=native -ffast-math)
    target_compile_options(bifurcation_generator_CPU PRIVATE -O3 -Wall -Wextra -march=native -ffast-math)
endif()

target_link_libraries(bifurcation_generator_GPU PRIVATE OpenCL::OpenCL)
target_include_directories(bifurcation_generator_GPU PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)