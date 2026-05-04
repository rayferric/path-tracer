#pragma once

#ifdef __CUDACC__
#include <cuda_runtime.h>
#define CUDA_CALLABLE __host__ __device__
#else
#define CUDA_CALLABLE
#endif

bool cuda_available();
