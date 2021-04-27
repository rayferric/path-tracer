namespace math {


template<typename T>
mat3<T>::mat3() : mat3(1) {}

template<typename T>
mat3<T>::mat3(T identity) :
		x(identity, 0, 0),
		y(0, identity, 0),
		z(0, 0, identity) {}

template<typename T>
mat3<T>::mat3(
		const vec3<T> &x,
		const vec3<T> &y,
		const vec3<T> &z) :
		x(x), y(y), z(z) {}

template<typename T>
mat3<T>::mat3(
		T xx, T xy, T xz,
		T yx, T yy, T yz,
		T zx, T zy, T zz) :
		x(xx, xy, xz),
		y(yx, yy, yz),
		z(zx, zy, zz) {}

template<typename T>
mat3<T>::mat3(const T *array) :
	x(array[0], array[1], array[2]),
	y(array[3], array[4], array[5]),
	z(array[6], array[7], array[8]) {}

template<typename T>
template<typename U>
mat3<T>::mat3(const mat3<U> &other) :
		mat3(other.x, other.y, other.z) {}

template<typename U>
U determinant(const mat3<U> &mat) {
	return mat.x.x * (mat.y.y * mat.z.z - mat.z.y * mat.y.z) -
		   mat.y.x * (mat.x.y * mat.z.z - mat.z.y * mat.x.z) +
		   mat.z.x * (mat.x.y * mat.y.z - mat.y.y * mat.x.z);
}

}
