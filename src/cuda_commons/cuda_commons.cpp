#include "./cuda_commons.hpp"

#ifndef ENABLE_CUDA
bool cuda_available() {
    return false;
}
#endif
