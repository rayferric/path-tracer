#include "util/strings.hpp"

namespace math {

template<scalar T>
vec4<T>::vec4() : vec4(0) {}

template<scalar T>
vec4<T>::vec4(T all) :
		x(all), y(all), z(all), w(all) {}

template<scalar T>
vec4<T>::vec4(T x, T y, T z, T w) :
		x(x), y(y), z(z), w(w) {}

#pragma region Operators

template<scalar U>
std::ostream &operator<<(std::ostream &lhs, const vec4<U> &rhs) {
	return lhs << '[' <<
			util::to_string(rhs.x) << ", " <<
			util::to_string(rhs.y) << ", " <<
			util::to_string(rhs.z) << ", " <<
			util::to_string(rhs.w) << ']';
}

// Vector + Vector

template<scalar L, scalar R>
auto operator+(const vec4<L> &lhs, const vec4<R> &rhs) {
	return vec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

template<scalar L, scalar R>
auto operator-(const vec4<L> &lhs, const vec4<R> &rhs) {
	return vec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}

template<scalar L, scalar R>
auto operator*(const vec4<L> &lhs, const vec4<R> &rhs) {
	return vec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
}

template<scalar L, scalar R>
auto operator/(const vec4<L> &lhs, const vec4<R> &rhs) {
	return vec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
}

// Vector + Scalar

template<scalar L, scalar R>
auto operator*(const vec4<L> &lhs, R rhs) {
	return vec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
}

template<scalar L, scalar R>
auto operator/(const vec4<L> &lhs, R rhs) {
	return vec4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
}

// Scalar + Vector

template<scalar L, scalar R>
auto operator*(L lhs, const vec4<R> &rhs) {
	return vec4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
}

#pragma endregion

}
