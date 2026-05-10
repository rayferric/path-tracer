#include "./cuda_commons.hpp"

#include <stdio.h>

bool cuda_available() {
    int device_count = 0;
    cudaError_t error = cudaGetDeviceCount(&device_count);
    return error == cudaSuccess && device_count > 0;
}

void *cuda_malloc(size_t size) {
    void *ptr = nullptr;
    CUDA_CHECK(cudaMalloc(&ptr, size));
    return ptr;
}
void cuda_free(void *ptr) {
    CUDA_CHECK(cudaFree(ptr));
}
void cuda_memcpy(void *dst, const void *src, size_t size, cuda_memcpy_kind kind) {
    cudaMemcpyKind cuda_kind;
    switch (kind) {
        case cuda_memcpy_kind::host_to_device:
            cuda_kind = cudaMemcpyHostToDevice;
            break;
        case cuda_memcpy_kind::device_to_host:
            cuda_kind = cudaMemcpyDeviceToHost;
            break;
        case cuda_memcpy_kind::device_to_device:
            cuda_kind = cudaMemcpyDeviceToDevice;
            break;
        case cuda_memcpy_kind::host_to_host:
            cuda_kind = cudaMemcpyHostToHost;
            break;
        default:
            return; // Invalid kind
    }
    CUDA_CHECK(cudaMemcpy(dst, src, size, cuda_kind));
}
