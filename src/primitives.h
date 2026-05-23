#pragma once

#include "linalg.h"

namespace renderer {

struct Vertex {
	vec4 position;
	vec3 color;
};

struct Triangle {
	Vertex v0;
	Vertex v1;
	Vertex v2;
};

inline Vertex makeVertex(const vec3& position, const vec3& color) {
	return {point(position), color};
}

} // namespace renderer
