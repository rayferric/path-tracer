#include "renderer/mesh.hpp"

using namespace math;

namespace renderer {

kd_tree_node *instantiate_kd_tree_node(
		const aabb &aabb,
		const std::vector<triangle> &triangles,
		const std::vector<uint32_t> &indices,
		uint8_t depth) {
	
	// Create leaf node once
	// we've reached certain depth
	if (depth == 0) {
		auto node = new kd_tree_leaf;
		node->aabb = aabb;
		
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

void mesh::build_kd_tree(uint8_t depth) {
	// Create root AABB
	renderer::aabb aabb;
	aabb.clear();
	for (vertex &v : vertices)
		aabb.add_point(v.position);

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

}
