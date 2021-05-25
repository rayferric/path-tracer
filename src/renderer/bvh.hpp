#pragma once

#include "pch.hpp"

#include "renderer/aabb.hpp"
#include "renderer/ray.hpp"
#include "renderer/triangle.hpp"

namespace renderer {

struct bvh_node {
	struct intersection {
		float distance;
		math::fvec3 barycentric;
		uint32_t index;
	};
	
	renderer::aabb aabb;

	virtual intersection intersect(const ray &ray) = 0;
};

struct bvh_branch : public bvh_node {
	std::shared_ptr<bvh_node> left, right;

	bvh_node::intersection intersect(const ray &ray) override;
};

struct bvh_leaf : public bvh_node {
	std::vector<triangle> triangles;
	std::vector<uint32_t> indices;

	bvh_node::intersection intersect(const ray &ray) override;
};

}
