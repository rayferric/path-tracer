#include "../scene.hpp"

// helper to load the gltf file and return the model
static tinygltf::Model load_gltf_file(const std::filesystem::path &file) {
	tinygltf::Model gltf_model;
	tinygltf::TinyGLTF gltf_loader;
	std::string err, warn;

	bool ret = gltf_loader.LoadASCIIFromFile(&gltf_model, &err, &warn, file.string());
	if (!ret) {
		ret = gltf_loader.LoadBinaryFromFile(&gltf_model, &err, &warn, file.string());
	}
	if (!ret) {
		throw std::runtime_error("Failed to load GLTF: " + err);
	}
	if (!warn.empty()) {
		std::cerr << "GLTF warning: " << warn << std::endl;
	}

	return gltf_model;
}

// extract all texture data and metadata
static void extract_textures(const tinygltf::Model &gltf_model, std::vector<texture_info> &tex_infos, std::vector<uint8_t> &tex_data_vec) {
	for (const auto &tex : gltf_model.textures) {
		if (tex.source < 0) {
			continue;
		}
		const auto &img = gltf_model.images[tex.source];

		texture_info info = {};
		info.data_offset = (uint32_t)tex_data_vec.size();
		info.format = texture_format::linear_noncolor;

		// handle uri-based images
		if (!img.uri.empty() && img.image.empty()) {
			int w, h, channels;
			stbi_set_flip_vertically_on_load(true);
			unsigned char *loaded_data = stbi_load(img.uri.c_str(), &w, &h, &channels, 0);
			stbi_set_flip_vertically_on_load(false);
			if (!loaded_data) {
				throw std::runtime_error("Failed to load texture image: " + img.uri);
			}

			info.width = w;
			info.height = h;
			info.channels = channels;

			size_t img_size = w * h * channels;
			tex_data_vec.insert(tex_data_vec.end(), loaded_data, loaded_data + img_size);
			stbi_image_free(loaded_data);
		}
		// handle embedded image data
		else {
			info.width = img.width;
			info.height = img.height;
			info.channels = img.component;

			size_t img_size = img.width * img.height * img.component * (img.bits == 16 ? 2 : 1);
			tex_data_vec.insert(tex_data_vec.end(), img.image.begin(), img.image.begin() + img_size);
		}

		tex_infos.push_back(info);
	}
}

// extract materials with texture mapping
static void extract_materials(const tinygltf::Model &gltf_model, std::vector<texture_info> &tex_infos, std::vector<material> &mats) {
	// default material
	material m = {};
	m.albedo = glm::fvec3(0.8f);
	m.opacity = 1.0f;
	m.roughness = 0.5f;
	m.metallic = 0.0f;
	m.emissive = glm::fvec3(0.0f);
	m.ior = 1.5f;
	m.shadow_catcher = false;
	m.albedo_tex_idx = m.opacity_tex_idx = m.roughness_tex_idx = m.metallic_tex_idx = m.emissive_tex_idx = m.normal_tex_idx = -1;
	mats.push_back(m);

	// map gltf texture index to our texture index and set format
	auto map_texture = [&](int gltf_tex_idx, texture_format fmt) -> int32_t {
		if (gltf_tex_idx < 0) {
			return -1;
		}
		if (gltf_tex_idx >= (int)gltf_model.textures.size()) {
			return -1;
		}

		const auto &tex = gltf_model.textures[gltf_tex_idx];
		if (tex.source < 0) {
			return -1;
		}

		int our_idx = 0;
		for (int i = 0; i < gltf_tex_idx; ++i) {
			if (gltf_model.textures[i].source >= 0) {
				our_idx++;
			}
		}

		if (our_idx >= 0 && our_idx < (int)tex_infos.size()) {
			tex_infos[our_idx].format = fmt;
		}
		return our_idx;
	};

	for (const auto &mat : gltf_model.materials) {
		material m = {};

		const auto &pbr = mat.pbrMetallicRoughness;
		m.albedo = glm::fvec3(pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2]);
		m.opacity = pbr.baseColorFactor[3];
		m.metallic = pbr.metallicFactor;
		m.roughness = pbr.roughnessFactor;
		m.emissive = glm::fvec3(mat.emissiveFactor[0], mat.emissiveFactor[1], mat.emissiveFactor[2]);

		m.ior = 1.5f;
		if (mat.extensions.count("KHR_materials_ior")) {
			auto &ior_ext = mat.extensions.at("KHR_materials_ior");
			if (ior_ext.Has("ior")) {
				m.ior = ior_ext.Get("ior").GetNumberAsDouble();
			}
		}

		m.shadow_catcher = false;
		if (mat.name.size() > 0) {
			std::string name_lower = mat.name;
			std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
			if (name_lower.find("shadow") != std::string::npos && name_lower.find("catcher") != std::string::npos) {
				m.shadow_catcher = true;
			}
		}

		m.albedo_tex_idx = map_texture(pbr.baseColorTexture.index, texture_format::srgb_color);
		m.metallic_tex_idx = map_texture(pbr.metallicRoughnessTexture.index, texture_format::linear_noncolor);
		m.roughness_tex_idx = m.metallic_tex_idx;
		m.normal_tex_idx = map_texture(mat.normalTexture.index, texture_format::linear_noncolor);
		m.emissive_tex_idx = map_texture(mat.emissiveTexture.index, texture_format::srgb_color);

		m.opacity_tex_idx = -1;
		if (m.albedo_tex_idx >= 0 && tex_infos[m.albedo_tex_idx].channels == 4) {
			m.opacity_tex_idx = m.albedo_tex_idx;
		}

		mats.push_back(m);
	}
}

// extract geometry into triangles and attributes
static void extract_geometry(const tinygltf::Model &gltf_model, std::vector<triangle> &tris, std::vector<triangle_ext> &tris_ext) {
	auto get_accessor_data = [&](int accessor_idx) -> const uint8_t * {
		if (accessor_idx < 0) {
			return nullptr;
		}
		const auto &accessor = gltf_model.accessors[accessor_idx];
		const auto &bufferView = gltf_model.bufferViews[accessor.bufferView];
		const auto &buffer = gltf_model.buffers[bufferView.buffer];
		return buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
	};

	auto get_node_transform = [&](const tinygltf::Node &node) -> glm::mat4 {
		if (node.matrix.size() == 16) {
			// matrix is stored column-major
			return glm::make_mat4(node.matrix.data());
		}

		glm::mat4 transform(1.0f);

		if (node.translation.size() == 3) {
			transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
		}

		if (node.rotation.size() == 4) {
			glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			transform *= glm::mat4_cast(q);
		}

		if (node.scale.size() == 3) {
			transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
		}

		return transform;
	};

	std::function<void(int, const glm::mat4 &)> process_node = [&](int node_idx, const glm::mat4 &parent_transform) {
		const auto &node = gltf_model.nodes[node_idx];
		glm::mat4 local_transform = get_node_transform(node);
		glm::mat4 world_transform = parent_transform * local_transform;
		glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(world_transform)));

		if (node.mesh >= 0) {
			const auto &mesh = gltf_model.meshes[node.mesh];

			for (const auto &prim : mesh.primitives) {
				if (prim.mode != TINYGLTF_MODE_TRIANGLES) {
					continue;
				}

				int mat_idx = prim.material >= 0 ? prim.material + 1 : 0; // +1 skips default

				auto pos_it = prim.attributes.find("POSITION");
				if (pos_it == prim.attributes.end()) {
					continue;
				}

				const auto &pos_accessor = gltf_model.accessors[pos_it->second];
				const uint8_t *pos_data = get_accessor_data(pos_it->second);

				const uint8_t *normal_data = nullptr;
				size_t normal_count = 0;
				auto normal_it = prim.attributes.find("NORMAL");
				if (normal_it != prim.attributes.end()) {
					normal_data = get_accessor_data(normal_it->second);
					normal_count = gltf_model.accessors[normal_it->second].count;
				}

				const uint8_t *uv_data = nullptr;
				size_t uv_count = 0;
				auto uv_it = prim.attributes.find("TEXCOORD_0");
				if (uv_it != prim.attributes.end()) {
					uv_data = get_accessor_data(uv_it->second);
					uv_count = gltf_model.accessors[uv_it->second].count;
				}

				const auto &idx_accessor = gltf_model.accessors[prim.indices];
				const uint8_t *idx_data = get_accessor_data(prim.indices);

				auto get_index = [&](size_t i) -> uint32_t {
					if (idx_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
						return ((const uint16_t *)idx_data)[i];
					} else if (idx_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
						return ((const uint32_t *)idx_data)[i];
					} else if (idx_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
						return ((const uint8_t *)idx_data)[i];
					}
					return 0;
				};

				for (size_t i = 0; i < idx_accessor.count; i += 3) {
					uint32_t i0 = get_index(i);
					uint32_t i1 = get_index(i + 1);
					uint32_t i2 = get_index(i + 2);

					triangle tri = {};
					triangle_ext attr = {};

					glm::vec3 p0 = *((const glm::fvec3 *)(pos_data + i0 * sizeof(glm::fvec3)));
					glm::vec3 p1 = *((const glm::fvec3 *)(pos_data + i1 * sizeof(glm::fvec3)));
					glm::vec3 p2 = *((const glm::fvec3 *)(pos_data + i2 * sizeof(glm::fvec3)));

					tri.corners[0] = glm::vec3(world_transform * glm::vec4(p0, 1.0f));
					tri.corners[1] = glm::vec3(world_transform * glm::vec4(p1, 1.0f));
					tri.corners[2] = glm::vec3(world_transform * glm::vec4(p2, 1.0f));

					if (normal_data && i0 < normal_count && i1 < normal_count && i2 < normal_count) {
						glm::vec3 n0 = *((const glm::fvec3 *)(normal_data + i0 * sizeof(glm::fvec3)));
						glm::vec3 n1 = *((const glm::fvec3 *)(normal_data + i1 * sizeof(glm::fvec3)));
						glm::vec3 n2 = *((const glm::fvec3 *)(normal_data + i2 * sizeof(glm::fvec3)));

						attr.normal[0] = glm::normalize(normal_matrix * n0);
						attr.normal[1] = glm::normalize(normal_matrix * n1);
						attr.normal[2] = glm::normalize(normal_matrix * n2);
					} else {
						glm::fvec3 e1 = tri.corners[1] - tri.corners[0];
						glm::fvec3 e2 = tri.corners[2] - tri.corners[0];
						glm::fvec3 n = glm::normalize(glm::cross(e1, e2));
						attr.normal[0] = attr.normal[1] = attr.normal[2] = n;
					}

					if (uv_data && i0 < uv_count && i1 < uv_count && i2 < uv_count) {
						attr.uv[0] = *((const glm::fvec2 *)(uv_data + i0 * sizeof(glm::fvec2)));
						attr.uv[1] = *((const glm::fvec2 *)(uv_data + i1 * sizeof(glm::fvec2)));
						attr.uv[2] = *((const glm::fvec2 *)(uv_data + i2 * sizeof(glm::fvec2)));
					} else {
						attr.uv[0] = attr.uv[1] = attr.uv[2] = glm::fvec2(0.0f);
					}

					attr.mat_idx = mat_idx;

					tris_ext.push_back(attr);
					tris.push_back(tri);
				}
			}
		}

		for (int child_idx : node.children) {
			process_node(child_idx, world_transform);
		}
	};

	// process all root nodes from the default scene
	int scene_idx = gltf_model.defaultScene >= 0 ? gltf_model.defaultScene : 0;
	if (scene_idx < gltf_model.scenes.size()) {
		const auto &scene = gltf_model.scenes[scene_idx];
		for (int node_idx : scene.nodes) {
			process_node(node_idx, glm::mat4(1.0f));
		}
	}
}

// extract camera transform and fov
static void try_extract_camera(const tinygltf::Model &gltf_model, glm::mat4 &camera_transform, float &camera_vfov_rad) {
	if (gltf_model.cameras.empty()) {
		return;
	}

	const auto &cam = gltf_model.cameras[0];
	if (cam.type == "perspective") {
		camera_vfov_rad = cam.perspective.yfov;
	}

	for (const auto &node : gltf_model.nodes) {
		if (node.camera == 0) {
			if (node.matrix.size() == 16) {
				camera_transform = glm::make_mat4(node.matrix.data());
			} else {
				glm::fvec3 t(0.0f), s(1.0f);
				glm::fquat r(1.0f, 0.0f, 0.0f, 0.0f);

				if (node.translation.size() == 3) {
					t = glm::fvec3(node.translation[0], node.translation[1], node.translation[2]);
				}
				if (node.rotation.size() == 4) {
					r = glm::fquat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
				}
				if (node.scale.size() == 3) {
					s = glm::fvec3(node.scale[0], node.scale[1], node.scale[2]);
				}

				camera_transform = glm::translate(glm::mat4(1.0f), t) * glm::mat4_cast(r) * glm::scale(glm::mat4(1.0f), s);
			}
			return;
		}
	}
}

// extract sunlight direction and properties from first directional light
static void try_extract_sunlight(const tinygltf::Model &gltf_model, glm::fvec3 &sunlight_dir, glm::fvec3 &sunlight_intensity) {
	// check if KHR_lights_punctual extension exists
	auto ext_it = gltf_model.extensions.find("KHR_lights_punctual");
	if (ext_it == gltf_model.extensions.end()) {
		return;
	}

	const auto &lights_ext = ext_it->second;
	if (!lights_ext.Has("lights")) {
		return;
	}

	const auto &lights = lights_ext.Get("lights");
	if (!lights.IsArray() || lights.ArrayLen() == 0) {
		return;
	}

	// find first directional light
	int directional_light_idx = -1;
	for (size_t i = 0; i < lights.ArrayLen(); i++) {
		const auto &light = lights.Get(i);
		if (light.Has("type")) {
			std::string type = light.Get("type").Get<std::string>();
			if (type == "directional") {
				directional_light_idx = i;

				// extract intensity and color
				if (light.Has("intensity")) {
					float intensity = light.Get("intensity").GetNumberAsDouble();
					glm::fvec3 color(1.0f);
					if (light.Has("color")) {
						const auto &color_arr = light.Get("color");
						if (color_arr.IsArray() && color_arr.ArrayLen() >= 3) {
							color.r = color_arr.Get(0).GetNumberAsDouble();
							color.g = color_arr.Get(1).GetNumberAsDouble();
							color.b = color_arr.Get(2).GetNumberAsDouble();
						}
					}
					sunlight_intensity = color * intensity;
				}

				break;
			}
		}
	}

	if (directional_light_idx < 0) {
		return;
	}

	// find node that references this light
	for (const auto &node : gltf_model.nodes) {
		auto node_ext_it = node.extensions.find("KHR_lights_punctual");
		if (node_ext_it == node.extensions.end()) {
			continue;
		}

		const auto &light_ext = node_ext_it->second;
		if (!light_ext.Has("light")) {
			continue;
		}

		int light_idx = light_ext.Get("light").Get<int>();
		if (light_idx == directional_light_idx) {
			// extract transform
			glm::mat4 transform(1.0f);
			if (node.matrix.size() == 16) {
				transform = glm::make_mat4(node.matrix.data());
			} else {
				glm::fvec3 t(0.0f), s(1.0f);
				glm::fquat r(1.0f, 0.0f, 0.0f, 0.0f);

				if (node.translation.size() == 3) {
					t = glm::fvec3(node.translation[0], node.translation[1], node.translation[2]);
				}
				if (node.rotation.size() == 4) {
					r = glm::fquat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
				}
				if (node.scale.size() == 3) {
					s = glm::fvec3(node.scale[0], node.scale[1], node.scale[2]);
				}

				transform = glm::translate(glm::mat4(1.0f), t) * glm::mat4_cast(r) * glm::scale(glm::mat4(1.0f), s);
			}

			// directional lights point along -Z in local space
			sunlight_dir = glm::normalize(glm::fvec3(transform * glm::fvec4(0.0f, 0.0f, -1.0f, 0.0f)));
			break;
		}
	}
}

template <typename T> T *copy_vector_to_array(const std::vector<T> &vec, uint32_t &out_count) {
	out_count = (uint32_t)vec.size();
	if (out_count == 0) {
		return nullptr;
	}
	T *arr = (T*)malloc(out_count * sizeof(T));
	memcpy(arr, vec.data(), out_count * sizeof(T));
	return arr;
}

void scene::load_gltf(const std::filesystem::path &file) {
	std::cout << "load_gltf" << std::endl;
	tinygltf::Model gltf_model = load_gltf_file(file);

	std::vector<texture_info> tex_infos;
	std::vector<uint8_t> tex_data_vec;
	std::vector<material> mats;
	std::vector<triangle> tris;
	std::vector<triangle_ext> tris_ext;

	extract_textures(gltf_model, tex_infos, tex_data_vec);
	extract_materials(gltf_model, tex_infos, mats);
	extract_geometry(gltf_model, tris, tris_ext);

	texture_data = copy_vector_to_array(tex_data_vec, texture_data_sz);
	textures = copy_vector_to_array(tex_infos, num_textures);
	materials = copy_vector_to_array(mats, num_materials);
	triangles = copy_vector_to_array(tris, num_triangles);
	triangles_ext = copy_vector_to_array(tris_ext, num_triangles);

	try_extract_camera(gltf_model, camera_transform, camera_vfov_rad);
	try_extract_sunlight(gltf_model, sunlight_dir, sunlight_intensity);
}
