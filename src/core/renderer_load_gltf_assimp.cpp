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
#include <memory>

using namespace geometry;
using namespace math;
using namespace scene;

namespace core {

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

}
