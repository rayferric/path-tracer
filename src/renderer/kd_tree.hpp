#pragma once

#include "pch.hpp"

#include "renderer/aabb.hpp"
#include "renderer/ray.hpp"
#include "renderer/triangle.hpp"

namespace renderer {

struct kd_tree_node {
	struct intersection {
		float distance = -1;
		math::fvec3 barycentric;
		uint32_t index;
	};
	
	renderer::aabb aabb;

	virtual intersection intersect(const ray &ray) = 0;
};

struct kd_tree_branch : public kd_tree_node {
	std::shared_ptr<kd_tree_node> left, right;

	kd_tree_node::intersection intersect(const ray &ray) override;
};

struct kd_tree_leaf : public kd_tree_node {
	std::vector<triangle> triangles;
	std::vector<uint32_t> indices;

	kd_tree_node::intersection intersect(const ray &ray) override;
};

}
