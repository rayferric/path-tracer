namespace math {

template<scalar T>
vec2<T>::vec2() : vec2(0) {}

template<scalar T>
vec2<T>::vec2(T all) : x(all), y(all) {}

template<scalar T>
vec2<T>::vec2(T x, T y) : x(x), y(y) {}

#pragma region Operators

// Vector + Vector

template<scalar L, scalar R>
auto operator+(const vec2<L> &lhs, const vec2<R> &rhs) {
	return vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

template<scalar L, scalar R>
auto operator-(const vec2<L> &lhs, const vec2<R> &rhs) {
	return vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

template<scalar L, scalar R>
auto operator*(const vec2<L> &lhs, const vec2<R> &rhs) {
	return vec2(lhs.x * rhs.x, lhs.y * rhs.y);
}

template<scalar L, scalar R>
auto operator/(const vec2<L> &lhs, const vec2<R> &rhs) {
	return vec2(lhs.x / rhs.x, lhs.y / rhs.y);
}

template<scalar L, scalar R>
auto operator%(const vec2<L> &lhs, const vec2<R> &rhs) {
	return vec2(lhs.x % rhs.x, lhs.y % rhs.y);
}

#pragma endregion

template<scalar T>
vec2<T> floor(const vec2<T> &vec) {
	return vec2<T>(math::floor(vec.x), math::floor(vec.y));
}

}
