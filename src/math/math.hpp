#pragma once

#include "pch.hpp"

namespace math {

constexpr double e       = std::numbers::e;
constexpr float  epsilon = 0.0001;
constexpr double phi     = std::numbers::phi;
constexpr double pi      = std::numbers::pi;
constexpr double sqrt2   = std::numbers::sqrt2;
constexpr double sqrt3   = std::numbers::sqrt3;

// Basic

template<typename X>
inline auto sqrt(X x);

template<typename X, typename Power>
inline auto pow(X x, Power power);

template<typename Base, typename X>
inline auto log(Base base, X x);

template<typename X>
inline auto log(X x);

template<typename X>
inline auto log2(X x);

template<typename Base, typename X>
inline auto exp(Base base, X x);

template<typename X>
inline auto exp(X x);

template<typename X>
inline auto exp2(X x);

template<typename X>
inline auto abs(X x);

template<typename A, typename B, typename Epsilon = decltype(epsilon)>
bool is_approx(A a, B b, Epsilon epsilon = epsilon);

// Trigonometry

template<typename Angle>
inline auto sin(Angle angle);

template<typename Angle>
inline auto cos(Angle angle);

template<typename Angle>
inline auto tan(Angle angle);

template<typename Y>
inline auto asin(Y y);

template<typename X>
inline auto acos(X x);

template<typename YOverX>
inline auto atan(YOverX y_over_x);

template<typename Y, typename X>
inline auto atan2(Y y, X x);

// Hyperbolic

template<typename DoubleArea>
inline auto sinh(DoubleArea double_area);

template<typename DoubleArea>
inline auto cosh(DoubleArea double_area);

template<typename DoubleArea>
inline auto tanh(DoubleArea double_area);

template<typename Y>
inline auto asinh(Y y);

template<typename X>
inline auto acosh(X x);

template<typename YOverX>
inline auto atanh(YOverX y_over_x);

// Miscellaneous

template<typename A, typename B>
inline auto min(A a, B b);

template<typename A, typename B, typename... Others>
inline auto min(A a, B b, Others ...others);

template<typename A, typename B>
inline auto max(A a, B b);

template<typename A, typename B, typename... Others>
inline auto max(A a, B b, Others ...others);

template<typename X, typename Min, typename Max>
inline auto clamp(X x, Min min, Max max);

template<typename X>
inline auto saturate(X x);

template<typename A, typename B, typename Weight>
inline auto lerp(A a, B b, Weight weight);

template<typename Edge, typename X>
inline int32_t step(Edge edge, X x);

template<typename From, typename To, typename X>
inline auto smoothstep(From from, To to, X x);

// Rounding

template<typename X>
inline auto floor(X x);

template<typename X>
inline auto ceil(X x);

template<typename X>
inline auto round(X x);

template<typename X>
inline auto fract(X x);

// Conversion

template<typename Degrees>
inline auto radians(Degrees degrees);

template<typename Radians>
inline auto degrees(Radians radians);

}

#include "math.inl"
