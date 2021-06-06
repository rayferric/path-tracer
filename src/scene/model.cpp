#include "scene/model.hpp"

#include "math/vec3.hpp"

using namespace math;

namespace core {

bool model::intersection::has_hit() const {
	return distance >= 0;
}

void model::recalculate_aabb() {
	aabb.clear();
	for (auto &surface : surfaces) {
		aabb.add(surface.mesh->aabb);
	}
}

model::intersection model::intersect(
		const ray &ray) const {
	scene::transform transform = get_parent()->
			get_global_transform();
	scene::transform inv_transform
			= transform.inverse();

	core::ray view_ray(
		inv_transform * ray.origin,
		inv_transform.basis * ray.get_dir()
	);

	if (!aabb.intersect(view_ray).has_hit())
		return {};

	mesh::intersection nearest_hit;
	const surface *hit_surface;

	for (const auto &surface : surfaces) {
		const auto &mesh = surface.mesh;

		auto hit = mesh->intersect(view_ray);

		if (!hit.has_hit())
			continue;
	
		if (!nearest_hit.has_hit() ||
				hit.distance < nearest_hit.distance) {
			nearest_hit = hit;
			hit_surface = &surface;
		}
	}

	if (!nearest_hit.has_hit())
		return {};

	intersection result;

	const auto &mesh = hit_surface->mesh;

	uvec3 &indices = mesh->triangles[nearest_hit.index];
	auto &v1 = mesh->vertices[indices.x];
	auto &v2 = mesh->vertices[indices.y];
	auto &v3 = mesh->vertices[indices.z];

	fmat3 normal_matrix = transpose(inverse(transform.basis));

	result.position = transform * (
			v1.position * nearest_hit.barycentric.x +
			v2.position * nearest_hit.barycentric.y +
			v3.position * nearest_hit.barycentric.z);
	result.tex_coord =
			v1.tex_coord * nearest_hit.barycentric.x +
			v2.tex_coord * nearest_hit.barycentric.y +
			v3.tex_coord * nearest_hit.barycentric.z;
	result.normal = normal_matrix * (
			v1.normal * nearest_hit.barycentric.x +
			v2.normal * nearest_hit.barycentric.y +
			v3.normal * nearest_hit.barycentric.z);
	result.normal = normal_matrix * (
			v1.normal * nearest_hit.barycentric.x +
			v2.normal * nearest_hit.barycentric.y +
			v3.normal * nearest_hit.barycentric.z);

	result.material = hit_surface->material;

	return result;
}

};
