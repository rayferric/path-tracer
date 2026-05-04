#pragma once

#include "../pch.hpp"

#include "../scene.hpp"

#include "./intersect_scene.hpp"
#include "./material_tex_sampling.hpp"
#include "./pbr.hpp"
#include "./rand.hpp"

CUDA_CALLABLE inline float avg_components(glm::vec3 v) {
	return 0.3333f * (v.x + v.y + v.z);
}

// no recursion version
// alpha is either 1 or zero
CUDA_CALLABLE inline glm::fvec3 path_trace(const scene_data &scn, glm::fvec3 pos, glm::fvec3 dir, rng_state &rng, uint32_t max_bounces, bool &camera_ray_hit_bg) {
	camera_ray_hit_bg = false;
	glm::fvec3 throughput(1.0f);
	glm::fvec3 accumulated_light(0.0f);
	bool camera_ray = true;

	for (uint32_t bounce = 0; bounce < max_bounces; bounce++) {
		hit_info hit = intersect_scene(scn, pos, dir);

		if (!hit.hit) {
			accumulated_light += throughput * sample_environment(scn, dir);
			if (camera_ray) {
				if (rand_float(rng) < avg_components(throughput)) {
					camera_ray_hit_bg = true;
				}
			}
			break;
		}

		const material &mat = scn.materials[hit.mat_idx];
		mat_sample mat_values = sample_material(scn, hit.mat_idx, hit.uv, hit.normal, hit.tangent);
		auto [albedo, opacity, roughness, metallic, emissive, normal, ior] = mat_values;

		// opacity testing - continue ray with probability 1-alpha
		// shadow catchers cannot be half-transparent, so exclude them
		if (opacity < 1.0f && rand_float(rng) > opacity && !mat.shadow_catcher) {
			pos = hit.position + dir * 0.001f;
			continue; // ^ skip to next iteration with same direction
		}

		// add emissive
		accumulated_light += throughput * emissive;

		glm::fvec3 outcoming = -dir;

		// fix normals below surface
		if (glm::dot(normal, outcoming) < 0.0f) {
			normal = -normal;
		}

		// fix small roughness causing numerical instability
		// i.e. mirror surface becomes black
		roughness = glm::max(roughness, 0.01f);

		// direct lighting
		if (glm::length(scn.sunlight_intensity) > 0.0f) {
			glm::fvec3 direct_incoming = -scn.sunlight_dir;
			direct_incoming = rand_cone_dir_uniform(rng, direct_incoming, scn.sunlight_angular_radius);

			if (glm::dot(normal, direct_incoming) > 0.0f) {
				glm::fvec3 shadow_origin = hit.position + direct_incoming * 0.001f;
				hit_info shadow_hit = intersect_scene(scn, shadow_origin, direct_incoming);

				if (!shadow_hit.hit) {
					glm::fvec3 diffuse_brdf = brdf_diffuse(normal, direct_incoming, albedo, metallic);
					glm::fvec3 specular_brdf = brdf_specular(normal, outcoming, direct_incoming, roughness);

					glm::fvec3 f0 = glm::mix(glm::fvec3(0.04f), albedo, metallic);
					glm::fvec3 fresnel_term = fresnel_f0(outcoming, direct_incoming, f0);

					glm::fvec3 brdf = glm::mix(diffuse_brdf, specular_brdf, fresnel_term);

					// pdf=1, because ray ALWAYS hits the sun
					glm::vec3 weight = brdf; /* / pdf; */
					accumulated_light += throughput * weight * scn.sunlight_intensity;
				}

				if (shadow_hit.hit && mat.shadow_catcher && bounce == 0) {
					// shadow catcher in shadow returns full black
					if (rand_float(rng) > 0.75f) { // half transparency
						camera_ray_hit_bg = true;
					}
					return glm::fvec3(0.0f);
				}
			}
		}

		// shadow catcher outside of shadow behaves like bg
		if (mat.shadow_catcher && camera_ray) {
			if (rand_float(rng) < avg_components(throughput)) {
				camera_ray_hit_bg = true;
			}
			break;
		}

		// indirect lighting
		glm::fvec3 reflected = glm::reflect(-outcoming, normal);
		float specular_probability = fresnel_ior(outcoming, reflected, ior);
		specular_probability = glm::max(specular_probability, metallic);
		bool specular_sample = rand_float(rng) < specular_probability;
		glm::fvec3 indirect_incoming = specular_sample ? importance_specular(rng, normal, outcoming, roughness) : importance_diffuse(rng, normal, outcoming);

		if (glm::dot(normal, indirect_incoming) <= 0.0f) {
			break; // ^ no valid indirect direction
		}

		glm::fvec3 diffuse_brdf = brdf_diffuse(normal, indirect_incoming, albedo, metallic);
		glm::fvec3 specular_brdf = brdf_specular(normal, outcoming, indirect_incoming, roughness);

		glm::fvec3 f0 = glm::mix(glm::fvec3(0.04f), albedo, metallic);
		glm::fvec3 fresnel_term = fresnel_f0(outcoming, indirect_incoming, f0);

		glm::fvec3 brdf = glm::mix(diffuse_brdf, specular_brdf, fresnel_term);

		float diffuse_pdf = pdf_diffuse(normal, indirect_incoming);
		float specular_pdf = pdf_specular(normal, outcoming, indirect_incoming, roughness);
		float pdf = glm::mix(diffuse_pdf, specular_pdf, specular_probability);

		glm::fvec3 weight = brdf / glm::max(pdf, 0.0001f);
		weight = glm::clamp(weight, glm::fvec3(0.0f), glm::fvec3(1.0f));
		throughput *= weight;

		// setup next iteration
		pos = hit.position + indirect_incoming * 0.001f;
		dir = indirect_incoming;
		camera_ray = false;

		// russian roulette early termination
		if (bounce > 3) {
			float continue_prob = glm::min(glm::max(throughput.x, glm::max(throughput.y, throughput.z)), 0.95f);
			if (rand_float(rng) > continue_prob) {
				break;
			}
			// compensate - increase throughput for the non-terminated rays
			throughput /= continue_prob;
		}
	}

	return accumulated_light;
}
