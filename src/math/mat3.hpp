#pragma once

#include "math/vec3.hpp"

namespace math {

template<typename T>
struct mat3 {
	vec3<T> x, y, z;

	mat3();

	mat3(T identity);

	mat3(const vec3<T> &x, const vec3<T> &y, const vec3<T> &z);

	mat3(T xx, T xy, T xz,
		 T yx, T yy, T yz,
		 T zx, T zy, T zz);

	mat3(const T *array);

	template<typename U>
	mat3(const mat3<U> &other);

	template<typename U>
	friend U determinant(const mat3<U> &mat);
};

using bmat3 = mat3<bool>;
using imat3 = mat3<int32_t>;
using umat3 = mat3<uint32_t>;
using fmat3 = mat3<float>;
using dmat3 = mat3<double>;

}

#include "mat3.inl"
