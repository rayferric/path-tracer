#pragma once

#include "pch.hpp"

#include "scene/transform.hpp"

namespace scene {

class entity {
public:
	entity();

	const std::string &get_name() const;

	void set_name(const std::string &name);

	const std::shared_ptr<entity> &get_parent() const;

	void set_parent(const std::shared_ptr<entity> &parent);

	const std::vector<std::shared_ptr<entity>> &get_children();

	const std::shared_ptr<entity> &find_child(const std::string &path) const;

	transform &get_local_transform();

	const transform &get_global_transform();

private:
	std::string name;

	std::weak_ptr<entity> parent;
	std::vector<std::shared_ptr<entity>> children;

	transform local_transform;
	transform global_transform_cache;
	bool global_transform_valid;

	std::vector<std::shared_ptr<component>> components;

	void invalidate_global_transform();
};

}
