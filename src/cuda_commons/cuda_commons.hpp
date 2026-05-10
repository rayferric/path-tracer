#pragma once

#ifdef __CUDACC__
#include <cuda_runtime.h>
#include <iostream>
#include <cstdlib>

#define CUDA_CALLABLE __host__ __device__

inline void check_cuda_error(cudaError_t result, const char* stmt, const char* file, int line) {
    if (result != cudaSuccess) {
        std::cerr << "CUDA Error: " << stmt << " - " << cudaGetErrorString(result) 
                  << " at " << file << ":" << line << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
#define CUDA_CHECK(stmt) check_cuda_error((stmt), #stmt, __FILE__, __LINE__)
#define CUDA_CHECK_LAST_KERNEL_LAUNCH() check_cuda_error(cudaGetLastError(), "Kernel Launch", __FILE__, __LINE__)

#else
#define CUDA_CALLABLE
#endif

bool cuda_available();

// cuda memory management functions, defined to allow calling them from non-cuda code
void *cuda_malloc(size_t size);
void cuda_free(void *ptr);
enum class cuda_memcpy_kind {
    host_to_device,
    device_to_host,
    device_to_device,
    host_to_host
};
void cuda_memcpy(void *dst, const void *src, size_t size, cuda_memcpy_kind kind);
