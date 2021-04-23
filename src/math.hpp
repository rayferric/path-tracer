#pragma once

#include <numbers>

namespace math {

constexpr double e = std::numbers::e;
constexpr double epsilon = 0.0001;
constexpr double phi = std::numbers::phi;
constexpr double pi = std::numbers::pi;
constexpr double sqrt2 = std::numbers::sqrt2;
constexpr double sqrt3 = std::numbers::sqrt3;

template<typename X, typename Power>
inline auto pow(X x, Power power);

}

#include "math.inl"
