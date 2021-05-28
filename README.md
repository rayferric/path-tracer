## How To Build
```
conan install -s build_type=Release -if build .
cmake -B ./build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build ./build --target raytracer --config Release
```
