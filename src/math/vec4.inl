#include "util/strings.hpp"

namespace math {

template<typename T>
vec4<T>::vec4() : vec4(0) {}

template<typename T>
vec4<T>::vec4(T all) : x(all), y(all), z(all), w(all) {}

template<typename T>
vec4<T>::vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

// Vector + Vector

template<typename L, typename R>
auto operator+(const vec4<L> &lhs, const vec4<R> &rhs) {
	return vec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

template<typename L, typename R>
auto operator-(const vec4<L> &lhs, const vec4<R> &rhs) {
	return vec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}

template<typename L, typename R>
auto operator*(const vec4<L> &lhs, const vec4<R> &rhs) {
	return vec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
}

template<typename L, typename R>
auto operator/(const vec4<L> &lhs, const vec4<R> &rhs) {
	return vec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
}

// Vector + Scalar

template<typename L, typename R>
auto operator*(const vec4<L> &lhs, R rhs) {
	return vec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
}

template<typename L, typename R>
auto operator/(const vec4<L> &lhs, R rhs) {
	return vec4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
}

// Scalar + Vector

template<typename L, typename R>
auto operator*(L lhs, const vec4<R> &rhs) {
	return vec4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
}

template<typename U>
std::ostream &operator<<(std::ostream &lhs, const vec4<U> &rhs) {
	return lhs << '[' <<
			util::to_string(rhs.x) << ", " <<
			util::to_string(rhs.y) << ", " <<
			util::to_string(rhs.z) << ", " <<
			util::to_string(rhs.w) << ']';
}

}
