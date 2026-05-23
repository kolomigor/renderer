#pragma once

#include "primitives.h"

#include <filesystem>
#include <vector>

namespace renderer {

struct Mesh {
	std::vector<Triangle> triangles;
};

Mesh loadObjMesh(const std::filesystem::path& filename, Material material, const mat4& transform);

} // namespace renderer
