#include "util/strings.hpp"

#include "math/math.hpp"

namespace util {

template<typename T>
static std::string format_fp(T value, bool trim_zeros, int32_t precision) {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision) << value;
	
	std::string str = ss.str();
	std::string trimmed = str;
	trimmed.erase(trimmed.find_last_not_of('0') + 1);

	if (trim_zeros) {
		str = trimmed;

		if (str == "-0.")
			str = "0";
		else if (str[str.size() - 1] == '.')
			str.pop_back();
	} else if (trimmed == "-0.")
		str.erase(str.begin());

	return str;
}

std::string to_string(bool value) {
	return value ? "true" : "false";
}

std::string to_string(int32_t value) {
	return std::to_string(value);
}

std::string to_string(float value, bool trim_zeros, int32_t precision) {
	return format_fp(value, trim_zeros, precision);
}

std::string to_string(double value, bool trim_zeros, int32_t precision) {
	return format_fp(value, trim_zeros, precision);
}

}
