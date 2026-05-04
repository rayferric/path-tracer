# Path Tracer

Interactive CUDA path tracer with PBR quality

## Features

- Realistic path tracing with PBR metallic-roughness materials
- CUDA GPU-accelerated backend
- Alternative CPU backend with tiling scheduler
- Interactive GUI render editor
- GLTF scene import with emissive objects, sunlight and cameras
- HDRI environment maps
- Transparent rendering for compositing

## Demo

![](./demo.gif)

## Gallery

<div style="display: flex; flex-direction: row; flex-wrap: wrap; justify-content: center;">
	<div style="display: flex; flex-direction: column;">
		<img style="width: 400px; margin: 20px;" src="./renders/porsche.preview.png" />
		<img style="width: 400px; margin: 20px;" src="./renders/cornell-box.preview.jpg" />
		<img style="width: 400px; margin: 20px;" src="./renders/sponza-alt.preview.jpg" />
		<img style="width: 400px; margin: 20px;" src="./renders/jack-of-blades.preview.png" />
	</div>
	<div style="display: flex; flex-direction: column;">
		<img style="width: 400px; margin: 20px;" src="./renders/dragon.preview.png" />
		<img style="width: 400px; margin: 20px;" src="./renders/sponza.preview.jpg" />
		<img style="width: 400px; margin: 20px;" src="./renders/sci-fi-helmet.preview.jpg" />
		<img style="width: 400px; margin: 20px;" src="./renders/cerberus.preview.png" />
	</div>
</div>

## Building

### Dependencies

- **[CMake](https://cmake.org/download/#latest)**
- **[CUDA SDK](https://developer.nvidia.com/cuda-downloads)**

### Commands

```
mkdir ./build
cmake -B ./build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build ./build --config Release
./build/path_tracer
```
