
#include "l/l_incl.h"

using namespace kjb_c;

#ifdef KJB_HAVE_CUDA
#ifdef KJB_HAVE_CUDART

#include "cuda.h" 
#include "cuda_runtime.h" 


/*
#define DEBUG
*/

int main(void)
{
  int nDevices;

  cudaGetDeviceCount(&nDevices);

  for (int i = 0; i < nDevices; i++) 
  {
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, i);

    pso("Device Number: %d\n", i);
    pso("  Device name: %s\n", prop.name);
    pso("  Memory Clock Rate (KHz): %d\n",
        prop.memoryClockRate);
    pso("  Memory Bus Width (bits): %d\n",
        prop.memoryBusWidth);
    pso("  Peak Memory Bandwidth (GB/s): %f\n\n",
        2.0*prop.memoryClockRate*(prop.memoryBusWidth/8)/1.0e6);
  }

  kjb_exit(EXIT_SUCCESS);
}


#else 
int main(void)
{

  p_stderr("CUDART is not available.\n");
  kjb_exit(EXIT_FAILURE);
}

#endif 

#else 
int main(void)
{

  p_stderr("CUDA is not available.\n");
  kjb_exit(EXIT_FAILURE);
}

#endif 


