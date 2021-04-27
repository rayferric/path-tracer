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

template<typename U>
vec2<U> floor(const vec2<U> &vec) {
	return vec2<U>(math::floor(vec.x), math::floor(vec.y));
}

}
