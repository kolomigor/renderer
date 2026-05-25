#include "scene.h"

#include "mesh.h"
#include "texture.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace renderer {
namespace {

using Json = nlohmann::json;

Material defaultMaterial() {
	return {{0.8f, 0.8f, 0.8f}, 0.35f, 0.85f, 0.25f, 32.0f};
}

Aspect makeAspect(Width width, Height height) {
	return Aspect{static_cast<float>(width) / static_cast<float>(height)};
}

std::filesystem::path resolvePath(const std::filesystem::path& scene_path,
                                  const std::string& path) {
	std::filesystem::path resolved(path);
	if (resolved.is_relative()) {
		resolved = scene_path.parent_path() / resolved;
	}
	return resolved.lexically_normal();
}

float readFloat(const Json& object, const std::string& key, float fallback) {
	if (!object.contains(key)) {
		return fallback;
	}
	return object.at(key).get<float>();
}

vec3 readVec3(const Json& object, const std::string& key, const vec3& fallback) {
	if (!object.contains(key)) {
		return fallback;
	}
	const Json& value = object.at(key);
	if (!value.is_array() || value.size() != 3) {
		throw std::runtime_error("expected " + key + " to be a vec3 array");
	}
	return {value[0].get<float>(), value[1].get<float>(), value[2].get<float>()};
}

Material readMaterial(const Json& object, const std::filesystem::path& scene_path) {
	Material material = defaultMaterial();
	material.albedo = readVec3(object, "albedo", material.albedo);
	material.ambient = readFloat(object, "ambient", material.ambient);
	material.diffuse = readFloat(object, "diffuse", material.diffuse);
	material.specular = readFloat(object, "specular", material.specular);
	material.shininess = readFloat(object, "shininess", material.shininess);
	material.emission = readVec3(object, "emission", material.emission);
	if (object.contains("texture")) {
		const std::filesystem::path texture_path =
		    resolvePath(scene_path, object.at("texture").get<std::string>());
		material.texture = std::make_shared<Texture>(loadTexture(texture_path));
	}
	return material;
}

std::unordered_map<std::string, Material> readMaterials(const Json& root,
                                                       const std::filesystem::path& scene_path) {
	std::unordered_map<std::string, Material> materials;
	materials.emplace("default", defaultMaterial());
	if (!root.contains("materials")) {
		return materials;
	}
	for (const auto& [name, config] : root.at("materials").items()) {
		materials[name] = readMaterial(config, scene_path);
	}
	return materials;
}

Material materialByName(const std::unordered_map<std::string, Material>& materials,
                        const std::string& name) {
	const auto material = materials.find(name);
	if (material == materials.end()) {
		throw std::runtime_error("unknown material: " + name);
	}
	return material->second;
}

mat4 readTransform(const Json& object) {
	const Json empty = Json::object();
	const Json& transform = object.contains("transform") ? object.at("transform") : empty;
	const vec3 translation = readVec3(transform, "translate", {0.0f, 0.0f, 0.0f});
	const vec3 rotation_degrees = readVec3(transform, "rotate", {0.0f, 0.0f, 0.0f});
	const vec3 scale_value = readVec3(transform, "scale", {1.0f, 1.0f, 1.0f});

	mat4 transform_matrix = identity();
	transform_matrix = translate(transform_matrix, translation);
	transform_matrix = rotate(transform_matrix, radians(rotation_degrees.x), {1.0f, 0.0f, 0.0f});
	transform_matrix = rotate(transform_matrix, radians(rotation_degrees.y), {0.0f, 1.0f, 0.0f});
	transform_matrix = rotate(transform_matrix, radians(rotation_degrees.z), {0.0f, 0.0f, 1.0f});
	transform_matrix = scale(transform_matrix, scale_value);
	return transform_matrix;
}

Camera readCamera(const Json& root, Width width, Height height) {
	const Json& camera = root.at("camera");
	const vec3 position = readVec3(camera, "position", {2.5f, 2.0f, 2.0f});
	const vec3 target = readVec3(camera, "target", {0.0f, 0.0f, -6.0f});
	const Fov fov{readFloat(camera, "fov", 60.0f)};
	const NearPlane near_plane{readFloat(camera, "near", 0.1f)};
	const FarPlane far_plane{readFloat(camera, "far", 100.0f)};
	return {fov, makeAspect(width, height), near_plane, far_plane, position, target};
}

Lights readLights(const Json& root) {
	const Json& lights_config = root.at("lights");
	const Json& ambient = lights_config.at("ambient");
	Lights lights(readVec3(ambient, "color", {1.0f, 1.0f, 1.0f}),
	              readFloat(ambient, "intensity", 0.5f));
	for (const Json& light : lights_config.at("directional")) {
		lights.addDirectionalLight({readVec3(light, "directionToLight", kWorldUp),
		                            readVec3(light, "color", {1.0f, 1.0f, 1.0f}),
		                            readFloat(light, "intensity", 1.0f)});
	}
	if (lights_config.contains("point")) {
		for (const Json& light : lights_config.at("point")) {
			lights.addPointLight({readVec3(light, "position", {0.0f, 0.0f, 0.0f}),
			                      readVec3(light, "color", {1.0f, 1.0f, 1.0f}),
			                      readFloat(light, "intensity", 1.0f),
			                      readFloat(light, "linearAttenuation", 0.09f),
			                      readFloat(light, "quadraticAttenuation", 0.032f)});
		}
	}
	return lights;
}

World readWorld(const Json& root, const std::filesystem::path& scene_path,
                const std::unordered_map<std::string, Material>& materials) {
	World world;
	for (const Json& mesh_config : root.at("meshes")) {
		const std::filesystem::path mesh_path =
		    resolvePath(scene_path, mesh_config.at("path").get<std::string>());
		const Material material =
		    materialByName(materials, mesh_config.value("material", std::string("default")));
		world.addMesh(loadObjMesh(mesh_path, material, readTransform(mesh_config)));
	}
	return world;
}

Json loadJson(const std::filesystem::path& filename) {
	std::ifstream input(filename);
	if (!input) {
		throw std::runtime_error("failed to open scene config: " + filename.string());
	}
	Json root;
	input >> root;
	return root;
}

} // namespace

Scene loadScene(const std::filesystem::path& filename, Width width, Height height) {
	const Json root = loadJson(filename);
	const std::unordered_map<std::string, Material> materials = readMaterials(root, filename);
	return {readWorld(root, filename, materials), readCamera(root, width, height),
	        readLights(root)};
}

} // namespace renderer
