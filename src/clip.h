#pragma once

#include "primitives.h"

#include <vector>

namespace renderer {

struct Plane {
	vec4 equation;
};

Plane nearClipPlaneInClipSpace();
std::vector<Triangle> clipTriangleBy(const Plane& plane, const Triangle& triangle);
std::vector<Triangle> clipTrianglesBy(const Plane& plane, const std::vector<Triangle>& triangles);

} // namespace renderer
