#pragma once

#include "linalg.h"

namespace renderer {

struct Material {
	vec3 albedo;
	float ambient;
	float diffuse;
	float specular;
	float shininess;
};

struct Vertex {
	vec4 position;
	vec3 color;
};

struct Triangle {
	Vertex v0;
	Vertex v1;
	Vertex v2;
	Material material;
};

inline Vertex makeVertex(const vec3& position, const vec3& color) {
	return {point(position), color};
}

inline Triangle makeTriangle(const vec3& a, const vec3& b, const vec3& c, Material material) {
	return {makeVertex(a, material.albedo), makeVertex(b, material.albedo),
	        makeVertex(c, material.albedo), material};
}

} // namespace renderer
