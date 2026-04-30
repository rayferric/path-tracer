cmake_minimum_required(VERSION 3.20)
project(path_tracer VERSION 1.0.0 LANGUAGES CXX)

# Add third-party subdirectories
add_subdirectory("vendor/glad")
add_subdirectory("vendor/glfw")
add_subdirectory("vendor/imgui")
add_subdirectory("vendor/stb")
add_subdirectory("vendor/tinygltf")

# Define "${PROJECT_NAME}" target
file(GLOB_RECURSE TARGET_SOURCES CONFIGURE_DEPENDS
    "src/*.hpp"
    "src/*.inl"
    "src/*.cpp"
)
add_executable(${PROJECT_NAME} ${TARGET_SOURCES})
target_precompile_headers(${PROJECT_NAME} PRIVATE
	"src/pch.hpp"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} PRIVATE
	"src"
)

find_package(assimp REQUIRED)
target_link_libraries(${PROJECT_NAME}
    glad glfw imgui stb tinygltf assimp
)

