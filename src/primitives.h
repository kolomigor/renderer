#pragma once

#include "linalg.h"
#include "texture.h"

#include <memory>

namespace renderer {

struct Material {
	vec3 albedo;
	float ambient;
	float diffuse;
	float specular;
	float shininess;
	std::shared_ptr<Texture> texture;
	vec3 emission{0.0f, 0.0f, 0.0f};
};

struct Vertex {
	vec4 position;
	vec3 color;
	vec2 texcoord;
	vec3 world_position;
	vec3 normal;
};

struct Triangle {
	Vertex v0;
	Vertex v1;
	Vertex v2;
	Material material;
};

inline vec3 triangleNormal(const vec3& a, const vec3& b, const vec3& c) {
	const vec3 normal = cross(b - a, c - a);
	if (length(normal) == 0.0f) {
		return {0.0f, 0.0f, 1.0f};
	}
	return normalize(normal);
}

inline Vertex makeVertex(const vec3& position, const vec3& color,
                         const vec2& texcoord = {0.0f, 0.0f},
                         const vec3& normal = {0.0f, 0.0f, 1.0f}) {
	return {point(position), color, texcoord, position, normal};
}

inline Triangle makeTriangle(const vec3& a, const vec3& b, const vec3& c, Material material) {
	const vec3 normal = triangleNormal(a, b, c);
	return {makeVertex(a, material.albedo, {0.0f, 0.0f}, normal),
	        makeVertex(b, material.albedo, {0.0f, 0.0f}, normal),
	        makeVertex(c, material.albedo, {0.0f, 0.0f}, normal),
	        material};
}

} // namespace renderer
