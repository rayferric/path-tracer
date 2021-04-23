#pragma once

#include "pch.hpp"

namespace util {

template<typename T>
std::string to_string(const T &value);

std::string to_string(bool value);

std::string to_string(int32_t value);

std::string to_string(float value, bool trim_zeros = true, int32_t precision = 6);

std::string to_string(double value, bool trim_zeros = true, int32_t precision = 6);

}

#include "strings.inl"
