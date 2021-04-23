#pragma once

#include "pch.hpp"

namespace math {

template<typename T>
struct vec3 {
	T x, y, z;

	vec3();

	vec3(T all);

	vec3(T x, T y, T z);

	// Vector + Vector

	template<typename L, typename R>
	friend auto operator+(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename L, typename R>
	friend auto operator-(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename L, typename R>
	friend auto operator*(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename L, typename R>
	friend auto operator/(const vec3<L> &lhs, const vec3<R> &rhs);
};

using bvec3 = vec3<bool>;
using ivec3 = vec3<int32_t>;
using uvec3 = vec3<uint32_t>;
using fvec3 = vec3<float>;
using dvec3 = vec3<double>;

}

#include "vec3.inl"
