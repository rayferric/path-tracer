#include "scene/entity.hpp"

namespace scene {

entity::entity() : name("entity") {}

const std::string &entity::get_name() const {
	return name;
}

void entity::set_name(const std::string &name) {
	this->name = name;
}

const std::shared_ptr<entity> &
		entity::get_parent() const {
	return parent;
}

void entity::set_parent(const
		std::shared_ptr<entity> &parent) {
	
}

const std::vector<std::shared_ptr<entity>> &
		entity::get_children();

const std::shared_ptr<entity> &entity::find_child(
		const std::string &path) const;

transform &entity::get_local_transform();

const transform &entity::get_global_transform();

void entity::invalidate_global_transform();

}