#include "math/math.hpp"

namespace math {

template<typename T>
vec2<T>::vec2() : vec2(0) {}

template<typename T>
vec2<T>::vec2(T all) : x(all), y(all) {}

template<typename T>
vec2<T>::vec2(T x, T y) : x(x), y(y) {}

// Vector + Vector

template<typename L, typename R>
auto operator+(const vec2<L> &lhs, const vec2<R> &rhs) {
	return vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

template<typename L, typename R>
auto operator-(const vec2<L> &lhs, const vec2<R> &rhs) {
	return vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

template<typename L, typename R>
auto operator*(const vec2<L> &lhs, const vec2<R> &rhs) {
	return vec2(lhs.x * rhs.x, lhs.y * rhs.y);
}

template<typename L, typename R>
auto operator/(const vec2<L> &lhs, const vec2<R> &rhs) {
	return vec2(lhs.x / rhs.x, lhs.y / rhs.y);
}

template<typename L, typename R>
auto operator%(const vec2<L> &lhs, const vec2<R> &rhs) {
	return vec2(lhs.x % rhs.x, lhs.y % rhs.y);
}

template<typename T>
vec2<T> floor(const vec2<T> &v) {
	return vec2<T>(math::floor(v.x), math::floor(v.y));
}

}
