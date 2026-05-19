#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <iomanip>

#include <shared/parameters.h>
#include <cpu/generator.hpp>

#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_MINIMUM_OPENCL_VERSION 200
#define CL_HPP_ENABLE_EXCEPTIONS 1

#include <CL/opencl.hpp>

using namespace std;

int main(int argc, char* argv[]){

    //Output directory for the generated files 
    const filesystem::path output_dir = "data";

    //Set the default parameter values
    T x_min = 0.0; 
    T x_max = 1.0; 
    T r_min = 2.5; 
    T r_max = 4.0; 
    int nx = 2048; 
    int ny = 2048; 

    //Parse command line arguments in vars
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            cout <<
                R"(Usage:
                bifurcation_generator_CPU.exe [options]

                Options:
                --xmin <float>       Minimum x value
                --xmax <float>       Maximum x value
                --rmin <float>       Minimum r value
                --rmax <float>       Maximum r value
                --nx <int>           Horizontal resolution
                --ny <int>           Vertical resolution

                Example:
                .\build\bifurcation_generator_CPU.exe ^
                    --xmin 0.0 ^
                    --xmax 1.0 ^
                    --rmin 2.5 ^
                    --rmax 4.0 ^
                    --nx 2048 ^
                    --ny 2048 ^
                )"
            << endl;

            return 0;
        }

        if (arg == "--xmin") x_min = stof(argv[++i]);
        else if (arg == "--xmax") x_max = stof(argv[++i]);
        else if (arg == "--rmin") r_min = stof(argv[++i]);
        else if (arg == "--rmax") r_max = stof(argv[++i]);
        else if (arg == "--nx") nx = stoi(argv[++i]);
        else if (arg == "--ny") ny = stoi(argv[++i]);
        else {
            cerr << "Unknown or incomplete argument: " << arg << "\n";
            cerr << "Use --help for usage information.\n";
            return 1;
        }
    }

    // Set the parsed parameters
    DiaGenParameters params =  initialize_parameters(x_min, x_max, r_min, r_max, nx, ny);

    //Generate filenames for the output file names
    const filesystem::path time_measurements_filename = "bifurcation_runtimes_" + to_string(params.y_axis) + "x" + to_string(params.x_axis) + "_GPU.txt";
    const filesystem::path diagram_filename = "bifurcation_diagram_" + to_string(params.y_axis) + "x" + to_string(params.x_axis) + "_GPU.txt";

    //Preallocate the diagram matrix for better cache performance
    vector<int> diagram_matrix(params.x_axis * params.y_axis, 0);


    // Run CPU computation
    auto t0 = tmark();
    cpu_generator(0, params.x_axis, params, diagram_matrix);
    auto t1 = tmark();
    auto t_cpu = delta_time(t0, t1);

    try{
        // Create and extract platforms provided by the computer
        vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        // Find GPU devices
        cl::Platform selected_platform{};
        cl::Device selected_device{};
        bool found_gpu_device = false;

        // Loop through platforms to find that of the device 0
        for(size_t i=0; i<platforms.size(); ++i)
        {
            vector<cl::Device> devices;
            platforms[i].getDevices(CL_DEVICE_TYPE_GPU, &devices);

            if(devices.size() == 0){ continue; }

            for(size_t j = 0; j<devices.size(); ++j){
                if(j==0){
                    selected_platform = platforms[i];
                    selected_device = devices[j];
                    found_gpu_device = true;
                }
            }

            // skip other platforms if a GPU device is found
            if(found_gpu_device){ break; }
        }

        cout << "Selected platform vendor: " << selected_platform.getInfo<CL_PLATFORM_VENDOR>() << "\n";
        cout << "Selected platform name:   " << selected_platform.getInfo<CL_PLATFORM_NAME>() << "\n";
        cout << "Selected device name:     " << selected_device.getInfo<CL_DEVICE_NAME>() << "\n";

        // Create context and command queue
        vector<cl_context_properties>cps{ CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(selected_platform()), 0};
        cl::Context context{ selected_device, cps.data() };

        // Enable profiling on the queue:
        cl::QueueProperties qps{cl::QueueProperties::Profiling};
        cl::CommandQueue queue{ context, selected_device, qps};

        // Load and compile kernel program:
        ifstream source{"src/gpu/bifurcation_generator.cl"};
        if( !source.is_open() ){ throw runtime_error{ string{"Error opening kernel file: bifurcation_generator.cl"} }; }
        string source_string{ istreambuf_iterator<char>{ source },
                                   istreambuf_iterator<char>{} };
        cl::Program program{ context, source_string };

        string build_option;
        if     ( is_same_v<T, float> ){ build_option = "-DT=float";}
        else if( is_same_v<T, double> ){ build_option = "-DT=double";}
        program.build({selected_device}, build_option.c_str());

        auto bifurcation_kernel = cl::KernelFunctor<cl::Buffer, cl::Buffer>(program, "gpu_generator2");

        // Allocate and setup data buffers:
        cl::Buffer buffer_params(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(DiaGenParameters), &params);
        cl::Buffer buffer_diagram{context, begin(diagram_matrix), end(diagram_matrix), false};  //read and write

        // Write the time data to a file
        ofstream info_file(output_dir / time_measurements_filename);

        info_file << "-----------------------------\n";
        info_file << "Selected platform vendor: " << selected_platform.getInfo<CL_PLATFORM_VENDOR>() << "\n";
        info_file << "Selected platform name: " << selected_platform.getInfo<CL_PLATFORM_NAME>() << "\n";
        info_file << "Selected device name: " << selected_device.getInfo<CL_DEVICE_NAME>() << "\n";
        info_file << "Max work-group size: " << selected_device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << "\n";
        info_file << "-----------------------------\n\n";

        auto max_items = selected_device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
        info_file << "Max work-item sizes: "
                << max_items[0] << " "
                << max_items[1] << " "
                << max_items[2] << "\n\n";

        info_file << "CPU time [ms]: " << t_cpu << "\n";
        std::cout << "CPU time: " << t_cpu << " ms\n";

        info_file << left << setw(22) << "Workers_per_column" << " | " << setw(22) << "GPU kernel time [ms]" << "\n";

        info_file << string(22, '-') << "-+-" << string(22, '-') << "\n";

        // Loop through the work-group sizes  
        for(int iexp=0; iexp < 10; ++iexp){
            fill(diagram_matrix.begin(), diagram_matrix.end(), 0);

            int workers_per_column = 2 << iexp;
            cl::NDRange global_size{static_cast<size_t>(params.x_axis), static_cast<size_t>(workers_per_column)};
            cl::NDRange local_size{1, static_cast<size_t>(workers_per_column)};
            // Extract grid size and run the warm up kernel
            //cl::NDRange grid_size = cl::NDRange{ static_cast<size_t>(params.x_axis)};
            bifurcation_kernel(cl::EnqueueArgs{queue, global_size, local_size}, buffer_params, buffer_diagram);
            
            // Synchronize:
            queue.finish();

            // Measured kernel launches:
            cl::Event ev = bifurcation_kernel(cl::EnqueueArgs{queue,global_size, local_size}, buffer_params, buffer_diagram);

            // Copy back results:
            std::vector<cl::Event> vev{ev};
            cl::Event read_ev;
            queue.enqueueReadBuffer(buffer_diagram, CL_FALSE, 0, diagram_matrix.size() * sizeof(int), diagram_matrix.data(), &vev, &read_ev);
            read_ev.wait();

            // Kernel-inly GPU time
            cl_ulong t_0 = ev.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            cl_ulong t_1 = ev.getProfilingInfo<CL_PROFILING_COMMAND_END>();
            auto t_gpu_kernel = (t_1-t_0)*1e-6;

            //Full GPU time: kernel+read
            //cl_ulong t_tot = read_ev.getProfilingInfo<CL_PROFILING_COMMAND_END>();
            //auto t_gpu_tot = (t_tot-t_0)*1e-6;

            std::cout << "GPU time: " << t_gpu_kernel << " ms\n";

            info_file << right << setw(22) << workers_per_column << " | " << setw(22) << fixed << setprecision(3) << t_gpu_kernel << "\n";

            // Record the "fastes" diagram matrix
            if(iexp == 6){
                cout << "Writing bifurcation diagram data to " << diagram_filename << " file:\n";
                ofstream output_file(output_dir / diagram_filename);

                for (int j = params.y_axis - 1; j >= 0; j--) {
                    for (int i = 0; i < params.x_axis; i++) {
                        output_file << diagram_matrix[i * params.y_axis + j] << " ";
                    }
                    output_file << "\n";
                }

                output_file.close();
            }
        }

        info_file.close();

    }
    catch(cl::BuildError& error){
        cout << "Build failed. Log:\n";
        for (const auto& log : error.getBuildLog()){
            cout << log.second; 
        }
        return -1;
    }
    catch(cl::Error& e){
        cout << "OpenCL error: " << e.what() << "\n";
        return -1;
    }
    catch(exception& e){
        cerr << "C++ STL error: " << e.what() << "\n";
        return -1; 
    }

    return 0;
}