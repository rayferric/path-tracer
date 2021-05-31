#include "renderer/mesh.hpp"

using namespace math;

namespace renderer {

kd_tree_node *instantiate_kd_tree_node(
		const aabb &aabb,
		const std::vector<triangle> &triangles,
		const std::vector<uint32_t> &indices,
		uint8_t depth) {
	// TODO: Use surface area heuristics

	// Create leaf node once
	// we've reached certain depth
	if (depth == 0) {
		auto node = new kd_tree_leaf;
		
		(node->triangles = triangles).shrink_to_fit();
		(node->indices = indices).shrink_to_fit();

		return node;
	}

	auto node = new kd_tree_branch;

	// We split in the middle
	fvec3 widths = aabb.max - aabb.min;
	node->axis = std::max_element(&widths.x,
			&widths.x + 3) - &widths.x;
	node->split = aabb.min[node->axis] +
			widths[node->axis] * 0.5F;

	// Split the AABB
	renderer::aabb laabb, raabb;
	laabb = raabb = aabb;

	// Left node contains objects
	// with smaller position
	laabb.max[node->axis] = node->split;
	raabb.min[node->axis] = node->split;

	// Split triangles and their indices
	std::vector<triangle> ltriangles, rtriangles;
	ltriangles.reserve(triangles.size());
	rtriangles.reserve(triangles.size());

	std::vector<uint32_t> lindices, rindices;
	lindices.reserve(indices.size());
	rindices.reserve(indices.size());

	for (size_t i = 0; i < triangles.size(); i++) {
		const triangle &triangle = triangles[i];
		uint32_t index = indices[i];

		bool lassign = false, rassign = false;
		
		for (size_t i = 0; i < 3; i++) {
			const fvec3 &vertex = *(&triangle.a + i);
			if (vertex[node->axis] < node->split)
				lassign = true;
			else
				rassign = true;
		}

		if (lassign) {
			ltriangles.push_back(triangle);
			lindices.push_back(index);
		}

		if (rassign) {
			rtriangles.push_back(triangle);
			rindices.push_back(index);
		}
	}

	if (ltriangles.size() > 0) {
		node->left.reset(instantiate_kd_tree_node(
				laabb, ltriangles, lindices,
				depth - 1));
	}

	if (rtriangles.size() > 0) {
		node->right.reset(instantiate_kd_tree_node(
				raabb, rtriangles, rindices,
				depth - 1));
	}

	return node;
}

void mesh::recalculate_aabb() {
	aabb.clear();
	for (vertex &v : vertices)
		aabb.add_point(v.position);
}

void mesh::build_kd_tree(uint8_t depth) {
	// Convert root vertices to triangles
	std::vector<triangle> triangles;
	triangles.reserve(this->triangles.size());

	for (size_t i = 0; i < this->triangles.size(); i++) {
		uvec3 indices = this->triangles[i];
		triangles.push_back(triangle(
			vertices[indices.x].position,
			vertices[indices.y].position,
			vertices[indices.z].position
		));
	}

	// Initialize root triangle indices
	std::vector<uint32_t> indices(this->triangles.size());
	std::iota(indices.begin(), indices.end(), 0);

	// Build the kD tree
	kd_tree.reset(instantiate_kd_tree_node(aabb,
			triangles, indices, depth));
}

fvec3 hsv2rgb(const fvec3 &c) {
    fvec4 K = fvec4(1, 0.666667F, 0.333333F, 3);
    fvec3 p = abs(fract(fvec3(c.x + K.x, c.x + K.y, c.x + K.z)) * 6 - fvec3(K.w));
    return c.z * lerp(fvec3(K.x), saturate(p - fvec3(K.x)), c.y);
}

mesh::intersection mesh::intersect(const ray &ray) const {
	auto result = aabb.intersect(ray);
	if (result.far < 0)
		return {};

	std::stack<std::tuple<const kd_tree_node *, float, float>> stack;
	stack.push(std::make_tuple(kd_tree.get(), result.near, result.far));

	while (!stack.empty()) {
		auto [node, min_dist, max_dist] = stack.top();
		stack.pop();

		// Explore down the tree until we reach a leaf
		while (node && typeid(*node) == typeid(kd_tree_branch)) {
			auto branch = static_cast<const kd_tree_branch *>(node);

			// Distance to the split plane
			float split_dist = (branch->split - ray.origin[branch
					->axis]) / ray.get_dir()[branch->axis];
			
			const kd_tree_node *first, *second;

			if (ray.origin[branch->axis] < branch->split) {
				first = branch->left.get();
				second = branch->right.get();
			} else {
				first = branch->right.get();
				second = branch->left.get();
			}
			
			// If ray points away from the split plane
			// or if the back of the AABB is closer
			// than distance to the split plane,
			// we've hit just the first node
			if (split_dist < 0 || split_dist > max_dist)
				node = first;

			// When node's AABB is further away than the split plane
			// then we've hit second node only
			else if (split_dist < min_dist)
				node = second;

			// Otherwise we've hit them both
			else {
				if (second)
					stack.push(std::make_tuple(second, split_dist, max_dist));

				node = first;
				max_dist = split_dist;
			}
		}
		
		if (!node)
			continue;

		// It's a leaf node
		auto leaf = static_cast<const kd_tree_leaf *>(node);

		triangle::intersection nearest_hit;
		uint32_t index = 0;

		for (uint32_t i = 0; i < leaf->triangles.size(); i++) {
			auto hit = leaf->triangles[i].intersect(ray);
			if (hit.distance >= 0 && (hit.distance <
					nearest_hit.distance || nearest_hit.distance < 0)) {
				nearest_hit = hit;
				index = i;
			}
		}

		if (nearest_hit.distance < min_dist ||
				nearest_hit.distance > max_dist)
			continue;

		return { nearest_hit.distance, nearest_hit.barycentric, leaf->indices[index] };
	}

	return {};
}

}
