#include "renderer/mesh.hpp"

namespace renderer {

void mesh::recalculate_aabb() {
	aabb.clear();

	for (vertex &v : vertices)
		aabb.add_point(v.position);
}

}
