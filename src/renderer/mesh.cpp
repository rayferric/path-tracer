#include "renderer/mesh.hpp"

using namespace math;

namespace renderer {

bvh_node *instantiate_bvh_node(uint32_t triangles_per_node, float tolerance, const std::vector<triangle> &triangles, const std::vector<uint32_t> &indices, uint32_t split_axis) {
	// Instantiate BVH node
	bvh_node *node;
	if (triangles.size() > triangles_per_node)
		node = new bvh_branch;
	else
		node = new bvh_leaf;

	// Initialize AABB which is common to both types
	node->aabb.clear();
	for (const renderer::triangle &triangle : triangles) {
		for (size_t i = 0; i < 3; i++)
			node->aabb.add_point(*(&triangle.a + i));
	}

	// Initialize remaining fields
	if (triangles.size() > triangles_per_node) {
		// Split triagle list in two
		// Some triangles might get
		// assigned to both L and R sets

		std::vector<triangle> ltriangles, rtriangles;
		std::vector<uint32_t> lindices, rindices;

		ltriangles.reserve(triangles.size());
		rtriangles.reserve(triangles.size());
		lindices.reserve(indices.size());
		rindices.reserve(indices.size());

		// Pick initial split axis
		fvec3 widths = node->aabb.max - node->aabb.min;
		if (split_axis == -1)
			split_axis = std::max_element(&widths.x,
					&widths.x + 3) - &widths.x;
		float split_pos = node->aabb.min[split_axis]
				+ (widths[split_axis] * 0.5F);

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

		if (ltriangles.size() < triangles.size() && rtriangles.size() < triangles.size()) {
			std::cout << "Node split in two: " << triangles.size() << " -> " << ltriangles.size() << " | " << rtriangles.size() << " (Split axis: " << split_axis << ")" << std::endl;

			ltriangles.shrink_to_fit();
			rtriangles.shrink_to_fit();
			lindices.shrink_to_fit();
			rindices.shrink_to_fit();

			split_axis = (split_axis + 1) % 3;

			if (ltriangles.size() > 0)
				static_cast<bvh_branch *>(node
						)->left.reset(instantiate_bvh_node(
						triangles_per_node, tolerance, ltriangles, lindices, split_axis));

			if (rtriangles.size() > 0)
				static_cast<bvh_branch *>(node
						)->right.reset(instantiate_bvh_node(
						triangles_per_node, tolerance, rtriangles, rindices, split_axis));
			
			return node;
		}

		// Replace unsuccessful branch with a leaf
		bvh_node *new_node = new bvh_leaf;
		new_node->aabb = node->aabb;
		delete node;
		node = new_node;
	}

	// Initialize leaf
	static_cast<bvh_leaf *>(node
			)->triangles = triangles;
	static_cast<bvh_leaf *>(node
			)->indices = indices;
	std::cout << "Finalized branch - leaf size: " << triangles.size() << std::endl;

	return node;
}

void mesh::build_bvh(uint32_t triangles_per_node, float tolerance) {
	std::vector<triangle> triangles;
	triangles.reserve(this->triangles.size());

	for (size_t i = 0; i < this->triangles.size(); i++) {
		uvec3 indices = this->triangles[i];
		triangles.push_back(triangle(
			vertices[indices.x].position,
			vertices[indices.y].position,
			vertices[indices.z].position)
		);
	}

	std::vector<uint32_t> indices(this->triangles.size());
	std::iota(indices.begin(), indices.end(), 0);

	bvh.reset(instantiate_bvh_node(triangles_per_node, tolerance, triangles, indices, -1));
}

}
