#pragma once

#include "primitives.h"

#include <array>
#include <vector>

namespace renderer {

struct Plane {
	vec4 equation;
};

std::array<Plane, 6> frustumClipPlanesInClipSpace();
std::vector<Triangle> clipTriangleBy(const Plane& plane, const Triangle& triangle);
std::vector<Triangle> clipTrianglesBy(const Plane& plane, const std::vector<Triangle>& triangles);
std::vector<Triangle> clipTriangleByFrustum(const Triangle& triangle);

} // namespace renderer
