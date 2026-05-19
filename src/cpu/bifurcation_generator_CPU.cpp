#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include <filesystem>

#include <thread>
#include <future>

using namespace std;

//Helper functions for time measurement
auto tmark(){
    return chrono::high_resolution_clock::now();
}

template<typename T1, typename T2>
auto delta_time(T1&& t1, T2&& t2){
    return chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count() / 1000.0;
}

//Function to compute the logistic map, defined as f(x) = r * x * (1 - x)
//Defined as a template to allow for potential future use with different floating point types (double, float)
template<typename FloatType>
FloatType logistic_map(FloatType x, FloatType r) {
    return r * x * (1 - x);
}

using T = float;

//Struct to hold the parameters for the bifurcation diagram generation
struct DiaGenParameters {
    //Default parameters for the bifurcation diagram generation, can be overridden by command line arguments
    int x_axis = 1000;
    int y_axis = 1000; 
    T x_min = 0.0f;
    T x_max = 1.0f;
    T r_min = 2.5;
    T r_max = 4.0;
    T x_init = 0.25f;   

    //Contants for stabilization and sampling iteration according to the wikipedia description of the bifurcation diagram calculation
    const int stabilization_iterations = 1000;
    const int sample_iterations = 1'000'000;

    T x_range_inv;
    T r_step;

    // Define  a function to avoid duplicated initialization
    private:
        void initialize_derived_parameters() {
            x_range_inv = 1.0f / (x_max - x_min);       //Precompute the inverse of the x range for faster calculation
            r_step = (r_max - r_min) / (x_axis - 1);      //Precompute the step size for r values to avoid redundant calculations
        }

    //Constructor to initialize the parameters
    public:
        DiaGenParameters() {
            initialize_derived_parameters();
        }

        DiaGenParameters(T x_min_, T x_max_, T r_min_, T r_max_, int resolution_x, int resolution_y) :
            x_axis(resolution_x),
            y_axis(resolution_y),
            x_min(x_min_),
            x_max(x_max_),
            r_min(r_min_),
            r_max(r_max_){
            initialize_derived_parameters();   
        }
};

//Type alias for the diagram matrix, to make the code more readable 
using DiagramMatrix = std::vector<int>;

//Function to calculate a portion of the bifurcation diagram columns, defined to be run in parallel threads
void calculate_columns(int i_start, int i_end, const DiaGenParameters& params, DiagramMatrix& diagram_matrix) {
    for (int i = i_start; i < i_end; i++) {
        T r = params.r_min + params.r_step * i;           //calculate r value for this column
        T x = params.x_init;                    //initialize x to the same for each r

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

int main(int argc, char* argv[]) {
    //Output directory for the generated files 
    const filesystem::path output_dir = "data";

    //Parse command line arguments in vars
    T x_min = 0.0f;
    T x_max = 1.0f; 
    T r_min = 2.5f; 
    T r_max = 4.0f; 
    int nx = 2048; 
    int ny = 2048; 
    int n_threads = std::thread::hardware_concurrency(); // /2; // I tried to divide it by two but it got slower so passed it

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // 
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

        if (arg == "--xmin") x_min = std::stof(argv[++i]);
        else if (arg == "--xmax") x_max = std::stof(argv[++i]);
        else if (arg == "--rmin") r_min = std::stof(argv[++i]);
        else if (arg == "--rmax") r_max = std::stof(argv[++i]);
        else if (arg == "--nx") nx = std::stoi(argv[++i]);
        else if (arg == "--ny") ny = std::stoi(argv[++i]);
        else {
            cerr << "Unknown or incomplete argument: " << arg << "\n";
            cerr << "Use --help for usage information.\n";
            return 1;
        }
    } 

    // Set the parsed parameters
    DiaGenParameters params(x_min, x_max, r_min, r_max, nx, ny);

    //Generate filenames for the output files
    const filesystem::path time_measurements_filename = "bifurcation_runtimes_" + to_string(params.y_axis) + "x" + to_string(params.x_axis) + "_CPU.txt";
    const filesystem::path diagram_filename = "bifurcation_diagram_" + to_string(params.y_axis) + "x" + to_string(params.x_axis) + "_CPU.txt";

    //Preallocate the diagram matrix for better cache performance
    DiagramMatrix diagram_matrix(params.x_axis * params.y_axis, 0);

    //Constants for time measurement and progress tracking
    const int NTIME = 200;
    const T progress_update_interval = 100.f / NTIME ;
    vector<double> time_measurements(NTIME);

    cout << "Starting bifurcation diagram generation with resolution " << params.x_axis << "x" << params.y_axis << " and " << NTIME << " time measurements\n";

    //time measurement loop
    for (int itime = 0; itime < NTIME; itime ++){
        //Reset the diagram matrix to zero for each time measurement iteration
        fill(diagram_matrix.begin(), diagram_matrix.end(), 0);

        // Define future vector for the values 
        std::vector<std::future<void>> futures(n_threads);

        int columns_per_thread = params.x_axis / n_threads;
        int remainder = params.x_axis % n_threads;
        int i_start = 0;

        //time measurement start
        auto t0 = tmark();

        //Main calcuation loop
        for(int t=0; t< n_threads; t++){
            int extra = (t < remainder) ? 1 : 0;
            int i_end = i_start + columns_per_thread + extra;

            futures[t] = std::async(std::launch::async, calculate_columns, i_start, i_end, std::cref(params), std::ref(diagram_matrix));

            i_start = i_end;
        }

        for(auto& future : futures){
            future.get();
        }

        //time measurement end and calculation of elapsed time
        auto t1 = tmark();
        time_measurements[itime] = delta_time(t0, t1);

        cout << "\rProgress: " << (itime+1)*progress_update_interval << "%" << std::flush;
    }

    cout << "\nBifurcation diagram generation completed\n";

    //Write time measurements to a file for later analysis
    cout << "\nWriting time measurements to " << time_measurements_filename << " file:\n";
    ofstream output_file(output_dir / time_measurements_filename);

    for (double time : time_measurements) {
        output_file << time << "\n";
        cerr << ".";
    }

    output_file.close();

    cout << "\nTime measurements are written successfully\n";

    //Write the diagram matrix to a file for later visualization
    cout << "\nWriting bifurcation diagram data to " << diagram_filename << " file:\n";
    output_file.open(output_dir / diagram_filename);

    for (int j = params.y_axis - 1; j >= 0; j--) {
        for (int i = 0; i < params.x_axis; i++) {
            output_file << diagram_matrix[i * params.y_axis + j] << " ";
        }
        output_file << "\n";
    }

    output_file.close();

    cout << "\nBifurcation diagram file is created successfully\n";

    return 0;
}