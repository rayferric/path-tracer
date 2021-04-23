#include <cmath>

namespace math {

template<typename X, typename Power>
inline auto pow(X x, Power power) {
	return std::pow(x, power);
}

}
