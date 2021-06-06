#pragma once

#include "pch.hpp"

namespace scene {

class entity;

class component {
public:
	std::shared_ptr<entity> get_parent() const;

	virtual ~component() {}

private:
	friend entity;

	std::weak_ptr<entity> parent;
};

}

#include "scene/entity.hpp"
