#pragma once

#include "pch.hpp"

#include "math.hpp"

namespace math {

template<scalar T>
struct vec3 {
    static const vec3<T> zero;
    static const vec3<T> one;
    static const vec3<T> left;
    static const vec3<T> right;
    static const vec3<T> down;
    static const vec3<T> up;
    static const vec3<T> forward;
    static const vec3<T> backward;

	T x, y, z;

	vec3();

	vec3(T all);

	vec3(T x, T y, T z);

	template<scalar U>
	vec3(const vec3<U> &other);

#pragma region Operators

	template<scalar U>
	friend std::ostream &operator<<(std::ostream &lhs, const vec3<U> &rhs);

	T &operator[](size_t index);

	const T &operator[](size_t index) const;

	template<scalar U>
	vec3<bool> operator==(const vec3<U> &rhs) const;

	template<scalar U>
	vec3<bool> operator!=(const vec3<U> &rhs) const;

	template<scalar U>
	vec3<bool> operator<=(const vec3<U> &rhs) const;

	template<scalar U>
	vec3<bool> operator>=(const vec3<U> &rhs) const;

	template<scalar U>
	vec3<bool> operator<(const vec3<U> &rhs) const;

	template<scalar U>
	vec3<bool> operator>(const vec3<U> &rhs) const;

	vec3<bool> operator!() const;

	vec3<T> operator-() const;

	// Vector + Vector

	template<scalar U>
	auto operator+(const vec3<U> &rhs) const;

	template<scalar U>
	auto operator-(const vec3<U> &rhs) const;

	template<scalar U>
	auto operator*(const vec3<U> &rhs) const;

	template<scalar U>
	auto operator/(const vec3<U> &rhs) const;

	template<scalar U>
	vec3<T> &operator+=(const vec3<U> &rhs);

	template<scalar U>
	vec3<T> &operator-=(const vec3<U> &rhs);

	template<scalar U>
	vec3<T> &operator*=(const vec3<U> &rhs);

	template<scalar U>
	vec3<T> &operator/=(const vec3<U> &rhs);

	// Vector + Scalar

	template<scalar U>
	auto operator*(U rhs) const;

	template<scalar U>
	auto operator/(U rhs) const;

	template<scalar U>
	vec3<T> &operator*=(U rhs);

	template<scalar U>
	vec3<T> &operator/=(U rhs);

	// Scalar + Vector

	template<scalar L, scalar R>
	friend auto operator*(L lhs, const vec3<R> &rhs);

#pragma endregion

};

using bvec3 = vec3<bool>;
using ivec3 = vec3<int32_t>;
using uvec3 = vec3<uint32_t>;
using fvec3 = vec3<float>;
using dvec3 = vec3<double>;

template<scalar T>
bool all(const vec3<T> &vec);

template<scalar T>
bool any(const vec3<T> &vec);

template<scalar L, scalar R,
		typename Ret = std::common_type_t<L, R>>
vec3<Ret> cross(const vec3<L> &lhs, const vec3<R> &rhs);

template<scalar A, scalar B,
		typename Ret = std::common_type_t<A, B>>
Ret distance(const vec3<A> &a, const vec3<B> &b);

template<scalar A, scalar B,
		typename Ret = std::common_type_t<A, B>>
Ret dot(const vec3<A> &a, const vec3<B> &b);

template<scalar T, scalar Epsilon = decltype(epsilon)>
bool is_normalized(const vec3<T> &vec, Epsilon epsilon = math::epsilon);

template<scalar T>
T length(const vec3<T> &vec);

template<scalar T>
vec3<T> normalize(const vec3<T> &vec);

template<scalar To, scalar From,
		typename Ret = std::common_type_t<To, From>>
vec3<Ret> proj(const vec3<To> &to, const vec3<From> &from);

#pragma region Component-Wise Math Wrappers

template<scalar X>
vec3<X> fract(const vec3<X> &x);

template<scalar From, scalar To, scalar Weight,
		typename Ret = std::common_type_t<From, To, Weight>>
vec3<Ret> lerp(const vec3<From> &from, const vec3<To> &to, Weight weight);

template<scalar A, scalar B,
		typename Ret = std::common_type_t<A, B>>
vec3<Ret> max(const vec3<A> &a, const vec3<B> &b);

template<scalar A, scalar B, scalar... Others,
		typename Ret = std::common_type_t<A, B, Others...>>
vec3<Ret> max(const vec3<A> &a, const vec3<B> &b,
		const vec3<Others> &...others);

template<scalar A, scalar B,
		typename Ret = std::common_type_t<A, B>>
vec3<Ret> min(const vec3<A> &a, const vec3<B> &b);

template<scalar A, scalar B, scalar... Others,
		typename Ret = std::common_type_t<A, B, Others...>>
vec3<Ret> min(const vec3<A> &a, const vec3<B> &b,
		const vec3<Others> &...others);

template<scalar X, scalar Y,
		typename Ret = std::common_type_t<X, Y>>
vec3<Ret> mod(const vec3<X> &x, const vec3<Y> &y);

#pragma endregion

}

#include "vec3.inl"
