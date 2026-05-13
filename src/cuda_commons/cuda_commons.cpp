#include "./cuda_commons.hpp"

#ifndef ENABLE_CUDA
bool cuda_available() {
	return false;
}
void *cuda_malloc(size_t size) {
	return nullptr;
}
void cuda_free(void *ptr) {}
void cuda_memcpy(void *dst, const void *src, size_t size, cuda_memcpy_kind kind) {}
#endif
