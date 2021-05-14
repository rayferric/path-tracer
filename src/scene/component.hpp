#pragma once

#include "pch.hpp"

namespace scene {

class entity;

class component {
public:
	std::shared_ptr<entity> get_parent() const;

private:
	friend entity;

	std::weak_ptr<entity> parent;
};

}
