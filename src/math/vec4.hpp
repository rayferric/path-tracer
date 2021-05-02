#pragma once

#include "pch.hpp"

#include "math.hpp"

namespace math {

template<scalar T>
struct vec4 {
    static const vec4<T> zero;
    static const vec4<T> one;
    static const vec4<T> left;
    static const vec4<T> right;
    static const vec4<T> down;
    static const vec4<T> up;
    static const vec4<T> forward;
    static const vec4<T> backward;
	static const vec4<T> past;
    static const vec4<T> future;

	T x, y, z, w;

	vec4();

	vec4(T all);

	vec4(T x, T y, T z, T w);

	template<scalar U>
	vec4(const vec4<U> &other);

#pragma region Operators

	template<scalar U>
	friend std::ostream &operator<<(std::ostream &lhs, const vec4<U> &rhs);

	T &operator[](size_t index);

	const T &operator[](size_t index) const;

	template<scalar U>
	vec4<bool> operator==(const vec4<U> &rhs) const;

	template<scalar U>
	vec4<bool> operator!=(const vec4<U> &rhs) const;

	template<scalar U>
	vec4<bool> operator<=(const vec4<U> &rhs) const;

	template<scalar U>
	vec4<bool> operator>=(const vec4<U> &rhs) const;

	template<scalar U>
	vec4<bool> operator<(const vec4<U> &rhs) const;

	template<scalar U>
	vec4<bool> operator>(const vec4<U> &rhs) const;

	template<boolean U>
	friend vec4<bool> operator!(const vec4<U> &mat);

	vec4<T> operator-() const;

	// Vector + Vector

	template<scalar U>
	auto operator+(const vec4<U> &rhs) const;

	template<scalar U>
	auto operator-(const vec4<U> &rhs) const;

	template<scalar U>
	auto operator*(const vec4<U> &rhs) const;

	template<scalar U>
	auto operator/(const vec4<U> &rhs) const;

	template<scalar U>
	vec4<T> &operator+=(const vec4<U> &rhs);

	template<scalar U>
	vec4<T> &operator-=(const vec4<U> &rhs);

	template<scalar U>
	vec4<T> &operator*=(const vec4<U> &rhs);

	template<scalar U>
	vec4<T> &operator/=(const vec4<U> &rhs);

	// Vector + Scalar

	template<scalar U>
	auto operator*(U rhs) const;

	template<scalar U>
	auto operator/(U rhs) const;

	template<scalar U>
	vec4<T> &operator*=(U rhs);

	template<scalar U>
	vec4<T> &operator/=(U rhs);

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

template<boolean T>
bool all(const vec4<T> &vec);

template<boolean T>
bool any(const vec4<T> &vec);

template<scalar A, scalar B,
		floating_point Ret = std::common_type_t<A, B>>
Ret distance(const vec4<A> &a, const vec4<B> &b);

template<scalar A, scalar B,
		scalar Ret = std::common_type_t<A, B>>
Ret dot(const vec4<A> &a, const vec4<B> &b);

template<scalar T, scalar Epsilon = decltype(epsilon)>
bool is_normalized(const vec4<T> &vec, Epsilon epsilon = math::epsilon);

template<floating_point T>
T length(const vec4<T> &vec);

template<floating_point T>
vec4<T> normalize(const vec4<T> &vec);

template<floating_point To, floating_point From,
		scalar Ret = std::common_type_t<To, From>>
vec4<Ret> proj(const vec4<To> &to, const vec4<From> &from);

#pragma region Component-Wise Math Wrappers

template<floating_point X>
vec4<X> fract(const vec4<X> &x);

template<scalar From, scalar To, scalar Weight,
		scalar Ret = std::common_type_t<From, To, Weight>>
vec4<Ret> lerp(const vec4<From> &from, const vec4<To> &to, Weight weight);

template<scalar A, scalar B,
		scalar Ret = std::common_type_t<A, B>>
vec4<Ret> max(const vec4<A> &a, const vec4<B> &b);

template<scalar A, scalar B, scalar... Others,
		scalar Ret = std::common_type_t<A, B, Others...>>
vec4<Ret> max(const vec4<A> &a, const vec4<B> &b,
		const vec4<Others> &...others);

template<scalar A, scalar B,
		scalar Ret = std::common_type_t<A, B>>
vec4<Ret> min(const vec4<A> &a, const vec4<B> &b);

template<scalar A, scalar B, scalar... Others,
		scalar Ret = std::common_type_t<A, B, Others...>>
vec4<Ret> min(const vec4<A> &a, const vec4<B> &b,
		const vec4<Others> &...others);

template<scalar X, scalar Y,
		scalar Ret = std::common_type_t<X, Y>>
vec4<Ret> mod(const vec4<X> &x, const vec4<Y> &y);

#pragma endregion

}

#include "vec4.inl"
