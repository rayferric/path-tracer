#include "renderer/mesh.hpp"

using namespace math;

namespace renderer {

bvh_node *instantiate_bvh_node(uint32_t triangles_per_node, float tolerance, const std::vector<triangle> &triangles) {
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
		ltriangles.reserve(triangles.size());
		rtriangles.reserve(triangles.size());

		// Pick initial split axis
		fvec3 widths = node->aabb.max - node->aabb.min;
		size_t split_axis = std::max_element(&widths.x,
					&widths.x + 3) - &widths.x;

		// Will try another axis if the
		// quality isn't good enough
		int32_t attempt = 0;
		do {
			ltriangles.clear();
			rtriangles.clear();

			float split_pos = node->aabb.min[split_axis]
					+ (widths[split_axis] * 0.5F);

			for (const renderer::triangle &triangle : triangles) {
				bool lassign = false, rassign = false;
				
				for (size_t i = 0; i < 3; i++) {
					fvec3 vertex = *(&triangle.a + i);
					if (vertex[split_axis] < split_pos)
						lassign = true;
					else
						rassign = true;
				}

				if (lassign)
					ltriangles.push_back(triangle);
				if (rassign)
					rtriangles.push_back(triangle);
			}

			split_axis = (split_axis + 1) % 3;
		} while ((
				ltriangles.size() > triangles.size() * tolerance ||
				rtriangles.size() > triangles.size() * tolerance 
				) && ++attempt < 3);

		std::cout << attempt << std::endl;

		if (attempt != 3) {
			std::cout << "Node split in two: " << triangles.size() << " -> " << ltriangles.size() << " | " << rtriangles.size() << " (Split axis: " << split_axis << ")" << std::endl;

			static_cast<bvh_branch *>(node
					)->left.reset(instantiate_bvh_node(
					triangles_per_node, tolerance, ltriangles));
			static_cast<bvh_branch *>(node
					)->right.reset(instantiate_bvh_node(
					triangles_per_node, tolerance, rtriangles));
			
			return node;
		}

		// Replace unsuccessful branch node with leaf
		bvh_node *new_node = new bvh_leaf;
		new_node->aabb = node->aabb;
		delete node;
		node = new_node;
	}

	// Initialize leaf
	static_cast<bvh_leaf *>(node
			)->triangles = triangles;
	std::cout << "Finalizing branch, leaf size: " << triangles.size() << std::endl;

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

	bvh.reset(instantiate_bvh_node(triangles_per_node, tolerance, triangles));
}

}
