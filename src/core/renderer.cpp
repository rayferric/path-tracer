#include "core/renderer.hpp"

#include "core/material.hpp"
#include "core/mesh.hpp"
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
#include "scene/transform.hpp"
#include "threading/thread_pool.hpp"

using namespace geometry;
using namespace math;
using namespace scene;

namespace core {

static fvec2 equirectangular_proj(const fvec3 &dir) {
	return fvec2(
		math::atan2(dir.z, dir.x) * 0.1591F + 0.5F,
		math::asin (dir.y)        * 0.3183F + 0.5F
	);
}

static fvec3 tonemap_approx_aces(const fvec3 &hdr) {
	constexpr float a = 2.51F; // TODO Make vectors constexpr-able
	const fvec3 b(0.03F);
	constexpr float c = 2.43F;
	const fvec3 d(0.59F);
	const fvec3 e(0.14F);
	return saturate((hdr * (a * hdr + b)) / (hdr * (c * hdr + d) + e));
}

static fvec3 reflect(const fvec3 &incident, const fvec3 &normal) {
	return incident - 2 * dot(normal, incident) * normal;
}

static std::unordered_map<std::string, std::weak_ptr<image::texture>> texture_cache;

static std::shared_ptr<image::texture> get_cached_texture(const std::filesystem::path &path, bool srgb) {
	if (path.empty())
		return nullptr;

	std::string path_str = path.string();

	if (texture_cache.contains(path_str)) {
		auto texture = texture_cache[path_str].lock();
		if (texture)
			return texture;
	}

	auto texture = image::image_texture::load(path, srgb);
	texture_cache[path_str] = texture;
	return texture;
}

renderer::renderer() :
		resolution(1920, 1080),
		thread_count(0),
		sample_count(1) {}

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

		aiString ai_normal_tex_path;
		aiString ai_albedo_opacity_tex_path;
		aiString ai_occlusion_tex_path;
		aiString ai_roughness_metallic_tex_path;
		aiString ai_emissive_tex_path;

		ai_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, ai_albedo_opacity);
		ai_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, ai_roughness);
		ai_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, ai_metallic);
		ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, ai_emissive);

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

			v.position  = *reinterpret_cast<fvec3 *>
					(ai_mesh->mVertices + i);
			v.tex_coord = *reinterpret_cast<fvec2 *>
					(ai_mesh->mTextureCoords[0] + i);
			v.normal    = *reinterpret_cast<fvec3 *>
					(ai_mesh->mNormals + i);
			v.tangent   = *reinterpret_cast<fvec3 *>
					(ai_mesh->mTangents + i);
		}

		for (size_t i = 0; i < mesh->triangles.size(); i++) {
			mesh->triangles[i] = *reinterpret_cast<uvec3 *>(
					ai_mesh->mFaces[i].mIndices);
		}

		mesh->recalculate_aabb();
		mesh->build_kd_tree();

		surfaces.emplace_back(mesh, materials[ai_mesh->mMaterialIndex]);
	}

	// We will instantiate just one camera

	if (ai_scene->mNumCameras == 0)
		throw std::runtime_error("Scene is missing a camera.");
	aiCamera *ai_camera = ai_scene->mCameras[0];

	// Instantiate nodes

	// We use raw pointer to save some performance
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
			// Assimp 5.0.1 doesn't support orthographic cameras
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

		// Instantiate children

		aiNode **ai_children = ai_node->mChildren;
		if (ai_children) {
			for(int i = 0; i < ai_node->mNumChildren; i++)
				stack.push({ entity.get(), ai_children[i] });
		}

		// Add to hierarchy

		if (parent)
			entity->set_parent(parent->shared_from_this());
		else
			root = std::move(entity);
	}

	if (!camera)
		throw std::runtime_error("Scene is missing a camera.");
}

void renderer::render(const std::filesystem::path &path) const {
	auto img = std::make_shared<image::image>(resolution, 3, false, true);

	threading::thread_pool pool(thread_count);

	for (uint32_t sample = 0; sample < sample_count; sample++) {
		std::cout << "Rendering sample #" << (sample + 1) << std::endl;

		for (uint32_t y = 0; y < resolution.y; y++) {
			pool.schedule([this, &img, y, sample] {
				for (uint32_t x = 0; x < resolution.x; x++) {
					uvec2 pixel(x, y);

					fvec2 ndc = (fvec2(pixel) / resolution) * 2 - fvec2::one;
					ndc.y = -ndc.y;
					float ratio = static_cast<float>(resolution.x) / resolution.y;

					ray ray = camera->get_ray(ndc, ratio);
					fvec3 color = integrate(ray);

					color = tonemap_approx_aces(color);

					float r = img->read(pixel, 0);
					float g = img->read(pixel, 1);
					float b = img->read(pixel, 2);

					color += fvec3(r, g, b) * sample;
					color *= 1.0F / (sample + 1);

					img->write(pixel, 0, color.x);
					img->write(pixel, 1, color.y);
					img->write(pixel, 2, color.z);
				}
			});
		}

		pool.wait();
	}

	std::cout << "Saving..." << std::endl;

	img->save(path);
}

fvec3 renderer::integrate(const ray &ray) const {
	auto camera_result = trace(ray);

	if (!camera_result.hit)
		return fvec3(environment->sample(equirectangular_proj(ray.get_dir())));

	fvec3 sample_dir = reflect(ray.get_dir(), camera_result.normal);
	fvec3 color = fvec3(environment->sample(equirectangular_proj(sample_dir)));
	color *= camera_result.material->get_albedo(camera_result.tex_coord);

	return color;
}

renderer::trace_result renderer::trace(const ray &ray) const {
	std::stack<entity *> stack;
	stack.push(root.get());

	model::intersection nearest_hit;

	while (!stack.empty()) {
		entity *entity = stack.top();
		stack.pop();

		for (const auto &child : entity->get_children())
			stack.push(child.get());

		if (auto model = entity->get_component<scene::model>()) {
			auto hit = model->intersect(ray);

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
		position,
		tex_coord,
		normal,
		tangent,
		material
	};
}

}
