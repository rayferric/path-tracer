#pragma once

inline glm::vec3 tonemap_approx_aces(const glm::fvec3 &hdr) {
	constexpr float a = 2.51f;
	constexpr glm::fvec3 b(0.03f);
	constexpr float c = 2.43F;
	constexpr glm::fvec3 d(0.59f);
	constexpr glm::fvec3 e(0.14f);
	return glm::clamp((hdr * (a * hdr + b)) / (hdr * (c * hdr + d) + e), glm::fvec3(0.0f), glm::fvec3(1.0f));
}

inline void tonemap_buffer_srgb8(uint8_t *out, const glm::fvec4 *in, size_t size, float exposure) {
    for (size_t i = 0; i < size; i++) {
        glm::fvec3 color = tonemap_approx_aces(glm::fvec3(in[i]) * exposure);
        // gamma correction - we're outputting to srgb buffer
        color = glm::pow(color, glm::fvec3(1.0f / 2.2f));
        for (int c = 0; c < 3; c++) {
            out[i * 4 + c] = static_cast<uint8_t>(std::clamp(color[c] * 255.0f, 0.0f, 255.0f));
        }
        out[i * 4 + 3] = static_cast<uint8_t>(std::clamp(in[i].a * 255.0f, 0.0f, 255.0f));
    }
}
