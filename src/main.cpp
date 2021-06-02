#include "pch.hpp"

#include "math/mat3.hpp"
#include "renderer/model.hpp"
#include "renderer/triangle.hpp"
#include "scene/entity.hpp"
#include "texture/image_texture.hpp"
#include "scene/load_gltf.hpp"

using namespace math;
using namespace renderer;
using namespace scene;

struct trace_state {
	bool hit;
	float min_distance;
	const mesh::vertex *v1, *v2, *v3;
	fvec3 barycentric;
	std::shared_ptr<scene::entity> entity;
};

void trace_entity(trace_state &state, const std::shared_ptr<entity> &entity, const ray &ray) {
	if (auto model = entity->get_component<renderer::model>()) {
		transform transform = entity->get_global_transform().inverse();

		renderer::ray view_ray(
			transform * ray.origin,
			transform.basis * ray.get_dir()
		);

		for (const auto &surface : model->surfaces) {
			const auto &mesh = surface.mesh;

			auto result = mesh->intersect(view_ray);

			if (result.distance < 0)
				continue;
		
			if (!state.hit || result.distance < state.min_distance) {
				state.hit = true;
				state.min_distance = result.distance;

				uvec3 &indices = mesh->triangles[result.index];
				state.v1 = &mesh->vertices[indices.x];
				state.v2 = &mesh->vertices[indices.y];
				state.v3 = &mesh->vertices[indices.z];

				state.barycentric = result.barycentric;

				state.entity = entity;
			}
		}
	}

	for (const auto &child : entity->get_children())
		trace_entity(state, child, ray);
}

fvec3 tonemap_approx_aces(const fvec3 &hdr) {
	constexpr float a = 2.51F;
	const fvec3 b(0.03F);
	constexpr float c = 2.43F;
	const fvec3 d(0.59F);
	const fvec3 e(0.14F);
	return saturate((hdr * (a * hdr + b)) / (hdr * (c * hdr + d) + e));
}

const fvec3 light_dir = normalize(fvec3(1));

fvec3 trace(const std::shared_ptr<entity> &entity, const ray &ray) {
	trace_state state = { false };
	trace_entity(state, entity, ray);

	fvec3 color(0);

	if (state.hit) {
		fvec3 pos(0), normal(0);

		pos += state.barycentric.x * state.v1->position;
		pos += state.barycentric.y * state.v2->position;
		pos += state.barycentric.z * state.v3->position;
		pos = state.entity->get_global_transform() * pos;

		normal += state.barycentric.x * state.v1->normal;
		normal += state.barycentric.y * state.v2->normal;
		normal += state.barycentric.z * state.v3->normal;

		color = normal;

		// renderer::ray light_ray(
		// 	pos + light_dir * math::epsilon * 3,
		// 	light_dir
		// );

		// state = { false };
		// trace_entity(state, entity, light_ray);

		// if (!state.hit)
		// 	color = fvec3(math::max(math::dot(normal, light_dir), 0));

		// color += fvec3(0.1F);

		// color = (state.min_distance - 3.8F) * 0.2F;
		// color = fvec3(pow(color.x, 2));
		// color *= state.barycentric;
	}

	// color = tonemap_approx_aces(color);

	return color;
}

const uint32_t resolution = 2048;

int main() {
	std::shared_ptr<entity> root = load_gltf("assets/dragon/dragon.gltf");

	ray ray(
		fvec3(0, 0, 5),
		fvec3(0, 0, -1)
	);

	auto img = std::make_shared<image>(uvec2(resolution, resolution), 3, false, true);
	
	for (uint32_t x = 0; x < resolution; x++) {
		for (uint32_t y = 0; y < resolution; y++) {
			fvec2 coord = fvec2(static_cast<float>(x) / resolution,
					static_cast<float>(y) / resolution) * 2 - fvec2(1);
			coord.y = -coord.y;
			coord *= math::tan(math::radians(20.0F));
			ray.set_dir(fvec3(coord.x, coord.y, -1));

			if (y == 0)
				std::cout << "Drawing row: " << x << std::endl;

			uvec2 pixel(x, y);

			img->write(pixel, 0, 0);
			img->write(pixel, 1, 0);
			img->write(pixel, 2, 0);

			fvec3 color = trace(root, ray);

			img->write(pixel, 0, color.x);
			img->write(pixel, 1, color.y);
			img->write(pixel, 2, color.z);
		}
	}

	img->save("render.png");

	return 0;
}
