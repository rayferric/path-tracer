#pragma once

#include "pch.hpp"

namespace math {

template<typename T>
struct vec3 {
	T x, y, z;

	vec3();

	vec3(T all);

	vec3(T x, T y, T z);

#pragma region Operators

	vec3<T> operator-() const;

	// Vector + Vector

	template<typename L, typename R>
	friend auto operator+(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename L, typename R>
	friend auto operator-(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename L, typename R>
	friend auto operator*(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename L, typename R>
	friend auto operator/(const vec3<L> &lhs, const vec3<R> &rhs);

	// Vector + Scalar

	template<typename L, typename R>
	friend auto operator*(const vec3<L> &lhs, R rhs);

	template<typename L, typename R>
	friend auto operator/(const vec3<L> &lhs, R rhs);

	// Scalar + Vector

	template<typename L, typename R>
	friend auto operator*(L lhs, const vec3<R> &rhs);

#pragma endregion

	template<typename L, typename R>
	friend auto dot(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename U>
	friend auto length(const vec3<U> &vec);

	template<typename U>
	friend auto normalize(const vec3<U> &vec);

	template<typename L, typename R>
	friend auto proj(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename L, typename R>
	friend auto cross(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename L, typename R>
	friend auto min(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename L, typename R>
	friend auto max(const vec3<L> &lhs, const vec3<R> &rhs);

	template<typename U>
	friend std::ostream &operator<<(std::ostream &lhs, const vec3<U> &rhs);
};

using bvec3 = vec3<bool>;
using ivec3 = vec3<int32_t>;
using uvec3 = vec3<uint32_t>;
using fvec3 = vec3<float>;
using dvec3 = vec3<double>;

}

#include "vec3.inl"
