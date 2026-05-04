#include "./cuda_commons.hpp"

bool cuda_available() {
    int device_count = 0;
    cudaError_t error = cudaGetDeviceCount(&device_count);
    return error == cudaSuccess && device_count > 0;
}
