cmake_minimum_required(VERSION 3.20)
project(raytracer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/stb")

file(GLOB_RECURSE TARGET_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(raytracer ${TARGET_SOURCES})

target_include_directories(raytracer PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(raytracer stb)

target_precompile_headers(raytracer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")
