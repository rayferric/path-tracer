#include "core/renderer.hpp"

#include "core/material.hpp"
#include "core/mesh.hpp"
#include "geometry/ray.hpp"
#include "image/image.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/mat3.hpp"
#include "math/math.hpp"
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

	// Create materials

	// std::vector<std::shared_ptr<material>> materials;
	// materials.reserve(ai_scene->mNumMaterials);

	// for(int i = 0; i < ai_scene->mNumMaterials; i++) {
	// 	aiMaterial *ai_material = ai_scene->mMaterials[i];
		
	// 	auto material = std::make_shared<core::material>();

	// 	int tmp;

	// 	ai_material->Get(AI_MATKEY_TWOSIDED, tmp);
	// 	if(tmp != 0) material->culling = false;
	// 	ai_material->Get(AI_MATKEY_GLTF_UNLIT, tmp);
	// 	if(tmp != 0) material.unlit = true;

    //     try(MemoryStack stack = MemoryStack.stackPush()) {
    //         FloatBuffer buf = stack.mallocFloat(1);
    //         IntBuffer max = stack.mallocInt(1).put(1).flip();
    //         aiGetMaterialFloatArray(aiMaterial, aiAI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, aiTextureType_NONE, 0, buf, max);
    //         material.setMetallic(buf.get(0));
    //         aiGetMaterialFloatArray(aiMaterial, aiAI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, aiTextureType_NONE, 0, buf, max);
    //         material.setRoughness(buf.get(0));
    //     }

    //     AIColor4D aiColor = AIColor4D.create();
    //     aiGetMaterialColor(aiMaterial, aiAI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, aiTextureType_NONE, 0, aiColor);
    //     material.setColor(new Vector4f(aiColor.r(), aiColor.g(), aiColor.b(), aiColor.a()));

    //     if(aiColor.a() < 1) {
    //         material.setTranslucent(true);
    //         material.setCulling(false);
    //     }

    //     aiColor.clear();
    //     aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_EMISSIVE, aiTextureType_NONE, 0, aiColor);
    //     material.setEmissive(new Vector3f(aiColor.r(), aiColor.g(), aiColor.b()));

    //     AIString aiString = AIString.create();

    //     aiGetMaterialString(aiMaterial, aiAI_MATKEY_GLTF_ALPHAMODE, aiTextureType_NONE, 0, aiString);
    //     if(aiString.dataString().equals("BLEND")) {
    //         material.setTranslucent(true);
    //         material.setCulling(false);
    //     }

    //     aiString.clear();
    //     Assimp.aiGetMaterialTexture(aiMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, 0, aiString, (IntBuffer)null, null, null, null, null, null);
    //     String fileName = aiString.dataString();
    //     if(!fileName.isEmpty())
    //         material.setColorMap(getCachedTexture(sceneDir + fileName));

    //     aiString.clear();
    //     Assimp.aiGetMaterialTexture(aiMaterial, aiTextureType_NORMALS, 0, aiString, (IntBuffer)null, null, null, null, null, null);
    //     fileName = aiString.dataString();
    //     if(!fileName.isEmpty())
    //         material.setNormalMap(getCachedTexture(sceneDir + fileName));

    //     aiString.clear();
    //     Assimp.aiGetMaterialTexture(aiMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, 0, aiString, (IntBuffer)null, null, null, null, null, null);
    //     fileName = aiString.dataString();
    //     if(!fileName.isEmpty())
    //         material.setMetallicRoughnessMap(getCachedTexture(sceneDir + fileName));

    //     aiString.clear();
    //     Assimp.aiGetMaterialTexture(aiMaterial, aiTextureType_LIGHTMAP, 0, aiString, (IntBuffer)null, null, null, null, null, null);
    //     fileName = aiString.dataString();
    //     if(!fileName.isEmpty())
    //         material.setOcclusionMap(getCachedTexture(sceneDir + fileName));

    //     aiString.clear();
    //     Assimp.aiGetMaterialTexture(aiMaterial, aiTextureType_EMISSIVE, 0, aiString, (IntBuffer)null, null, null, null, null, null);
    //     fileName = aiString.dataString();
    //     if(!fileName.isEmpty())
    //         material.setEmissiveMap(getCachedTexture(sceneDir + fileName));

    //     return material;
	// }

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
	
	// for (uint32_t x = 0; x < resolution.x; x++) {
	// 	for (uint32_t y = 0; y < resolution.y; y++) {
	// 		if (y == 0)
	// 			std::cout << "Drawing column #" << x << std::endl;

	// 		fvec2 ndc = (fvec2(x, y) / resolution) * 2 - fvec2::one;
	// 		ndc.y = -ndc.y;
	// 		float ratio = static_cast<float>(resolution.x) / resolution.y;

	// 		ray ray = camera->get_ray(ndc, ratio);
	// 		fvec3 color = integrate(ray);

	// 		img->write(uvec2(x, y), 0, color.x);
	// 		img->write(uvec2(x, y), 1, color.y);
	// 		img->write(uvec2(x, y), 2, color.z);
	// 	}
	// }

	std::cout << "Integrating..." << std::endl;

	img->parallel_for_each([&img, this](uvec2 pixel) {
		fvec2 ndc = (fvec2(pixel) / resolution) * 2 - fvec2::one;
		ndc.y = -ndc.y;
		float ratio = static_cast<float>(resolution.x) / resolution.y;

		ray ray = camera->get_ray(ndc, ratio);
		fvec3 color = integrate(ray);

		img->write(pixel, 0, color.x);
		img->write(pixel, 1, color.y);
		img->write(pixel, 2, color.z);
	});

	img->save(path);
}

fvec3 renderer::integrate(const ray &ray) const {
	auto camera_result = trace(ray);

	if (!camera_result.hit)
		return fvec3::zero;
	else {
		geometry::ray shadow_ray(
			camera_result.position + fvec3(1 / math::sqrt3) * math::epsilon,
			fvec3(1 / math::sqrt3)
		);

		auto shadow_result = trace(shadow_ray);

		if (shadow_result.hit)
			return fvec3(0.1F);
		else
			return max(dot(camera_result.normal, fvec3(1 / math::sqrt3)), 0.1F);
	}
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

}
