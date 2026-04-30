#include "core/renderer.hpp"

#include "core/material.hpp"
#include "core/mesh.hpp"
#include "core/pbr.hpp"
#include "geometry/ray.hpp"
#include "image/image.hpp"
#include "image/image_texture.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/mat3.hpp"
#include "math/math.hpp"
#include "scene/camera.hpp"
#include "scene/entity.hpp"
#include "scene/model.hpp"
#include "scene/sun_light.hpp"
#include "scene/transform.hpp"
#include "util/rand_cone_vec.hpp"
#include "util/thread_pool2.hpp"

using namespace geometry;
using namespace math;
using namespace scene;

namespace core {

static float rand() {
	static std::mt19937 rng{std::random_device{}()};
	static std::uniform_real_distribution<float> dist(0, 1);

	return dist(rng);
}

static fvec3 rand_sphere_dir() {
	float s = rand() * 2 * math::pi;
	float t = rand() * 2 - 1;

	return fvec3(math::sin(s), math::cos(s), t) / math::sqrt(t * t + 1);
}

static fvec2 equirectangular_proj(const fvec3 &dir) {
	return fvec2(
		math::atan2(dir.z, dir.x) * 0.1591F + 0.5F,
		math::asin (dir.y)        * 0.3183F + 0.5F
	);
}

static fvec3 tonemap_approx_aces(const fvec3 &hdr) {
	constexpr float a = 2.51F;
	constexpr fvec3 b(0.03F);
	constexpr float c = 2.43F;
	constexpr fvec3 d(0.59F);
	constexpr fvec3 e(0.14F);
	return saturate((hdr * (a * hdr + b)) / (hdr * (c * hdr + d) + e));
}

static fvec3 reflect(const fvec3 &incident, const fvec3 &normal) {
	return incident - 2 * dot(normal, incident) * normal;
}

std::shared_ptr<image::image> renderer::init_hdr_accum() const {
	// return 4 channel hdr no srgb
	return std::make_shared<image::image>(resolution, 4, true, false);
}

std::shared_ptr<image::image> renderer::tonemap_output(const std::shared_ptr<image::image> &hdr) const {
	auto sdr = std::make_shared<image::image>(resolution, 4, false, true);

	for (uint32_t y = 0; y < resolution.y; y++) {
		for (uint32_t x = 0; x < resolution.x; x++) {
			uvec2 xy(x, y);

			fvec3 color;
			color[0] = hdr->read(xy, 0);
			color[1] = hdr->read(xy, 1);
			color[2] = hdr->read(xy, 2);
			float alpha = hdr->read(xy, 3);

			color = tonemap_approx_aces(color);

			sdr->write(xy, 0, color.x);
			sdr->write(xy, 1, color.y);
			sdr->write(xy, 2, color.z);
			sdr->write(xy, 3, alpha);
		}
	}

	return sdr;
}

void renderer::render_sample(const std::shared_ptr<image::image> &hdr, uint32_t sample_idx) const {
	util::thread_pool pool;

	for (uint32_t y = 0; y < resolution.y; y++) {
		pool.submit([&, y] () {
			for (uint32_t x = 0; x < resolution.x; x++) {
				uvec2 xy(x, y);

				// Do not offset the first sample so we can get a consistent alpha mask for smart blending
				fvec2 aa_offset = fvec2(rand(), rand());

				fvec2 ndc = ((fvec2(xy) + aa_offset) /
						resolution) * 2 - fvec2::one;
				ndc.y = -ndc.y;
				float ratio = static_cast<float>(resolution.x) / resolution.y;

				ray ray = camera->get_ray(ndc, ratio);
				fvec4 new_sample = trace(bounce_count, ray);

				fvec4 old_samples = hdr->read4(xy);

				fvec3 new_color = math::fvec3(new_sample);
				fvec3 old_color = math::fvec3(old_samples);
				float new_alpha = new_sample.w;
				float old_alpha = old_samples.w;

				// Smart blending - needed for transparent background
				if (transparent_background && new_sample.w > 0.5 && old_alpha == 0.0f) {
					// If an opaque sample will claim this pixel
					// pixels[x][y].color = fvec3(new_sample); // Overwrite the color
					// pixels[x][y].alpha = 1 / (sample_idx + 1); // And blend the alpha
					old_color = new_color;
					old_alpha = 1.0f / (sample_idx + 1);
				} else if (transparent_background && new_sample.w < 0.5 && old_alpha != 0.0f) {
					// If a transparent sample encounters a claimed pixel
					// pixels[x][y].alpha = pixels[x][y].alpha * sample_idx + new_sample.w; // Blend only alpha
					// pixels[x][y].alpha /= sample_idx + 1;
					old_color = old_color; // Preserve color and blend only alpha
					old_alpha = (old_alpha * sample_idx + new_sample.w) / (sample_idx + 1); // Blend alpha
				} else if (transparent_background && new_sample.w < 0.5) {
					// If transparent sample blends with an unclaimed pixel
					// Do nothing and preserve the default transparent black color
				} else {
					// Otherwise if an opaque sample blends with a claimed pixel (or transparent background is disabled)
					// pixels[x][y].color = pixels[x][y].color * sample_idx + fvec3(new_sample); // Blend color
					// pixels[x][y].color /= sample_idx + 1;
					// pixels[x][y].alpha = pixels[x][y].alpha * sample_idx + new_sample.w; // Blend alpha
					// pixels[x][y].alpha /= sample_idx + 1;
					old_color = (old_color * sample_idx + new_color) / (sample_idx + 1); // Blend color
					old_alpha = (old_alpha * sample_idx + new_alpha) / (sample_idx + 1); // Blend alpha
				}

				hdr->write4(xy, fvec4(old_color, old_alpha));
			}
		});
	}
	pool.wait();
}

void renderer::render(const std::filesystem::path &path) const {
	auto hdr = init_hdr_accum();

	for (uint32_t sample = 0; sample < sample_count; sample++) {
		std::cout << "Drawing sample " << sample + 1 << " out of " << sample_count << '.' << std::endl;
		render_sample(hdr, sample);
		
		auto sdr = tonemap_output(hdr);
		sdr->save(path);
	}
}

fvec3 renderer::intersect_result::get_normal() const {
	vec3 binormal = cross(normal, tangent);
	fmat3 tbn(tangent, binormal, normal);

	return tbn * material->get_normal(tex_coord);
}

fvec4 renderer::trace(uint8_t bounce, const ray &ray) const {
	if (bounce == 0)
		return fvec4::future;

	auto result = intersect(ray);

	if (!result.hit) {
		float alpha = transparent_background ? 0 : 1;

		if (environment)
			return fvec4(fvec3(environment->sample(equirectangular_proj(ray.get_dir()))) * environment_factor, alpha);
		else
			return fvec4(environment_factor, alpha);
	}

	if (visualize_kd_tree_depth)
		return fvec4(result.position, 1);

	// Material properties

	fvec3 albedo = result.material->get_albedo(result.tex_coord);
	float opacity = result.material->get_opacity(result.tex_coord);
	float roughness = result.material->get_roughness(result.tex_coord);
	float metallic = result.material->get_metallic(result.tex_coord);
	fvec3 emissive = result.material->get_emissive(result.tex_coord) * 10; // DEBUG
	float ior = result.material->ior;

	// Handle opacity
	if (!math::is_approx(opacity, 1) && rand() > opacity) {
		geometry::ray opacity_ray(
			result.position + ray.get_dir() * math::epsilon,
			ray.get_dir()
		);
		return trace(bounce, opacity_ray);
	}

	fvec3 normal = result.get_normal();
	fvec3 outcoming = -ray.get_dir();

	// With smooth shading the outcoming vector may point under the surface
	if (math::dot(normal, outcoming) <= 0)
		return fvec4::future;
		// outcoming = reflect(outcoming, normal); // Do not use this
		// We cannot correct the outcoming vector because that will result in rays getting trapped in a surface
		// A new ray might intersect at its starting point and get reflected back to it multiple times

	// Correct the roughness

	roughness = math::max(roughness, 0.05F); // Small roughness might cause precision artifacts

	// Are we going to sample specular or diffuse BRDF lobe?

	float specular_probability = pbr::fresnel(outcoming, reflect(-outcoming, normal), ior);
	specular_probability = math::max(specular_probability, metallic);
	bool specular_sample = core::rand() < specular_probability;

	// Direct Lighting

	fvec3 direct_out;

	if (sun_light) {
		fvec3 direct_incoming = sun_light->get_entity()->get_global_transform().basis * fvec3::backward;
		direct_incoming = util::rand_cone_vec(rand(), math::cos(rand() * sun_light->angular_radius), direct_incoming);

		// Sunlight lobe might intersect with the surface, so let's avoid that
		if (math::dot(normal, direct_incoming) > 0) {
			geometry::ray direct_ray(
				result.position + direct_incoming * math::epsilon,
				direct_incoming
			);
			auto direct_result = intersect(direct_ray);

			if (!direct_result.hit) {
				// If a shadow catcher is not in shadow, treat it as if it was fully transparent
				if (result.material->shadow_catcher && bounce == bounce_count) {
					geometry::ray opacity_ray(
						result.position + ray.get_dir() * math::epsilon,
						ray.get_dir()
					);
					return trace(bounce, opacity_ray);
				}

				// Diffuse BRDF

				float diffuse_pdf = pbr::pdf_diffuse(normal, direct_incoming);
				fvec3 diffuse_brdf = diffuse_pdf * albedo;

				// Specular BRDF

				float specular_pdf = pbr::pdf_specular(normal, outcoming, direct_incoming, roughness);
				fvec3 specular_brdf(specular_pdf);

				// Fresnel

				fvec3 fresnel = lerp(fvec3(0.04F), albedo, metallic);
				{
					fvec3 halfway = normalize(outcoming + direct_incoming);
					float cos_theta = dot(outcoming, halfway);

					fresnel = lerp(fresnel, fvec3::one, math::pow(1 - cos_theta, 5));
				}

				// Final BRDF

				diffuse_brdf = lerp(diffuse_brdf, fvec3::zero, metallic); // Metallic should realistically be either 1 or 0
				fvec3 brdf = lerp(diffuse_brdf, specular_brdf, fresnel);

				// Final PDF

				// !00% chance of hitting the sun
				diffuse_pdf = 1;
				specular_pdf = 1;
				float pdf = lerp(diffuse_pdf, specular_pdf, specular_probability);

				fvec3 direct_in = sun_light->energy;
				direct_out = brdf * direct_in / math::max(pdf, math::epsilon);
				direct_out = math::clamp(direct_out, fvec3::zero, direct_in);
			} else {
				// If a shadow catcher is in shadow, return zero
				if (result.material->shadow_catcher && bounce == bounce_count)
					return fvec4::future;
			}
		}
	}

	// Indirect Lighting

	fvec3 indirect_out;

	// Importance sampling

	fvec2 rand(core::rand(), core::rand());
	fvec3 indirect_incoming = specular_sample ?
			pbr::importance_specular(rand, normal, outcoming, roughness) :
			pbr::importance_diffuse(rand, normal, outcoming);

	// Specular BRDF lobe might intersect with the surface, so let's avoid that
	if (math::dot(normal, indirect_incoming) > 0) {
		// Diffuse BRDF

		float diffuse_pdf = pbr::pdf_diffuse(normal, indirect_incoming);
		fvec3 diffuse_brdf = diffuse_pdf * albedo;

		// Specular BRDF

		float specular_pdf = pbr::pdf_specular(normal, outcoming, indirect_incoming, roughness);
		fvec3 specular_brdf(specular_pdf);

		// Fresnel

		fvec3 fresnel = lerp(fvec3(0.04F), albedo, metallic);
		{
			fvec3 halfway = normalize(outcoming + indirect_incoming);
			float cos_theta = dot(outcoming, halfway);

			fresnel = lerp(fresnel, fvec3::one, math::pow(1 - cos_theta, 5));
		}

		// Final BRDF

		diffuse_brdf = lerp(diffuse_brdf, fvec3::zero, metallic); // Metallic should realistically be either 1 or 0
		fvec3 brdf = lerp(diffuse_brdf, specular_brdf, fresnel);

		// Final PDF

		float pdf = lerp(diffuse_pdf, specular_pdf, specular_probability);

		// Next bounce

		geometry::ray indirect_ray(
			result.position + indirect_incoming * math::epsilon,
			indirect_incoming
		);

		// This division by PDF partially cancels out with the BRDF
		fvec3 indirect_in = fvec3(trace(bounce - 1, indirect_ray));
		indirect_out = brdf * indirect_in / math::max(pdf, math::epsilon);

		// We refuse to return more than what was given and this prevents hot pixels
		indirect_out = math::clamp(indirect_out, fvec3::zero, indirect_in);

		// if (pdf == 0) {
		if (std::isnan(math::length(indirect_out))) {
		// if (math::any(indirect_out > fvec3(1000))) {
			float n_dot_o = dot(normal, outcoming);
			float n_dot_i = dot(normal, indirect_incoming);

			std::cout << "NaN value at outcoming: " << outcoming << " indirect_incoming: " << indirect_incoming << " brdf: " << brdf << std::endl;
			std::cout << "fresnel: " << fresnel << " diffuse_brdf: " << diffuse_brdf << " specular_brdf: " << specular_brdf << std::endl;
			std::cout << "n_dot_o: " << n_dot_o << " n_dot_i: " << n_dot_i << " bounce: " << static_cast<uint32_t>(bounce) << std::endl;
			std::cout << "position: " << result.position << " pdf: " << pdf << " indirect_in: " << indirect_in << " indirect_out: " << indirect_out << std::endl;
			std::cout << "diffuse_pdf: " << diffuse_pdf << " specular_pdf: " << specular_pdf << " roughness: " << roughness << std::endl;
			std::cout << std::endl;
		}
	}

	return fvec4(direct_out + indirect_out + emissive, 1);
}

renderer::intersect_result renderer::intersect(const ray &ray) const {
	std::stack<entity *> stack;
	stack.push(root.get());

	model::intersection nearest_hit;

	while (!stack.empty()) {
		entity *entity = stack.top();
		stack.pop();

		for (const auto &child : entity->get_children())
			stack.push(child.get());

		if (auto model = entity->get_component<scene::model>()) {
			auto hit = model->intersect(ray, visualize_kd_tree_depth);

			if (!hit.has_hit())
				continue;

			if (hit.distance < nearest_hit.distance
					|| !nearest_hit.has_hit()) {
				nearest_hit = hit;
			}
		}
	}

	if (!nearest_hit.has_hit())
		return { false };

	if (visualize_kd_tree_depth) {
		return {
			true, // Yes, we've hit
			nullptr, // No material
			nearest_hit.barycentric, // Random voxel color instead of position
			fvec2::zero, // No tex coord
			fvec3::zero, // No normal
			fvec3::zero // No tangent
		};
	}

	const auto &mesh     = nearest_hit.surface->mesh;
	const auto &material = nearest_hit.surface->material;

	uvec3 &indices = mesh->triangles[nearest_hit.triangle_index];
	auto &v1 = mesh->vertices[indices.x];
	auto &v2 = mesh->vertices[indices.y];
	auto &v3 = mesh->vertices[indices.z];

	// Normals will have to be normalized if transform applies scale
	transform transform = nearest_hit.transform;
	fmat3 normal_matrix = transpose(inverse(nearest_hit.transform.basis));

	fvec3 position = transform * (
			v1.position  * nearest_hit.barycentric.x +
			v2.position  * nearest_hit.barycentric.y +
			v3.position  * nearest_hit.barycentric.z);
	fvec2 tex_coord =
			v1.tex_coord * nearest_hit.barycentric.x +
			v2.tex_coord * nearest_hit.barycentric.y +
			v3.tex_coord * nearest_hit.barycentric.z;
	fvec3 normal = normalize(normal_matrix * (
			v1.normal    * nearest_hit.barycentric.x +
			v2.normal    * nearest_hit.barycentric.y +
			v3.normal    * nearest_hit.barycentric.z));
	fvec3 tangent = normalize(normal_matrix * (
			v1.tangent   * nearest_hit.barycentric.x +
			v2.tangent   * nearest_hit.barycentric.y +
			v3.tangent   * nearest_hit.barycentric.z));

	return {
		true,
		material,
		position,
		tex_coord,
		normal,
		tangent
	};
}

}
