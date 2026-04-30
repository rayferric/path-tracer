// #include "core/renderer.hpp"

// #include "core/material.hpp"
// #include "core/mesh.hpp"
// #include "core/pbr.hpp"
// #include "geometry/ray.hpp"
// #include "image/image.hpp"
// #include "image/image_texture.hpp"
// #include "math/vec2.hpp"
// #include "math/vec3.hpp"
// #include "math/mat3.hpp"
// #include "math/math.hpp"
// #include "scene/camera.hpp"
// #include "scene/entity.hpp"
// #include "scene/model.hpp"
// #include "scene/sun_light.hpp"
// #include "scene/transform.hpp"
// #include "util/rand_cone_vec.hpp"
// #include "util/thread_pool.hpp"
// #include <memory>

// using namespace geometry;
// using namespace math;
// using namespace scene;

// namespace core {

// static std::shared_ptr<image::texture> get_cached_texture(const std::filesystem::path &path, bool srgb) {
// 	static std::unordered_map<std::string, std::weak_ptr<image::texture>> texture_cache;

// 	if (path.empty())
// 		return nullptr;

// 	std::string path_str = path.string();
// 	path_str = std::regex_replace(path_str, std::regex("%20"), " "); // GLTF encodes spaces as %20

// 	if (texture_cache.contains(path_str)) {
// 		auto texture = texture_cache[path_str].lock();
// 		if (texture)
// 			return texture;
// 	}

// 	auto texture = image::image_texture::load(path_str, srgb);
// 	texture_cache[path_str] = texture;
// 	return texture;
// }

// // Convert triangle fan/strip to triangle list
// static void triangulate_primitive(
// 	const std::vector<uint32_t> &indices,
// 	int mode,
// 	std::vector<math::uvec3> &out_triangles)
// {
// 	if (mode == TINYGLTF_MODE_TRIANGLES) {
// 		// Already triangles
// 		for (size_t i = 0; i < indices.size(); i += 3)
// 			out_triangles.push_back(math::uvec3(indices[i], indices[i+1], indices[i+2]));
// 	}
// 	else if (mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
// 		// Triangle strip: each new vertex forms triangle with previous two
// 		for (size_t i = 2; i < indices.size(); i++) {
// 			if (i % 2 == 0)
// 				out_triangles.push_back(math::uvec3(indices[i-2], indices[i-1], indices[i]));
// 			else
// 				out_triangles.push_back(math::uvec3(indices[i-1], indices[i-2], indices[i]));
// 		}
// 	}
// 	else if (mode == TINYGLTF_MODE_TRIANGLE_FAN) {
// 		// Triangle fan: all triangles share first vertex
// 		for (size_t i = 2; i < indices.size(); i++)
// 			out_triangles.push_back(math::uvec3(indices[0], indices[i-1], indices[i]));
// 	}
// }

// // Calculate tangents using MikkTSpace-like approach
// static void calculate_tangents(std::vector<vertex> &vertices, const std::vector<math::uvec3> &triangles) {
// 	// Initialize tangents to zero
// 	for (auto &v : vertices)
// 		v.tangent = math::fvec3(0, 0, 0);
	
// 	// Accumulate tangents per triangle
// 	for (const auto &tri : triangles) {
// 		vertex &v0 = vertices[tri.x];
// 		vertex &v1 = vertices[tri.y];
// 		vertex &v2 = vertices[tri.z];
		
// 		math::fvec3 edge1 = v1.position - v0.position;
// 		math::fvec3 edge2 = v2.position - v0.position;
		
// 		math::fvec2 duv1 = v1.tex_coord - v0.tex_coord;
// 		math::fvec2 duv2 = v2.tex_coord - v0.tex_coord;
		
// 		float det = duv1.x * duv2.y - duv2.x * duv1.y;
// 		if (math::abs(det) < 1e-6f) continue; // Degenerate UV
		
// 		float inv_det = 1.0f / det;
// 		math::fvec3 tangent = (edge1 * duv2.y - edge2 * duv1.y) * inv_det;
		
// 		// Accumulate (will normalize later)
// 		v0.tangent = v0.tangent + tangent;
// 		v1.tangent = v1.tangent + tangent;
// 		v2.tangent = v2.tangent + tangent;
// 	}
	
// 	// Orthogonalize and normalize
// 	for (auto &v : vertices) {
// 		// Gram-Schmidt orthogonalize against normal
// 		v.tangent = v.tangent - v.normal * math::dot(v.normal, v.tangent);
		
// 		float len = math::length(v.tangent);
// 		if (len > 1e-6f)
// 			v.tangent = v.tangent / len;
// 		else
// 			v.tangent = math::fvec3(1, 0, 0); // Fallback
// 	}
// }

// void renderer::load_gltf(const std::filesystem::path &path) {
// 	// Load file
	
// 	tinygltf::Model model;
// 	tinygltf::TinyGLTF loader;
// 	std::string err, warn;
	
// 	bool success = loader.LoadASCIIFromFile(&model, &err, &warn, path.string());
// 	if (!warn.empty())
// 		std::cerr << "glTF warning: " << warn << std::endl;
// 	if (!success || !err.empty())
// 		throw std::runtime_error("Failed to load glTF: " + err);
	
// 	// Create materials
	
// 	std::vector<std::shared_ptr<material>> materials;
// 	materials.reserve(model.materials.size());
	
// 	for (const auto &gltf_mat : model.materials) {
// 		auto material = std::make_shared<core::material>();
		
// 		const auto &pbr = gltf_mat.pbrMetallicRoughness;
		
// 		// Extract PBR factors
// 		material->albedo_fac = fvec3(
// 			pbr.baseColorFactor[0],
// 			pbr.baseColorFactor[1],
// 			pbr.baseColorFactor[2]
// 		);
// 		material->opacity_fac = pbr.baseColorFactor[3];
// 		material->roughness_fac = pbr.roughnessFactor;
// 		material->metallic_fac = pbr.metallicFactor;
// 		material->emissive_fac = fvec3(
// 			gltf_mat.emissiveFactor[0],
// 			gltf_mat.emissiveFactor[1],
// 			gltf_mat.emissiveFactor[2]
// 		);
		
// 		auto gltf_dir = path.parent_path();
		
// 		// Helper to load texture from texture index
// 		auto load_texture = [&](int tex_idx, bool srgb) -> std::shared_ptr<image::texture> {
// 			if (tex_idx < 0) return nullptr;
// 			const auto &tex = model.textures[tex_idx];
// 			if (tex.source < 0) return nullptr;
// 			const auto &img = model.images[tex.source];
			
// 			// TinyGLTF stores URI or embedded data
// 			if (!img.uri.empty()) {
// 				return get_cached_texture(gltf_dir / img.uri, srgb);
// 			}
// 			// BEHAVIOR CHANGE: Embedded images not handled here
// 			// You'd need to pass img.image buffer to texture loader
// 			return nullptr;
// 		};
		
// 		// Normal map
// 		if (gltf_mat.normalTexture.index >= 0)
// 			material->normal_tex = load_texture(gltf_mat.normalTexture.index, false);
		
// 		// Base color (albedo + opacity)
// 		if (pbr.baseColorTexture.index >= 0) {
// 			auto albedo_opacity_tex = load_texture(pbr.baseColorTexture.index, true);
// 			material->albedo_tex = albedo_opacity_tex;
// 			if (gltf_mat.alphaMode != "OPAQUE")
// 				material->opacity_tex = albedo_opacity_tex;
// 		}
		
// 		// Occlusion
// 		if (gltf_mat.occlusionTexture.index >= 0)
// 			material->occlusion_tex = load_texture(gltf_mat.occlusionTexture.index, false);
		
// 		// Metallic-roughness (packed in single texture)
// 		if (pbr.metallicRoughnessTexture.index >= 0) {
// 			auto roughness_metallic_tex = load_texture(pbr.metallicRoughnessTexture.index, false);
// 			material->roughness_tex = roughness_metallic_tex;
// 			material->metallic_tex = roughness_metallic_tex;
// 		}
		
// 		// Emissive
// 		if (gltf_mat.emissiveTexture.index >= 0)
// 			material->emissive_tex = load_texture(gltf_mat.emissiveTexture.index, true);
		
// 		// Shadow catcher detection
// 		if (gltf_mat.name.find("shadow") != std::string::npos &&
// 		    gltf_mat.name.find("catcher") != std::string::npos)
// 			material->shadow_catcher = true;
		
// 		materials.push_back(material);
// 	}
	
// 	// Create meshes
	
// 	std::vector<scene::model::surface> surfaces;
	
// 	for (const auto &gltf_mesh : model.meshes) {
// 		for (const auto &primitive : gltf_mesh.primitives) {
// 			// BEHAVIOR CHANGE: Only handle triangles, skip other modes
// 			if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
// 				continue;
			
// 			auto mesh = std::make_shared<core::mesh>();
			
// 			// Helper to access attribute data
// 			auto get_accessor = [&](const std::string &attr_name) -> const tinygltf::Accessor* {
// 				auto it = primitive.attributes.find(attr_name);
// 				if (it == primitive.attributes.end()) return nullptr;
// 				return &model.accessors[it->second];
// 			};
			
// 			const auto *pos_acc = get_accessor("POSITION");
// 			const auto *uv_acc = get_accessor("TEXCOORD_0");
// 			const auto *norm_acc = get_accessor("NORMAL");
// 			const auto *tan_acc = get_accessor("TANGENT");
			
// 			if (!pos_acc)
// 				throw std::runtime_error("Mesh primitive missing POSITION attribute");
			
// 			size_t vertex_count = pos_acc->count;
// 			mesh->vertices.resize(vertex_count);
			
// 			// Extract positions
// 			const auto &pos_view = model.bufferViews[pos_acc->bufferView];
// 			const auto &pos_buf = model.buffers[pos_view.buffer];
// 			const float *positions = reinterpret_cast<const float*>(
// 				&pos_buf.data[pos_view.byteOffset + pos_acc->byteOffset]
// 			);
			
// 			for (size_t i = 0; i < vertex_count; i++) {
// 				mesh->vertices[i].position.x = positions[i * 3 + 0];
// 				mesh->vertices[i].position.y = positions[i * 3 + 1];
// 				mesh->vertices[i].position.z = positions[i * 3 + 2];
// 			}
			
// 			// Extract UVs
// 			if (uv_acc) {
// 				const auto &uv_view = model.bufferViews[uv_acc->bufferView];
// 				const auto &uv_buf = model.buffers[uv_view.buffer];
// 				const float *uvs = reinterpret_cast<const float*>(
// 					&uv_buf.data[uv_view.byteOffset + uv_acc->byteOffset]
// 				);
				
// 				for (size_t i = 0; i < vertex_count; i++) {
// 					mesh->vertices[i].tex_coord.x = uvs[i * 2 + 0];
// 					mesh->vertices[i].tex_coord.y = uvs[i * 2 + 1];
// 				}
// 			}
			
// 			// Extract normals
// 			if (norm_acc) {
// 				const auto &norm_view = model.bufferViews[norm_acc->bufferView];
// 				const auto &norm_buf = model.buffers[norm_view.buffer];
// 				const float *normals = reinterpret_cast<const float*>(
// 					&norm_buf.data[norm_view.byteOffset + norm_acc->byteOffset]
// 				);
				
// 				for (size_t i = 0; i < vertex_count; i++) {
// 					mesh->vertices[i].normal.x = normals[i * 3 + 0];
// 					mesh->vertices[i].normal.y = normals[i * 3 + 1];
// 					mesh->vertices[i].normal.z = normals[i * 3 + 2];
// 				}
// 			}
			
// 			// Extract tangents
// 			// BEHAVIOR CHANGE: If tangents missing, they'll be zero
// 			// Original Assimp auto-calculated them
// 			if (tan_acc) {
// 				const auto &tan_view = model.bufferViews[tan_acc->bufferView];
// 				const auto &tan_buf = model.buffers[tan_view.buffer];
// 				const float *tangents = reinterpret_cast<const float*>(
// 					&tan_buf.data[tan_view.byteOffset + tan_acc->byteOffset]
// 				);
				
// 				for (size_t i = 0; i < vertex_count; i++) {
// 					// glTF tangents are vec4, we only use xyz
// 					mesh->vertices[i].tangent.x = tangents[i * 4 + 0];
// 					mesh->vertices[i].tangent.y = tangents[i * 4 + 1];
// 					mesh->vertices[i].tangent.z = tangents[i * 4 + 2];
// 				}
// 			}
			
// 			// Extract indices into temporary buffer
//             const auto &idx_acc = model.accessors[primitive.indices];
//             const auto &idx_view = model.bufferViews[idx_acc.bufferView];
//             const auto &idx_buf = model.buffers[idx_view.buffer];
//             const uint8_t *idx_data = &idx_buf.data[idx_view.byteOffset + idx_acc.byteOffset];
            
//             std::vector<uint32_t> indices;
//             indices.resize(idx_acc.count);
            
//             for (size_t i = 0; i < idx_acc.count; i++) {
//                 if (idx_acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
//                     const uint16_t *idx_ptr = reinterpret_cast<const uint16_t*>(idx_data);
//                     indices[i] = idx_ptr[i];
//                 } else if (idx_acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
//                     const uint32_t *idx_ptr = reinterpret_cast<const uint32_t*>(idx_data);
//                     indices[i] = idx_ptr[i];
//                 } else {
//                     indices[i] = idx_data[i];
//                 }
//             }

//             triangulate_primitive(indices, primitive.mode, mesh->triangles);

//             // Calculate tangents if not present
//             if (!tan_acc)
//                 calculate_tangents(mesh->vertices, mesh->triangles);
			
// 			mesh->recalculate_aabb();
// 			mesh->build_kd_tree();
			
// 			int mat_idx = primitive.material >= 0 ? primitive.material : 0;
// 			surfaces.push_back({ mesh, materials[mat_idx] });
// 		}
// 	}
	
// 	// Find camera node

//     int scene_idx = model.defaultScene >= 0 ? model.defaultScene : 0;
//     const auto &scene = model.scenes[scene_idx];

// 	// Find first camera in scene
//     int camera_node_idx = -1;
//     int camera_idx = -1;

//     std::function<void(int)> find_camera;
//     find_camera = [&](int node_idx) {
//         if (camera_node_idx >= 0) return; // Already found
        
//         const auto &node = model.nodes[node_idx];
//         if (node.camera >= 0) {
//             camera_node_idx = node_idx;
//             camera_idx = node.camera;
//             return;
//         }
        
//         for (int child_idx : node.children)
//             find_camera(child_idx);
//     };

//     for (int node_idx : scene.nodes) {
//         find_camera(node_idx);
//         if (camera_node_idx >= 0) break;
//     }

//     if (camera_node_idx < 0)
//         throw std::runtime_error("Scene does not contain any camera.");
	
// 	// Find sun light node
// 	// Requires KHR_lights_punctual extension
	
// 	// Find first directional light in scene
//     int sun_light_node_idx = -1;
//     int sun_light_idx = -1;

//     auto ext_it = model.extensions.find("KHR_lights_punctual");
//     if (ext_it != model.extensions.end()) {
//         const auto &lights_array = ext_it->second.Get("lights");
        
//         std::function<void(int)> find_sun_light;
//         find_sun_light = [&](int node_idx) {
//             if (sun_light_node_idx >= 0) return; // Already found
            
//             const auto &node = model.nodes[node_idx];
//             auto light_ext = node.extensions.find("KHR_lights_punctual");
//             if (light_ext != node.extensions.end()) {
//                 int light_idx = light_ext->second.Get("light").GetNumberAsInt();
                
//                 if (lights_array.IsArray() && light_idx < lights_array.ArrayLen()) {
//                     const auto &light = lights_array.Get(light_idx);
//                     if (light.Has("type") && light.Get("type").Get<std::string>() == "directional") {
//                         sun_light_node_idx = node_idx;
//                         sun_light_idx = light_idx;
//                         return;
//                     }
//                 }
//             }
            
//             for (int child_idx : node.children)
//                 find_sun_light(child_idx);
//         };
        
//         for (int node_idx : scene.nodes) {
//             find_sun_light(node_idx);
//             if (sun_light_node_idx >= 0) break;
//         }
//     }
	
// 	// Instantiate nodes
	
//     root = std::make_shared<scene::entity>();
// 	std::stack<std::tuple<std::shared_ptr<entity>, int>> stack;
	
// 	for (int node_idx : scene.nodes)
// 		stack.push({ root, node_idx });
	
// 	while (!stack.empty()) {
// 		auto [parent, node_idx] = stack.top();
// 		stack.pop();
		
// 		const auto &gltf_node = model.nodes[node_idx];
		
// 		// Set properties
		
// 		auto entity = std::make_shared<scene::entity>();
// 		entity->set_name(gltf_node.name);
		
// 		// Extract transform (glTF supports TRS or matrix)
// 		fvec3 translation(0, 0, 0);
// 		fmat3 rotation_scale = fmat3::identity;
		
// 		if (!gltf_node.matrix.empty()) {
// 			// Matrix form (column-major)
// 			const auto &m = gltf_node.matrix;
// 			translation = fvec3(m[12], m[13], m[14]);
// 			rotation_scale = fmat3(
// 				m[0], m[4], m[8],
// 				m[1], m[5], m[9],
// 				m[2], m[6], m[10]
// 			);
// 		} else {
//             // TRS form
//             if (!gltf_node.translation.empty())
//                 translation = fvec3(gltf_node.translation[0], gltf_node.translation[1], gltf_node.translation[2]);
            
//             fmat3 rotation_mat = fmat3::identity;
//             if (!gltf_node.rotation.empty()) {
//                 // // glTF quaternion is [x, y, z, w]
//                 float x = gltf_node.rotation[0];
//                 float y = gltf_node.rotation[1];
//                 float z = gltf_node.rotation[2];
//                 float w = gltf_node.rotation[3];
                
//                 // // Quaternion to rotation matrix
//                 // rotation_mat = fmat3(
//                 //     1 - 2*(y*y + z*z),     2*(x*y - w*z),     2*(x*z + w*y),
//                 //         2*(x*y + w*z), 1 - 2*(x*x + z*z),     2*(y*z - w*x),
//                 //         2*(x*z - w*y),     2*(y*z + w*x), 1 - 2*(x*x + y*y)
//                 // );
//                 rotation_mat = quat(w, x, y, z).to_basis();
//             }
            
//             fmat3 scale_mat = fmat3::identity;
//             if (!gltf_node.scale.empty()) {
//                 scale_mat = fmat3(
//                     gltf_node.scale[0], 0, 0,
//                     0, gltf_node.scale[1], 0,
//                     0, 0, gltf_node.scale[2]
//                 );
//             }
            
//             rotation_scale = rotation_mat * scale_mat;
//         }
		
// 		entity->set_local_transform(transform(translation, rotation_scale));
		
// 		// Add components
		
// 		if (gltf_node.mesh >= 0) {
// 			auto model_comp = entity->add_component<scene::model>();
			
// 			// Collect all primitives from this mesh
// 			const auto &gltf_mesh = model.meshes[gltf_node.mesh];
// 			size_t surface_start = 0;
			
// 			// Find which surfaces belong to this mesh
// 			for (size_t i = 0; i < model.meshes.size(); i++) {
// 				if (i == gltf_node.mesh) {
// 					for (size_t j = 0; j < gltf_mesh.primitives.size(); j++) {
// 						if (gltf_mesh.primitives[j].mode == TINYGLTF_MODE_TRIANGLES)
// 							model_comp->surfaces.push_back(surfaces[surface_start + j]);
// 					}
// 					break;
// 				}
// 				// Count surfaces from previous meshes
// 				for (const auto &prim : model.meshes[i].primitives)
// 					if (prim.mode == TINYGLTF_MODE_TRIANGLES)
// 						surface_start++;
// 			}
			
// 			model_comp->recalculate_aabb();
// 		}
		
// 		if (node_idx == camera_node_idx) {
// 			auto camera = entity->add_component<scene::camera>();
// 			this->camera = camera;
			
// 			// glTF perspective camera stores yfov directly
//             const auto &gltf_camera = model.cameras[camera_idx];
// 			if (gltf_camera.type == "perspective")
// 				camera->set_fov(gltf_camera.perspective.yfov);
// 			// Orthographic cameras not handled
// 		}
		
// 		if (sun_light_node_idx >= 0 && node_idx == sun_light_node_idx) {
// 			auto sun_light = entity->add_component<scene::sun_light>();
// 			this->sun_light = sun_light;
			
// 			// Extract light properties from extension
// 			auto ext_it = model.extensions.find("KHR_lights_punctual");
// 			if (ext_it != model.extensions.end()) {
// 				const auto &lights = ext_it->second.Get("lights");
// 				const auto &light = lights.Get(sun_light_idx);
				
// 				if (light.Has("color")) {
// 					const auto &color = light.Get("color");
// 					sun_light->energy = fvec3(
// 						color.Get(0).GetNumberAsDouble(),
// 						color.Get(1).GetNumberAsDouble(),
// 						color.Get(2).GetNumberAsDouble()
// 					);
// 				}
				
// 				if (light.Has("intensity")) {
// 					float intensity = light.Get("intensity").GetNumberAsDouble();
// 					sun_light->energy = sun_light->energy * intensity;
// 				}
// 			}
// 		}
		
// 		// Queue children
		
// 		for (int child_idx : gltf_node.children)
// 			stack.push({ entity, child_idx });
		
// 		// Add to hierarchy

// 		if (parent)
// 			entity->set_parent(parent->shared_from_this());
// 	}
	
// 	if (!camera)
// 		throw std::runtime_error("Scene is missing a camera.");
// }

// } // namespace core
