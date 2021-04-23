namespace math {

// Basic

template<typename X>
inline auto sqrt(X x) {
	return std::sqrt(x);
}

template<typename X, typename Power>
inline auto pow(X x, Power power) {
	return std::pow(x, power);
}

template<typename Base, typename X>
inline auto log(Base base, X x) {
	return std::log(x) / std::log(base);
}

template<typename X>
inline auto log(X x) {
	return std::log(x);
}

template<typename X>
inline auto log2(X x) {
	return std::log2(x);
}

template<typename Base, typename X>
inline auto exp(Base base, X x) {
	return std::pow(base, x);
}

template<typename X>
inline auto exp(X x) {
	return std::exp(x);
}

template<typename X>
inline auto exp2(X x) {
	return std::exp2(x);
}

template<typename X>
inline auto abs(X x) {
	return std::abs(x);
}

template<typename A, typename B, typename Epsilon>
bool is_approx(A a, B b, Epsilon epsilon) {
	return a == b || math::abs(a - b) < epsilon;
}

// Trigonometry

template<typename Angle>
inline auto sin(Angle angle) {
	return std::sin(angle);
}

template<typename Angle>
inline auto cos(Angle angle) {
	return std::cos(angle);
}

template<typename Angle>
inline auto tan(Angle angle) {
	return std::tan(angle);
}

template<typename Y>
inline auto asin(Y y) {
	return std::asin(y);
}

template<typename X>
inline auto acos(X x) {
	return std::acos(x);
}

template<typename YOverX>
inline auto atan(YOverX y_over_x) {
	return std::atan(y_over_x);
}

template<typename Y, typename X>
inline auto atan2(Y y, X x) {
	return std::atan2(y, x);
}

// Hyperbolic

template<typename DoubleArea>
inline auto sinh(DoubleArea double_area) {
	return std::sinh(double_area);
}

template<typename DoubleArea>
inline auto cosh(DoubleArea double_area) {
	return std::cosh(double_area);
}

template<typename DoubleArea>
inline auto tanh(DoubleArea double_area) {
	return std::tanh(double_area);
}

template<typename Y>
inline auto asinh(Y y) {
	return std::asinh(y);
}

template<typename X>
inline auto acosh(X x) {
	return std::acosh(x);
}

template<typename YOverX>
inline auto atanh(YOverX y_over_x) {
	return std::atanh(y_over_x);
}

// Miscellaneous

template<typename A, typename B>
inline auto min(A a, B b) {
	return b < a ? b : a;
}

template<typename A, typename B>
inline auto max(A a, B b) {
	return b > a ? b : a;
}

template<typename X, typename Min, typename Max>
inline auto clamp(X x, Min min, Max max) {
	return math::min(math::max(x, min), max);
}

template<typename X>
inline auto saturate(X x) {
	return clamp(x, 0, 1);
}

template<typename A, typename B, typename Weight>
inline auto lerp(A a, B b, Weight weight) {
	return a + (b - a) * weight;
}

template<typename Edge, typename X>
inline auto step(Edge edge, X x) {
	return x >= edge ? 1 : 0;
}

template<typename From, typename To, typename X>
inline auto smoothstep(From from, To to, X x) {
	inline auto t = saturate((x - from) / (to - from));
    return t * t * (3 - 2 * t);
}

// Rounding

template<typename X>
inline auto floor(X x) {
	return std::floor(x);
}

template<typename X>
inline auto ceil(X x) {
	return std::ceil(x);
}

template<typename X>
inline auto round(X x) {
	return std::round(x);
}

template<typename X>
inline auto fract(X x) {
	return x - floor(x);
}

// Conversion

template<typename Degrees>
inline auto radians(Degrees degrees) {
	return degrees * (math::pi / 180);
}

template<typename Radians>
inline auto degrees(Radians radians) {
	return radians * (180 / math::pi);
}

}
