#pragma once

#include "./pch.hpp"

#include "./scene.hpp"

void render_sample(const scene_data &scn, glm::fvec4 *out_buf, uint32_t width, uint32_t height, uint32_t sample_idx, bool transparent_bg, void **cache = nullptr);
void render_sample_free_cache(void **cache);

#ifdef ENABLE_CUDA
void render_sample_cuda(const scene_data &cuda_scn, glm::fvec4 *out_buf, uint32_t width, uint32_t height, uint32_t sample_idx, bool transparent_bg, void **cache = nullptr);
void render_sample_cuda_free_cache(void **cache);
#endif
