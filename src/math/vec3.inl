#include "math/math.hpp"
#include "util/strings.hpp"

namespace math {

template<typename T>
vec3<T>::vec3() : vec3(0) {}

template<typename T>
vec3<T>::vec3(T all) : x(all), y(all), z(all) {}

template<typename T>
vec3<T>::vec3(T x, T y, T z) : x(x), y(y), z(z) {}

#pragma region Operators

template<typename T>
vec3<T> vec3<T>::operator-() const {
	return vec3<T>(-x, -y, -z);
}

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

// Vector + Scalar

template<typename L, typename R>
auto operator*(const vec3<L> &lhs, R rhs) {
	return vec3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}

template<typename L, typename R>
auto operator/(const vec3<L> &lhs, R rhs) {
	return vec3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}

// Scalar + Vector

template<typename L, typename R>
auto operator*(L lhs, const vec3<R> &rhs) {
	return vec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

#pragma endregion

template<typename L, typename R>
auto dot(const vec3<L> &lhs, const vec3<R> &rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template<typename U>
auto length(const vec3<U> &vec) {
	return sqrt(dot(vec, vec));
}

template<typename U>
auto normalize(const vec3<U> &vec) {
	return vec / length(vec);
}

template<typename L, typename R>
auto proj(const vec3<L> &lhs, const vec3<R> &rhs) {
	return (dot(lhs, rhs) / dot(lhs, lhs)) * lhs;
}

template<typename L, typename R>
auto cross(const vec3<L> &lhs, const vec3<R> &rhs) {
	return vec3(
		(lhs.y * rhs.z) - (lhs.z * rhs.y),
		(lhs.z * rhs.x) - (lhs.x * rhs.z),
		(lhs.x * rhs.y) - (lhs.y * rhs.x)
	);
}

template<typename L, typename R>
auto min(const vec3<L> &lhs, const vec3<R> &rhs) {
	return vec3(min(lhs.x, rhs.x),
				min(lhs.y, rhs.y),
				min(lhs.z, rhs.z));
}

template<typename L, typename R>
auto max(const vec3<L> &lhs, const vec3<R> &rhs) {
	return vec3(max(lhs.x, rhs.x),
				max(lhs.y, rhs.y),
				max(lhs.z, rhs.z));
}

template<typename U>
std::ostream &operator<<(std::ostream &lhs, const vec3<U> &rhs) {
	return lhs << '[' <<
			util::to_string(rhs.x) << ", " <<
			util::to_string(rhs.y) << ", " <<
			util::to_string(rhs.z) << ']';
}

}
