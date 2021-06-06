#include "core/renderer.hpp"

#include "core/material.hpp"
#include "core/mesh.hpp"
#include "geometry/ray.hpp"
#include "image/image.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/mat3.hpp"
#include "scene/camera.hpp"
#include "scene/entity.hpp"
#include "scene/model.hpp"
#include "scene/transform.hpp"

using namespace geometry;
using namespace math;
using namespace scene;

namespace core {

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

	// Create surfaces

	std::vector<model::surface> surfaces;
	surfaces.reserve(ai_scene->mNumMeshes);

	auto material = std::make_shared<core::material>();

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

		surfaces.push_back({ mesh, material });
	}

	// We will instantiate just one camera

	if (ai_scene->mNumCameras == 0)
		throw std::runtime_error("Scene is missing a camera.");
	std::unordered_map<std::string, aiCamera *> cameras;

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

		uint32_t *ai_mesh_indices = ai_node->mMeshes;
		if (ai_mesh_indices) {
			auto model = entity->add_component<scene::model>();

			for (uint32_t i = 0; i < ai_node->mNumMeshes; i++)
				model->surfaces.push_back(surfaces[ai_mesh_indices[i]]);
		}

		if (entity->get_name() == ai_camera->mName.C_Str()) {
			// Assimp 5.0.1 doesn't support orthographic cameras
			auto camera = entity->add_component<scene::camera>();

			// Choose as the active one
			this->camera = camera;

			//Map horizontal FOV to vertical
			camera->set_fov(ai_camera->mHorizontalFOV /
					ai_camera->mAspect);
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

}

void renderer::render(const std::filesystem::path &path) const {
	auto img = std::make_shared<image::image>(resolution, 3, false, true);
	
	for (uint32_t x = 0; x < resolution.x; x++) {
		for (uint32_t y = 0; y < resolution.y; y++) {
			fvec2 screen_pos = (fvec2(x, y) / resolution) * 2 - fvec2(1);
			screen_pos.y = -screen_pos.y;
			float ratio = static_cast<float>(resolution.x) / resolution.y;

			ray ray = camera->get_ray(screen_pos, ratio);

			if (y == 0)
				std::cout << "Drawing row: " << x << std::endl;

			fvec3 color = integrate(root, ray);

			img->write(uvec2(x, y), 0, color.x);
			img->write(uvec2(x, y), 1, color.y);
			img->write(uvec2(x, y), 2, color.z);
		}
	}

	img->save(path);
}

renderer::trace_result renderer::trace(const ray &ray) const {
	std::stack<entity *> stack;
	stack.push(root.get());

	model::intersection nearest_hit;

	while (!stack.empty()) {
		entity *entity = stack.top();
		stack.pop();

		if (auto model = entity->get_component<scene::model>()) {
			auto hit = model->intersect(ray);

			if (hit.distance < nearest_hit.distance
					|| !nearest_hit.has_hit()) {
				nearest_hit = hit;
			}
		}

		for (const auto &child : entity->get_children())
			stack.push(child.get());
	}

	if (!nearest_hit.has_hit())
		return { false };

	const auto &mesh     = nearest_hit.surface->mesh;
	const auto &material = nearest_hit.surface->material;

	uvec3 &indices = mesh->triangles[nearest_hit.triangle_index];
	auto &v1 = mesh->vertices[indices.x];
	auto &v2 = mesh->vertices[indices.y];
	auto &v3 = mesh->vertices[indices.z];

	transform &transform = nearest_hit.transform;
	fmat3 normal_matrix = transpose(inverse(nearest_hit.transform.basis));

	fvec3 position = transform * (
			v1.position  * nearest_hit.barycentric.x +
			v2.position  * nearest_hit.barycentric.y +
			v3.position  * nearest_hit.barycentric.z);
	fvec2 tex_coord =
			v1.tex_coord * nearest_hit.barycentric.x +
			v2.tex_coord * nearest_hit.barycentric.y +
			v3.tex_coord * nearest_hit.barycentric.z;
	fvec3 normal = normal_matrix * (
			v1.normal    * nearest_hit.barycentric.x +
			v2.normal    * nearest_hit.barycentric.y +
			v3.normal    * nearest_hit.barycentric.z);
	fvec3 tangent = normal_matrix * (
			v1.tangent   * nearest_hit.barycentric.x +
			v2.tangent   * nearest_hit.barycentric.y +
			v3.tangent   * nearest_hit.barycentric.z);

	return {
		true,
		position,
		tex_coord,
		normal,
		tangent,
		material
	};
}

fvec3 renderer::integrate(const ray &ray) const {
	auto result = trace(ray);

	if (!result.hit)
		return fvec3::zero;
	else
		return 
}

}
