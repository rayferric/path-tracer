cmake_minimum_required(VERSION 3.20)
project(raytracer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

# Third Party Dependencies
add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/stb")

# Conan Packages
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
find_package(assimp)

# Ray Tracer Target
file(GLOB_RECURSE TARGET_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(raytracer ${TARGET_SOURCES})

target_include_directories(raytracer PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(raytracer stb assimp::assimp)

target_precompile_headers(raytracer PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp"
)
