cmake_minimum_required(VERSION 3.20)
project(path_tracer VERSION 1.0.0 LANGUAGES CXX)

# disable MSVC's default CRT linkage to suppress warnings
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
endif()

option(ENABLE_CUDA "Enable CUDA support" ON)
if(ENABLE_CUDA)
    enable_language(CUDA)
    # suppress NVCC warnings caused by GLM
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --diag-suppress 20012,20094 --expt-relaxed-constexpr")
endif()

add_subdirectory("vendor/glad")
add_subdirectory("vendor/glfw")
add_subdirectory("vendor/glm")
add_subdirectory("vendor/imgui")
add_subdirectory("vendor/portable-file-dialogs")
add_subdirectory("vendor/stb")
add_subdirectory("vendor/tinygltf")

file(GLOB_RECURSE TARGET_SOURCES CONFIGURE_DEPENDS
    "src/*.hpp"
    "src/*.inl"
    "src/*.cpp"
)

if(ENABLE_CUDA)
    file(GLOB_RECURSE CUDA_SOURCES CONFIGURE_DEPENDS "src/*.cu")
    list(APPEND TARGET_SOURCES ${CUDA_SOURCES})
endif()

add_executable(${PROJECT_NAME} ${TARGET_SOURCES})
target_precompile_headers(${PROJECT_NAME} PRIVATE
	"src/pch.hpp"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(ENABLE_CUDA)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_CUDA)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CUDA_STANDARD 20
        CUDA_STANDARD_REQUIRED ON
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_ARCHITECTURES native
    )
endif()

target_link_libraries(${PROJECT_NAME}
    glad glfw glm imgui portable_file_dialogs stb tinygltf
)
