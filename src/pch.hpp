#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <execution>
#include <filesystem>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numbers>
#include <numeric>
#include <queue>
#include <random>
#include <regex>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <tinygltf/tiny_gltf.h>

#include <assimp/Importer.hpp>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>