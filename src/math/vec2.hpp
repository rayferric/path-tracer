#pragma once

#include "pch.hpp"

namespace math {

template<typename T>
struct vec2 {
	T x, y;

	vec2();

	vec2(T all);

	vec2(T x, T y);

	// Vector + Vector

	template<typename L, typename R>
	friend auto operator+(const vec2<L> &lhs, const vec2<R> &rhs);

	template<typename L, typename R>
	friend auto operator-(const vec2<L> &lhs, const vec2<R> &rhs);

	template<typename L, typename R>
	friend auto operator*(const vec2<L> &lhs, const vec2<R> &rhs);

	template<typename L, typename R>
	friend auto operator/(const vec2<L> &lhs, const vec2<R> &rhs);

	template<typename L, typename R>
	friend auto operator%(const vec2<L> &lhs, const vec2<R> &rhs);

	template<typename U>
	friend vec2<U> floor(const vec2<U> &vec);
};

using bvec2 = vec2<bool>;
using ivec2 = vec2<int32_t>;
using uvec2 = vec2<uint32_t>;
using fvec2 = vec2<float>;
using dvec2 = vec2<double>;

}

#include "vec2.inl"
