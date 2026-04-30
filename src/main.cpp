#include "pch.hpp"

#include "core/pbr.hpp"
#include "core/renderer.hpp"
#include "image/image_texture.hpp"
#include "gl/window.hpp"

#include <glad/gl.h>
#include <iostream>

using namespace math;

// A simple vertex shader to generate a full-screen triangle without VBOs
const char* vs_code = R"(
#version 460 core
out vec2 TexCoords;
void main() {
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    TexCoords.x = (x + 1.0) * 0.5;
    TexCoords.y = 1.0 - (y + 1.0) * 0.5; // Flip Y to match image data orientation
    gl_Position = vec4(x, y, 0.0, 1.0);
}
)";

// Fragment shader to sample the screen texture
const char* fs_code = R"(
#version 460 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D screenTexture;
void main() {
    float checkerSize = 20.0;
    vec2 pos = floor(gl_FragCoord.xy / checkerSize);
    float pattern = mod(pos.x + pos.y, 2.0);
    vec3 bgColor = mix(vec3(0.15), vec3(0.25), pattern);
    
    vec4 texColor = texture(screenTexture, TexCoords);
    
    // Alpha blend over the checkered background
    FragColor = vec4(mix(bgColor, texColor.rgb, texColor.a), 1.0);
}
)";

// Utility function to compile shaders and print errors if any
GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error: " << infoLog << std::endl;
    }
    return shader;
}

int main() {
    // Initialize and open our GLFW window via the wrapper
    window win;
    win.open(1280, 720, "Path Tracer");

    // Compile shader program for drawing an image to the screen
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_code);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_code);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    // Create VAO (mandatory for OpenGL core profile even without buffers)
    GLuint vao;
    glGenVertexArrays(1, &vao);

    // Create and configure texture for screen presentation
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // Nearest filtering gives blocks/pixels rather than blurry edges on lower resolutions
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Initialize renderer
    core::renderer renderer;
    renderer.sample_count = 10000;
    renderer.bounce_count = 4;
    
    // Keep render resolution quite low to keep frame times interactive on the CPU!
    renderer.resolution = uvec2(320, 180);
    renderer.environment_factor = fvec3::zero;
    
    std::cout << "Loading scene..." << std::endl;
    renderer.load_gltf("assets/porsche/porsche.gltf");
    auto hdri = image::image::load("assets/dirt-road.hdr", false);
    renderer.environment = std::make_shared<image::image_texture>(hdri);
    renderer.environment_factor = fvec3(3.0);
    renderer.sun_light->angular_radius = 0.05F;
    renderer.transparent_background = true;

    // Initial HDR accumulation buffer
    auto hdr = renderer.init_hdr_accum();
    uint32_t current_sample = 0;

    // Setup our window event loop
    window::loop_info info;
    
    info.on_update = [&](float dt) {
        if (current_sample < renderer.sample_count) {
            // Print progress back over the same line
            std::cout << "Rendering sample " << current_sample + 1 << " / " << renderer.sample_count << '\r' << std::flush;
            
            // Accumulate one specific sample index and step
            renderer.render_sample(hdr, current_sample);
            current_sample++;
            
            // Tonemap to standard SDR image format
            auto sdr = renderer.tonemap_output(hdr);
            
            // Copy standard dynamic range bytes to OpenGL texture 
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sdr->get_size().x, sdr->get_size().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, sdr->get_data().data());
        }
    };

    info.on_draw = [&]() {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindVertexArray(vao);
        // Draw exactly 3 vertices so the shader maps them into a full screen quad/triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);
    };

    info.on_resize = [&](uint32_t w, uint32_t h) {
        glViewport(0, 0, w, h);
    };

    std::cout << "Starting rendering loop... " << std::endl;
    win.run_loop(info);

    // Optionally save it as well after closing the window!
    // auto sdr = renderer.tonemap_output(hdr);
    // sdr->save("renders/porsche-interactive.png");

    return 0;
}
