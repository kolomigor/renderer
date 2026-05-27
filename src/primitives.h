#pragma once

#include "linalg.h"
#include "texture.h"

#include <memory>

namespace renderer {

struct Material {
	vec3 albedo;
	std::shared_ptr<Texture> texture;
};

struct Vertex {
	vec4 position;
	vec3 color;
	vec2 texcoord;
};

struct Triangle {
	Vertex v0;
	Vertex v1;
	Vertex v2;
	Material material;
};

inline Vertex makeVertex(const vec3& position, const vec3& color,
                         const vec2& texcoord = {0.0f, 0.0f}) {
	return {point(position), color, texcoord};
}

inline Triangle makeTriangle(const vec3& a, const vec3& b, const vec3& c, Material material) {
	return {makeVertex(a, material.albedo), makeVertex(b, material.albedo),
	        makeVertex(c, material.albedo),
	        material};
}

} // namespace renderer
