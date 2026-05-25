#include "mesh.h"

#include <tiny_obj_loader.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace renderer {
namespace {

vec3 transformedPosition(const mat4& transform, const vec3& position) {
	const vec4 transformed = transform * point(position);
	return xyz(transformed) / transformed.w;
}

vec3 readPosition(const tinyobj::attrib_t& attributes, const tinyobj::index_t& index,
                  const std::filesystem::path& filename) {
	if (index.vertex_index < 0) {
		throw std::runtime_error("missing vertex index in OBJ file: " + filename.string());
	}
	const std::size_t position_index = static_cast<std::size_t>(index.vertex_index) * 3;
	if (position_index + 2 >= attributes.vertices.size()) {
		throw std::runtime_error("vertex index is out of bounds in OBJ file: " + filename.string());
	}
	return {attributes.vertices[position_index], attributes.vertices[position_index + 1],
	        attributes.vertices[position_index + 2]};
}

vec2 readTexcoord(const tinyobj::attrib_t& attributes, const tinyobj::index_t& index,
                  const std::filesystem::path& filename) {
	if (index.texcoord_index < 0) {
		return {0.0f, 0.0f};
	}
	const std::size_t texcoord_index = static_cast<std::size_t>(index.texcoord_index) * 2;
	if (texcoord_index + 1 >= attributes.texcoords.size()) {
		throw std::runtime_error("texture coordinate index is out of bounds in OBJ file: " +
		                         filename.string());
	}
	return {attributes.texcoords[texcoord_index], attributes.texcoords[texcoord_index + 1]};
}

Vertex makeMeshVertex(const vec3& position, const vec2& texcoord, const Material& material,
                      const vec3& normal) {
	return makeVertex(position, material.albedo, texcoord, normal);
}

void addTriangle(Mesh *mesh, const mat4& transform, const tinyobj::attrib_t& attributes,
                 const tinyobj::index_t& a, const tinyobj::index_t& b, const tinyobj::index_t& c,
                 Material material, const std::filesystem::path& filename) {
	const vec3 p0 = transformedPosition(transform, readPosition(attributes, a, filename));
	const vec3 p1 = transformedPosition(transform, readPosition(attributes, b, filename));
	const vec3 p2 = transformedPosition(transform, readPosition(attributes, c, filename));
	const vec3 normal = triangleNormal(p0, p1, p2);
	mesh->triangles.push_back({makeMeshVertex(p0, readTexcoord(attributes, a, filename), material,
	                                          normal),
	                           makeMeshVertex(p1, readTexcoord(attributes, b, filename), material,
	                                          normal),
	                           makeMeshVertex(p2, readTexcoord(attributes, c, filename), material,
	                                          normal),
	                           material});
}

Material objMaterial(const tinyobj::material_t& source, const std::filesystem::path& base_path,
                     Material fallback,
                     std::unordered_map<std::string, std::shared_ptr<Texture>> *texture_cache) {
	fallback.albedo = {source.diffuse[0], source.diffuse[1], source.diffuse[2]};
	fallback.ambient = std::max({source.ambient[0], source.ambient[1], source.ambient[2]});
	fallback.specular = std::max({source.specular[0], source.specular[1], source.specular[2]});
	if (source.shininess > 0.0f) {
		fallback.shininess = source.shininess;
	}
	fallback.emission = {source.emission[0], source.emission[1], source.emission[2]};

	if (!source.diffuse_texname.empty()) {
		const std::filesystem::path texture_path =
		    (base_path / source.diffuse_texname).lexically_normal();
		const std::string cache_key = texture_path.string();
		auto texture = texture_cache->find(cache_key);
		if (texture == texture_cache->end()) {
			texture =
			    texture_cache->emplace(cache_key, std::make_shared<Texture>(loadTexture(texture_path)))
			        .first;
		}
		fallback.texture = texture->second;
	}
	return fallback;
}

} // namespace

Mesh loadObjMesh(const std::filesystem::path& filename, Material material, const mat4& transform) {
	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning;
	std::string error;
	const std::filesystem::path base_path = filename.parent_path();
	const bool loaded =
	    tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error,
	                     filename.string().c_str(), base_path.string().c_str(), true);
	if (!loaded) {
		throw std::runtime_error("failed to load OBJ file " + filename.string() + ": " + error);
	}

	Mesh mesh;
	std::unordered_map<std::string, std::shared_ptr<Texture>> texture_cache;
	for (const tinyobj::shape_t& shape : shapes) {
		std::size_t index_offset = 0;
		for (std::size_t face = 0; face < shape.mesh.num_face_vertices.size(); ++face) {
			const unsigned char face_vertices = shape.mesh.num_face_vertices[face];
			if (face_vertices < 3) {
				index_offset += face_vertices;
				continue;
			}
			Material face_material = material;
			if (face < shape.mesh.material_ids.size()) {
				const int material_id = shape.mesh.material_ids[face];
				if (material_id >= 0 && material_id < static_cast<int>(materials.size())) {
					face_material = objMaterial(materials[static_cast<std::size_t>(material_id)],
					                            base_path, material, &texture_cache);
				}
			}
			for (std::size_t vertex = 1; vertex + 1 < face_vertices; ++vertex) {
				addTriangle(&mesh, transform, attributes, shape.mesh.indices[index_offset],
				            shape.mesh.indices[index_offset + vertex],
				            shape.mesh.indices[index_offset + vertex + 1], face_material, filename);
			}
			index_offset += face_vertices;
		}
	}
	if (mesh.triangles.empty()) {
		throw std::runtime_error("OBJ file has no drawable triangles: " + filename.string());
	}
	return mesh;
}

} // namespace renderer
