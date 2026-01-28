#pragma once
#include "glm.h"
#include <array>
#include <vector>

struct ClipVertex {
	glm::vec4 position;
    glm::vec3 color;
};

using ClipTriangle = std::array<ClipVertex, 3>;

std::vector<ClipTriangle> ClipTriangleNear(const ClipVertex& c0, const ClipVertex& c1,
                                           const ClipVertex& c2);
