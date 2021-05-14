#include "scene/component.hpp"

namespace scene {

std::shared_ptr<entity> component::get_parent() const {
	return parent.lock();
}

}
