namespace math {

template<typename T>
vec3<T>::vec3() : vec3(0) {}

template<typename T>
vec3<T>::vec3(T all) : x(all), y(all), z(all) {}

template<typename T>
vec3<T>::vec3(T x, T y, T z) : x(x), y(y), z(z) {}

// Vector + Vector

template<typename L, typename R>
auto operator+(const vec3<L> &lhs, const vec3<R> &rhs) {
	return vec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

template<typename L, typename R>
auto operator-(const vec3<L> &lhs, const vec3<R> &rhs) {
	return vec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

template<typename L, typename R>
auto operator*(const vec3<L> &lhs, const vec3<R> &rhs) {
	return vec3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

template<typename L, typename R>
auto operator/(const vec3<L> &lhs, const vec3<R> &rhs) {
	return vec3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
}

}
