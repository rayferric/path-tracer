#include "core/renderer.hpp"

#include "core/mesh.hpp"
#include "scene/model.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/mat3.hpp"
#include "scene/entity.hpp"
#include "scene/transform.hpp"

using namespace math;
using namespace scene;

namespace core {

static std::shared_ptr<mesh> process_ai_mesh(aiMesh *ai_mesh) {
	std::shared_ptr<mesh> mesh = std::make_shared<core::mesh>();

	mesh->vertices.resize(ai_mesh->mNumVertices);

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

	mesh->triangles.resize(ai_mesh->mNumFaces);

	for (size_t i = 0; i < mesh->triangles.size(); i++) {
		mesh->triangles[i] = *reinterpret_cast<uvec3 *>(
				ai_mesh->mFaces[i].mIndices);
	}

	mesh->recalculate_aabb();
	mesh->build_kd_tree();
	return mesh;
}

static std::shared_ptr<entity> process_ai_node(aiNode *ai_node,
		const std::vector<model::surface> &surfaces) {
	std::shared_ptr<entity> entity
			= std::make_shared<scene::entity>();

	// Instantiate children

	aiNode **ai_children = ai_node->mChildren;
	if (ai_children) {
		for(int i = 0; i < ai_node->mNumChildren; i++)
			process_ai_node(ai_children[i], surfaces)->set_parent(entity);
	}

	// Set properties

	entity->set_name(ai_node->mName.C_Str());

	aiMatrix4x4 m = ai_node->mTransformation;
	transform &transform = entity->get_local_transform();

	transform.origin = fvec3(m.a4, m.b4, m.c4);
	transform.basis = fmat3(
		m.a1, m.b1, m.c1,
		m.a2, m.b2, m.c2,
		m.a3, m.b3, m.c3
	);

	// Add components

	uint32_t *ai_mesh_indices = ai_node->mMeshes;
	if (ai_mesh_indices) {
		auto model = entity->add_component<scene::model>();

		for (uint32_t i = 0; i < ai_node->mNumMeshes; i++)
			model->surfaces.push_back(surfaces[ai_mesh_indices[i]]);
	}

	return entity;
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

	// Create surfaces

	std::vector<model::surface> surfaces;
	surfaces.reserve(ai_scene->mNumMeshes);

	std::shared_ptr<material> material
			= std::make_shared<core::material>();

	for(uint32_t i = 0; i < ai_scene->mNumMeshes; i++) {
		aiMesh *ai_mesh = ai_scene->mMeshes[i];

		std::shared_ptr<mesh> mesh = process_ai_mesh(ai_mesh);

		surfaces.push_back({ mesh, material });
	}

	// Instantiate nodes

	root = process_ai_node(ai_scene->mRootNode, surfaces);

	// Load cameras

	for(uint32_t i = 0; i < ai_scene->mNumCameras; i++) {
		aiCamera *ai_camera = ai_scene->mCameras[i];

		//Map horizontal to vertical
		float fov = ai_camera->mHorizontalFOV / ai_camera->mAspect;
		
	}
}

}
