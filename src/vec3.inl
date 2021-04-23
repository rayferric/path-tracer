template<typename T>
vec3<T>::vec3() : vec3(0) {}

template<typename T>
vec3<T>::vec3(T all) : x(all), y(all), z(all) {}

template<typename T>
vec3<T>::vec3(T x, T y, T z) : x(x), y(y), z(z) {}
