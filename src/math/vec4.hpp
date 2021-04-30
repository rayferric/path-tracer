#pragma once

#include "pch.hpp"

#include "math.hpp"

namespace math {

template<scalar T>
struct vec4 {
	T x, y, z, w;

	vec4();

	vec4(T all);

	vec4(T x, T y, T z, T w);

#pragma region Operators

	template<scalar U>
	friend std::ostream &operator<<(std::ostream &lhs, const vec4<U> &rhs);

	// Vector + Vector

	template<scalar L, scalar R>
	friend auto operator+(const vec4<L> &lhs, const vec4<R> &rhs);

	template<scalar L, scalar R>
	friend auto operator-(const vec4<L> &lhs, const vec4<R> &rhs);

	template<scalar L, scalar R>
	friend auto operator*(const vec4<L> &lhs, const vec4<R> &rhs);

	template<scalar L, scalar R>
	friend auto operator/(const vec4<L> &lhs, const vec4<R> &rhs);

	// Vector + Scalar

	template<scalar L, scalar R>
	friend auto operator*(const vec4<L> &lhs, R rhs);

	template<scalar L, scalar R>
	friend auto operator/(const vec4<L> &lhs, R rhs);

	// Scalar + Vector

	template<scalar L, scalar R>
	friend auto operator*(L lhs, const vec4<R> &rhs);

#pragma endregion

};

using bvec4 = vec4<bool>;
using ivec4 = vec4<int32_t>;
using uvec4 = vec4<uint32_t>;
using fvec4 = vec4<float>;
using dvec4 = vec4<double>;

}

#include "vec4.inl"
