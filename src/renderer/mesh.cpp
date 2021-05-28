#include "renderer/mesh.hpp"

using namespace math;

namespace renderer {

kd_tree_node *instantiate_kd_tree_node(
		uint8_t split_axis, float split_pos,
		const std::vector<triangle> &triangles,
		const std::vector<uint32_t> &indices,
		uint8_t depth) {
	
	// Create leaf node once
	// we've reached certain depth
	if (depth == 0) {
		auto node = new kd_tree_leaf;
		
		(node->triangles = triangles).shrink_to_fit();
		(node->indices = indices).shrink_to_fit();

		return node;
	}

	auto node = new kd_tree_branch;
	node->aabb = aabb;

	// We split in the middle
	fvec3 widths = aabb.max - aabb.min;
	size_t split_axis = std::max_element(&widths.x,
			&widths.x + 3) - &widths.x;
	float split_pos = aabb.min[split_axis] +
			widths[split_axis] * 0.5F;

	// Split the AABB
	renderer::aabb laabb, raabb;
	laabb = raabb = aabb;

	// Left node contains objects
	// with smaller position
	laabb.max[split_axis] = split_pos;
	raabb.min[split_axis] = split_pos;

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
			if (vertex[split_axis] < split_pos)
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

	// Compute initial split axis
	fvec3 widths = aabb.max - aabb.min;
	size_t split_axis = std::max_element(&widths.x,
			&widths.x + 3) - &widths.x;

	// Build the kD tree
	kd_tree.reset(instantiate_kd_tree_node(aabb,
			triangles, indices, depth));
}

mesh::intersection mesh::intersect(const ray &ray) {
	intersection result;
	std::stack<std::tuple<kd_tree_node *,
			float, float>> stack;

	while (result.distance < 0 && !stack.empty()) {
		auto [node, split_min, split_max] = stack.top();
		stack.pop();

		while (typeid(*node) == typeid(kd_tree_branch)) {
			auto branch = static_cast<kd_tree_branch *>(node);

			float split = (branch->split - ray.origin[branch
					->axis]) / ray.get_dir()[branch->axis];
			
			kd_tree_node *first, *second;

			if (split - ray.origin[branch->axis] >= 0) {
				first = branch->left.get();
				second = branch->right.get();
			} else {
				first = branch->right.get();
				second = branch->left.get();
			}

			if (split >= split_max || split < 0)
				node = first;
			else if (split <= split_min)
				node = second;


		}
	}
}

}
