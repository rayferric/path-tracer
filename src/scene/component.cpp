#include "scene/component.hpp"

namespace scene {

std::shared_ptr<entity> component::get_entity() const {
	return entity.lock();
}

}
