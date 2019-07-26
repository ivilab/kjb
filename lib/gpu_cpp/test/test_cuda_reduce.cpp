/* $Id$ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or fitness
   |  for any particular task. Nonetheless, I am interested in hearing about
   |  problems that you encounter.
   |
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/
// vim: tabstop=4 shiftwidth=4 foldmethod=marker



#include <gpu_cpp/gpu_cuda.h>
#include "l/l_sys_rand.h"
#include <iostream>

float cpu_reduce(const float *data, int size)
{
    float sum = data[0];
    float c = (float)0.0;
    int i;
    for (i = 1; i < size; i++)
    {
        float y = data[i] - c;
        float t = sum + y;      
        c = (t - sum) - y;  
        sum = t;            
    }
    return sum;
}

int main(int argc, char* argv[])
{
    using namespace kjb::gpu;
    using namespace kjb;
    using namespace std;
    using kjb_c::kjb_rand;

    int time_factor = 1;
    if(argc >= 2)
    {
        time_factor = atoi(argv[1]);
    }

    int NUM_ITERATIONS = 8000;
    int MAX_SIZE = 30000;
    int MIN_SIZE = 30000;

#ifdef KJB_HAVE_CUDA
    Cuda::set_jit_log_buffer_size(0); // no jit output
    Cuda_context ctx(Cuda::get_device(0));
    Cuda_reduce_module reduce_module;

    int N = (int)(MIN_SIZE + (kjb_rand() * (MAX_SIZE - MIN_SIZE)));
    float* array = new float[N];

    for(int i = 0; i < N; i++)
    {
        array[i] = kjb_c::kjb_rand();
    }

    for(int i = 0; i < time_factor * NUM_ITERATIONS; i++)
    {

        float gpu_result;
        try
        {
            gpu_result = reduce_module.reduce(array, N);
        }
        catch(Cuda_error& e)
        {
            std::cerr << e.get_msg() << std::endl;
            exit(1);
        }
        float cpu_result = cpu_reduce(array, N);

        if(fabs(gpu_result - cpu_result)/ cpu_result > FLT_EPSILON)
        {
            cerr << "ERROR: Results don't match\narray size: " << N << "\ncpu result: " << cpu_result << "\ngpu_result: " << gpu_result << endl;
            exit(0);
        }

    }

    delete[] array;
#else
    cout << "No testing is possible because this system lacks CUDA.\n";
#endif

    return EXIT_SUCCESS;
}
