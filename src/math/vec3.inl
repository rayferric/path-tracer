#include "util/strings.hpp"

namespace math {

template<scalar T>
const vec3<T> vec3<T>::zero(0);

template<scalar T>
const vec3<T> vec3<T>::one(1);

template<scalar T>
const vec3<T> vec3<T>::left(-1, 0, 0);

template<scalar T>
const vec3<T> vec3<T>::right(1, 0, 0);

template<scalar T>
const vec3<T> vec3<T>::down(0, -1, 0);

template<scalar T>
const vec3<T> vec3<T>::up(0, 1, 0);

template<scalar T>
const vec3<T> vec3<T>::forward(0, 0, -1);

template<scalar T>
const vec3<T> vec3<T>::backward(0, 0, 1);

template<scalar T>
vec3<T>::vec3() : vec3(0) {}

template<scalar T>
vec3<T>::vec3(T all) :
		x(all), y(all), z(all) {}

template<scalar T>
vec3<T>::vec3(T x, T y, T z) :
		x(x), y(y), z(z) {}

template<scalar T>
template<scalar U>
vec3<T>::vec3(const vec3<U> &other) :
		x(other.x), y(other.y), z(other.z) {}

#pragma region Operators

template<scalar U>
std::ostream &operator<<(std::ostream &lhs, const vec3<U> &rhs) {
	return lhs << '[' <<
			util::to_string(rhs.x) << ", " <<
			util::to_string(rhs.y) << ", " <<
			util::to_string(rhs.z) << ']';
}

template<scalar T>
T &vec3<T>::operator[](size_t index) {
	return *(reinterpret_cast<float *>(this) + index);
}

template<scalar T>
const T &vec3<T>::operator[](size_t index) const {
	return *(reinterpret_cast<const float *>(this) + index);
}

template<scalar T>
vec3<T> vec3<T>::operator-() const {
	return vec3<T>(-x, -y, -z);
}

template<scalar T>
vec3<bool> vec3<T>::operator!() const {
	return vec3<bool>(!x, !y, !z);
}

template<scalar T>
template<scalar U>
vec3<bool> vec3<T>::operator==(const vec3<U> &rhs) const {
	return vec3<bool>(x == rhs.x, y == rhs.y, z == rhs.z);
}

template<scalar T>
template<scalar U>
vec3<bool> vec3<T>::operator!=(const vec3<U> &rhs) const {
	return vec3<bool>(x != rhs.x, y != rhs.y, z != rhs.z);
}

template<scalar T>
template<scalar U>
vec3<bool> vec3<T>::operator<=(const vec3<U> &rhs) const {
	return vec3<bool>(x <= rhs.x, y <= rhs.y, z <= rhs.z);
}

template<scalar T>
template<scalar U>
vec3<bool> vec3<T>::operator>=(const vec3<U> &rhs) const {
	return vec3<bool>(x >= rhs.x, y >= rhs.y, z >= rhs.z);
}

template<scalar T>
template<scalar U>
vec3<bool> vec3<T>::operator<(const vec3<U> &rhs) const {
	return vec3<bool>(x < rhs.x, y < rhs.y, z < rhs.z);
}

template<scalar T>
template<scalar U>
vec3<bool> vec3<T>::operator>(const vec3<U> &rhs) const {
	return vec3<bool>(x > rhs.x, y > rhs.y, z > rhs.z);
}

// Vector + Vector

template<scalar T>
template<scalar U>
auto vec3<T>::operator+(const vec3<U> &rhs) const {
	return vec3(x + rhs.x, y + rhs.y, z + rhs.z);
}

template<scalar T>
template<scalar U>
auto vec3<T>::operator-(const vec3<U> &rhs) const {
	return vec3(x - rhs.x, y - rhs.y, z - rhs.z);
}

template<scalar T>
template<scalar U>
auto vec3<T>::operator*(const vec3<U> &rhs) const {
	return vec3(x * rhs.x, y * rhs.y, z * rhs.z);
}

template<scalar T>
template<scalar U>
auto vec3<T>::operator/(const vec3<U> &rhs) const {
	return vec3(x / rhs.x, y / rhs.y, z / rhs.z);
}

// Vector + Scalar

template<scalar T>
template<scalar U>
auto vec3<T>::operator*(U rhs) const {
	return vec3(x * rhs, y * rhs, z * rhs);
}

template<scalar T>
template<scalar U>
auto vec3<T>::operator/(U rhs) const {
	return vec3(x / rhs, y / rhs, z / rhs);
}

// Scalar + Vector

template<scalar L, scalar R>
auto operator*(L lhs, const vec3<R> &rhs) {
	return vec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

#pragma endregion

template<scalar T>
bool all(const vec3<T> &vec) {
	return vec.x && vec.y && vec.z;
}

template<scalar T>
bool any(const vec3<T> &vec) {
	return vec.x || vec.y || vec.z;
}

template<scalar L, scalar R,
		typename Ret = std::common_type_t<L, R>>
vec3<Ret> cross(const vec3<L> &lhs, const vec3<R> &rhs) {
	return vec3<Ret>(
		(lhs.y * rhs.z) - (lhs.z * rhs.y),
		(lhs.z * rhs.x) - (lhs.x * rhs.z),
		(lhs.x * rhs.y) - (lhs.y * rhs.x)
	);
}

template<scalar A, scalar B,
		typename Ret = std::common_type_t<A, B>>
Ret distance(const vec3<A> &a, const vec3<B> &b) {
	return length(a - b);
}

template<scalar A, scalar B,
		typename Ret = std::common_type_t<A, B>>
Ret dot(const vec3<A> &a, const vec3<B> &b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<scalar T, scalar Epsilon>
bool is_normalized(const vec3<T> &vec, Epsilon epsilon) {
	return is_approx(dot(vec, vec), 1, epsilon);
}

template<scalar T>
T length(const vec3<T> &vec) {
	return sqrt(dot(vec, vec));
}

template<scalar T>
vec3<T> normalize(const vec3<T> &vec) {
	return vec * (1 / length(vec));
}

template<scalar To, scalar From,
		typename Ret = std::common_type_t<To, From>>
vec3<Ret> proj(const vec3<To> &to, const vec3<From> &from) {
	return (dot(to, from) / dot(to, to)) * to;
}

#pragma region Component-Wise Math Wrappers

template<scalar X>
vec3<X> fract(const vec3<X> &x) {
	return vec3<X>(fract(x.x), fract(x.y), fract(x.z));
}

template<scalar From, scalar To, scalar Weight,
		typename Ret = std::common_type_t<From, To, Weight>>
vec3<Ret> lerp(const vec3<From> &from, const vec3<To> &to, Weight weight) {
	return vec3<Ret>(
			lerp(from.x, to.x, weight),
			lerp(from.y, to.y, weight),
			lerp(from.z, to.z, weight));
}

template<scalar A, scalar B,
		typename Ret = std::common_type_t<A, B>>
vec3<Ret> max(const vec3<A> &a, const vec3<B> &b) {
	return vec3<Ret>(
			max(a.x, b.x),
			max(a.y, b.y),
			max(a.z, b.z));
}

template<scalar A, scalar B, scalar... Others,
		typename Ret = std::common_type_t<A, B, Others...>>
vec3<Ret> max(const vec3<A> &a, const vec3<B> &b,
		const vec3<Others> &...others) {
	return max(max(a, b), others...);
}

template<scalar A, scalar B,
		typename Ret = std::common_type_t<A, B>>
vec3<Ret> min(const vec3<A> &a, const vec3<B> &b) {
	return vec3<Ret>(
			min(a.x, b.x),
			min(a.y, b.y),
			min(a.z, b.z));
}

template<scalar A, scalar B, scalar... Others,
		typename Ret = std::common_type_t<A, B, Others...>>
vec3<Ret> min(const vec3<A> &a, const vec3<B> &b,
		const vec3<Others> &...others) {
	return min(min(a, b), others...);
}

template<scalar X, scalar Y,
		typename Ret = std::common_type_t<X, Y>>
vec3<Ret> mod(const vec3<X> &x, const vec3<Y> &y) {
	return vec3<Ret>(mod(x.x, y.x, x.y, y.y, x.z, y.z));
}

#pragma endregion

}
