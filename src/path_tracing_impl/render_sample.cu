#include "../pch.hpp"

#include "./render_sample_one_pixel.hpp"

__global__ void render_sample_kernel(
    const scene_data scn,
    glm::fvec4 *out_buf,
    uint32_t width,
    uint32_t height,
    uint32_t sample_idx,
    bool transparent_bg
) {
    uint32_t px = blockIdx.x * blockDim.x + threadIdx.x;
    uint32_t py = blockIdx.y * blockDim.y + threadIdx.y;

    if (px < width && py < height) {
        render_sample_one_pixel(scn, out_buf, width, height, px, py, sample_idx, transparent_bg);
    }
}

void render_sample_cuda(
    const scene_data &scn,
    glm::fvec4 *out_buf,
    uint32_t width,
    uint32_t height,
    uint32_t sample_idx,
    bool transparent_bg
) {
    dim3 block(16, 16); // 256 threads per block, good occupancy
    dim3 grid((width + 15) / 16, (height + 15) / 16);

    render_sample_kernel<<<grid, block>>>(
        scn, out_buf, width, height, sample_idx, transparent_bg
    );

    cudaDeviceSynchronize();
}
