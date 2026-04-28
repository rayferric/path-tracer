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
#include "util/thread_pool.hpp"

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

static std::shared_ptr<image::texture> get_cached_texture(const std::filesystem::path &path, bool srgb) {
	static std::unordered_map<std::string, std::weak_ptr<image::texture>> texture_cache;

	if (path.empty())
		return nullptr;

	std::string path_str = path.string();
	path_str = std::regex_replace(path_str, std::regex("%20"), " "); // GLTF encodes spaces as %20

	if (texture_cache.contains(path_str)) {
		auto texture = texture_cache[path_str].lock();
		if (texture)
			return texture;
	}

	auto texture = image::image_texture::load(path_str, srgb);
	texture_cache[path_str] = texture;
	return texture;
}

void renderer::load_gltf(const std::filesystem::path &path) {
	// Import file

	Assimp::Importer importer;
	const aiScene *ai_scene =
			importer.ReadFile(path.string(),
			aiProcess_Triangulate |
			aiProcess_CalcTangentSpace |
			aiProcess_JoinIdenticalVertices);

	if(!ai_scene || (ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) > 0
			|| !ai_scene->mRootNode)
		throw std::runtime_error(importer.GetErrorString());

	// Create materials

	std::vector<std::shared_ptr<material>> materials;
	materials.reserve(ai_scene->mNumMaterials);

	for(int i = 0; i < ai_scene->mNumMaterials; i++) {
		aiMaterial *ai_material = ai_scene->mMaterials[i];
		
		auto material = std::make_shared<core::material>();

		aiColor4D ai_albedo_opacity;
		ai_real ai_roughness, ai_metallic;
		aiColor3D ai_emissive;
		aiString ai_alpha_mode;

		aiString ai_normal_tex_path;
		aiString ai_albedo_opacity_tex_path;
		aiString ai_occlusion_tex_path;
		aiString ai_roughness_metallic_tex_path;
		aiString ai_emissive_tex_path;

		ai_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, ai_albedo_opacity);
		ai_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, ai_roughness);
		ai_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, ai_metallic);
		ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, ai_emissive);
		ai_material->Get(AI_MATKEY_GLTF_ALPHAMODE, ai_alpha_mode);

		ai_material->GetTexture(aiTextureType_NORMALS, 0, &ai_normal_tex_path);
		ai_material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &ai_albedo_opacity_tex_path);
		ai_material->GetTexture(aiTextureType_LIGHTMAP, 0, &ai_occlusion_tex_path);
		ai_material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &ai_roughness_metallic_tex_path);
		ai_material->GetTexture(aiTextureType_EMISSIVE, 0, &ai_emissive_tex_path);

		material->albedo_fac    = fvec3(ai_albedo_opacity.r, ai_albedo_opacity.g, ai_albedo_opacity.b);
		material->opacity_fac   = ai_albedo_opacity.a;
		material->roughness_fac = ai_roughness;
		material->metallic_fac  = ai_metallic;
		material->emissive_fac  = fvec3(ai_emissive.r, ai_emissive.g, ai_emissive.b);

		auto gltf_dir = path.parent_path();

		if (ai_normal_tex_path.length > 0) {
			auto normal_tex = get_cached_texture(gltf_dir / ai_normal_tex_path.C_Str(), false);
			material->normal_tex = normal_tex;
		}

		if (ai_albedo_opacity_tex_path.length > 0) {
			auto albedo_opacity_tex = get_cached_texture(gltf_dir / ai_albedo_opacity_tex_path.C_Str(), true);
			material->albedo_tex = albedo_opacity_tex;
			if (ai_alpha_mode.C_Str() != std::string("OPAQUE"))
				material->opacity_tex = albedo_opacity_tex;
		}

		if (ai_occlusion_tex_path.length > 0) {
			auto occlusion_tex = get_cached_texture(gltf_dir / ai_occlusion_tex_path.C_Str(), false);
			material->occlusion_tex = occlusion_tex;
		}

		if (ai_roughness_metallic_tex_path.length > 0) {
			auto roughness_metallic_tex = get_cached_texture(gltf_dir / ai_roughness_metallic_tex_path.C_Str(), false);
			material->roughness_tex = roughness_metallic_tex;
			material->metallic_tex = roughness_metallic_tex;
		}

		if (ai_emissive_tex_path.length > 0) {
			auto emissive_tex = get_cached_texture(gltf_dir / ai_emissive_tex_path.C_Str(), true);
			material->emissive_tex = emissive_tex;
		}

		std::string name{ai_material->GetName().C_Str()};
		if (name.find("shadow") != std::string::npos && name.find("catcher") != std::string::npos)
			material->shadow_catcher = true;

		materials.push_back(material);
	}

	// Create meshes

	std::vector<scene::model::surface> surfaces;
	surfaces.reserve(ai_scene->mNumMeshes);

	for(uint32_t i = 0; i < ai_scene->mNumMeshes; i++) {
		aiMesh *ai_mesh = ai_scene->mMeshes[i];

		std::shared_ptr<mesh> mesh = std::make_shared<core::mesh>();
		mesh->vertices.resize(ai_mesh->mNumVertices);
		mesh->triangles.resize(ai_mesh->mNumFaces);

		for (size_t i = 0; i < mesh->vertices.size(); i++) {
			vertex &v = mesh->vertices[i];

			aiVector3D &position  = ai_mesh->mVertices[i];
			aiVector3D &tex_coord = ai_mesh->mTextureCoords[0][i];
			aiVector3D &normal    = ai_mesh->mNormals[i];
			aiVector3D &tangent   = ai_mesh->mTangents[i];

			v.position.x = position.x;
			v.position.y = position.y;
			v.position.z = position.z;

			v.tex_coord.x = tex_coord.x;
			v.tex_coord.y = tex_coord.y;

			v.normal.x = normal.x;
			v.normal.y = normal.y;
			v.normal.z = normal.z;
			
			v.tangent.x = tangent.x;
			v.tangent.y = tangent.y;
			v.tangent.z = tangent.z;
		}

		for (size_t i = 0; i < mesh->triangles.size(); i++) {
			unsigned int *indices = ai_mesh->mFaces[i].mIndices;
			
			mesh->triangles[i].x = indices[0];
			mesh->triangles[i].y = indices[1];
			mesh->triangles[i].z = indices[2];
		}

		mesh->recalculate_aabb();
		mesh->build_kd_tree();

		surfaces.push_back({ mesh, materials[ai_mesh->mMaterialIndex] });
	}

	// We will instantiate just one camera

	if (ai_scene->mNumCameras < camera_index + 1)
		throw std::runtime_error("Scene does not contain camera #" + util::to_string(camera_index) + ".");
	aiCamera *ai_camera = ai_scene->mCameras[camera_index];

	aiLight *ai_sun_light = nullptr;
	if (sun_light_index != renderer::no_sun_light) {
		if (ai_scene->mNumLights < sun_light_index + 1) {
			std::cerr << "Scene does not contain sun light #" << sun_light_index << ". No sun light will be used." << std::endl;
			goto skip_sun_light;
		}
		if (ai_scene->mLights[sun_light_index]->mType != aiLightSourceType::aiLightSource_DIRECTIONAL) {
			std::cerr << "Light #" << sun_light_index << " is not a sun light. No sun light will be used." << std::endl;
			goto skip_sun_light;
		}
		ai_sun_light = ai_scene->mLights[sun_light_index];
	}
	skip_sun_light:

	// Instantiate nodes
	std::stack<std::tuple<entity *, aiNode *>> stack;
	stack.push({ nullptr, ai_scene->mRootNode });

	while (!stack.empty()) {
		auto [parent, ai_node] = stack.top();
		stack.pop();

		// Set Properties

		auto entity = std::make_shared<scene::entity>();

		entity->set_name(ai_node->mName.C_Str());

		aiMatrix4x4 m = ai_node->mTransformation;
		entity->set_local_transform(transform(
			fvec3(m.a4, m.b4, m.c4),
			fmat3(
				m.a1, m.b1, m.c1,
				m.a2, m.b2, m.c2,
				m.a3, m.b3, m.c3
			)
		));

		// Add components

		if (ai_node->mMeshes) {
			auto model = entity->add_component<scene::model>();

			for (uint32_t i = 0; i < ai_node->mNumMeshes; i++)
				model->surfaces.push_back(surfaces[ai_node->mMeshes[i]]);

			model->recalculate_aabb();
		}

		if (entity->get_name() == ai_camera->mName.C_Str()) {
			// Assimp 5.0.1 does not support orthographic cameras
			auto camera = entity->add_component<scene::camera>();

			// Choose as the active one
			this->camera = camera;

			// Assimp does some illogical
			// maths behind the curtain...
			// Normally we would scale
			// half-angle tangents instead
			float vfov = ai_camera->mHorizontalFOV
					/ ai_camera->mAspect;
			camera->set_fov(vfov);
		}

		if (ai_sun_light && entity->get_name() == ai_sun_light->mName.C_Str()) {
			// Assimp 5.0.1 doesn't support orthographic cameras
			auto sun_light = entity->add_component<scene::sun_light>();

			// Choose as the active one
			this->sun_light = sun_light;

			sun_light->energy = math::fvec3(ai_sun_light->mColorDiffuse.r, ai_sun_light->mColorDiffuse.g, ai_sun_light->mColorDiffuse.b);
			// Assimp 5.2.2 does not support directional light radii
		}

		// Queue children up for instantiation

		aiNode **ai_children = ai_node->mChildren;
		if (ai_children) {
			for(int i = 0; i < ai_node->mNumChildren; i++)
				stack.push({ entity.get(), ai_children[i] });
		}

		// Add the current node to hierarchy

		if (parent)
			entity->set_parent(parent->shared_from_this());
		else
			root = std::move(entity);
	}

	if (!camera)
		throw std::runtime_error("Scene is missing a camera.");
}

void renderer::render(const std::filesystem::path &path) const {
	util::thread_pool pool(thread_count);

	std::vector<std::shared_ptr<util::future>> todo;
	todo.reserve(resolution.y);

	// Claiming pixels by opaque samples is needed for proper transparent background blending
	struct pixel {
		math::fvec3 color;
		float alpha;
		bool claimed;
	};

	std::vector<std::vector<pixel>> pixels;
	pixels.resize(resolution.x);
	for (auto &column : pixels)
		column.resize(resolution.y, { fvec3::zero, 0, false });

	auto img = std::make_shared<image::image>(resolution, 4, false, true);

	for (uint32_t sample = 0; sample < sample_count; sample++) {
		std::cout << "Drawing sample " << sample + 1 << " out of " << sample_count << '.' << std::endl;

		for (uint32_t y = 0; y < resolution.y; y++) {
			todo.push_back(pool.submit([&, y] (uint32_t) {
				for (uint32_t x = 0; x < resolution.x; x++) {
					uvec2 pixel(x, y);

					// Do not offset the first sample so we can get a consistent alpha mask for smart blending
					fvec2 aa_offset = fvec2(rand(), rand());

					fvec2 ndc = ((fvec2(pixel) + aa_offset) /
							resolution) * 2 - fvec2::one;
					ndc.y = -ndc.y;
					float ratio = static_cast<float>(resolution.x) / resolution.y;

					ray ray = camera->get_ray(ndc, ratio);
					fvec4 data = trace(bounce_count, ray);

					// Smart blending - needed for transparent background
					if (transparent_background) {
						if (data.w > 0.5 && !pixels[x][y].claimed) {
							// If an opaque sample will claim this pixel
							pixels[x][y].color = fvec3(data); // Overwrite the color
							pixels[x][y].alpha = 1 / (sample + 1); // And blend the alpha
							pixels[x][y].claimed = true; // Mark the pixel as claimed
							continue;
						} else if (data.w < 0.5 && pixels[x][y].claimed) {
							// If a transparent sample encounters a claimed pixel
							pixels[x][y].alpha = pixels[x][y].alpha * sample + data.w; // Blend only alpha
							pixels[x][y].alpha /= sample + 1;
							continue;
						} else if (data.w < 0.5) {
							// If transparent sample blends with an unclaimed pixel
							// Do nothing and preserve the default transparent black color
							continue;
						}
					}

					// Otherwise if an opaque sample blends with a claimed pixel (or transparent background is disabled)
					pixels[x][y].color = pixels[x][y].color * sample + fvec3(data); // Blend color
					pixels[x][y].color /= sample + 1;
					pixels[x][y].alpha = pixels[x][y].alpha * sample + data.w; // Blend alpha
					pixels[x][y].alpha /= sample + 1;
				}
			}));
		}

		for (auto &future : todo)
			future->wait();

		todo.clear();

		if (sample != 0 && sample % 5 == 0 || sample == sample_count - 1) {
			std::cout << "Saving..." << std::endl;

			for (uint32_t y = 0; y < resolution.y; y++) {
				for (uint32_t x = 0; x < resolution.x; x++) {
					fvec3 color = tonemap_approx_aces(pixels[x][y].color);
					float alpha = pixels[x][y].alpha;

					uvec2 pixel(x, y);
					img->write(pixel, 0, color.x);
					img->write(pixel, 1, color.y);
					img->write(pixel, 2, color.z);
					img->write(pixel, 3, alpha);
				}
			}

			img->save(path);
		}
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
