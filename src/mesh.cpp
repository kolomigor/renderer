#include "mesh.h"

#include <tiny_obj_loader.h>

#include <stdexcept>
#include <string>

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

void addTriangle(Mesh *mesh, const mat4& transform, const tinyobj::attrib_t& attributes,
                 const tinyobj::index_t& a, const tinyobj::index_t& b, const tinyobj::index_t& c,
                 Material material, const std::filesystem::path& filename) {
	const vec3 p0 = transformedPosition(transform, readPosition(attributes, a, filename));
	const vec3 p1 = transformedPosition(transform, readPosition(attributes, b, filename));
	const vec3 p2 = transformedPosition(transform, readPosition(attributes, c, filename));
	mesh->triangles.push_back(makeTriangle(p0, p1, p2, material));
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
	for (const tinyobj::shape_t& shape : shapes) {
		std::size_t index_offset = 0;
		for (const unsigned char face_vertices : shape.mesh.num_face_vertices) {
			if (face_vertices < 3) {
				index_offset += face_vertices;
				continue;
			}
			for (std::size_t vertex = 1; vertex + 1 < face_vertices; ++vertex) {
				addTriangle(&mesh, transform, attributes, shape.mesh.indices[index_offset],
				            shape.mesh.indices[index_offset + vertex],
				            shape.mesh.indices[index_offset + vertex + 1], material, filename);
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
