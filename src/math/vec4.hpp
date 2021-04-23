#pragma once

#include "pch.hpp"

namespace math {

template<typename T>
struct vec4 {
	T x, y, z, w;

	vec4();

	vec4(T all);

	vec4(T x, T y, T z, T w);

	// Vector + Vector

	template<typename L, typename R>
	friend auto operator+(const vec4<L> &lhs, const vec4<R> &rhs);

	template<typename L, typename R>
	friend auto operator-(const vec4<L> &lhs, const vec4<R> &rhs);

	template<typename L, typename R>
	friend auto operator*(const vec4<L> &lhs, const vec4<R> &rhs);

	template<typename L, typename R>
	friend auto operator/(const vec4<L> &lhs, const vec4<R> &rhs);

	// Vector + Scalar

	template<typename L, typename R>
	friend auto operator*(const vec4<L> &lhs, R rhs);

	template<typename L, typename R>
	friend auto operator/(const vec4<L> &lhs, R rhs);

	// Scalar + Vector

	template<typename L, typename R>
	friend auto operator*(L lhs, const vec4<R> &rhs);

	template<typename T>
	friend std::ostream &operator<<(std::ostream &lhs, const vec4<T> &rhs);
};

using bvec4 = vec4<bool>;
using ivec4 = vec4<int32_t>;
using uvec4 = vec4<uint32_t>;
using fvec4 = vec4<float>;
using dvec4 = vec4<double>;

}

#include "vec4.inl"
