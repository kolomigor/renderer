#pragma once

#include "mesh.h"
#include "primitives.h"

#include <vector>

namespace renderer {

class World {
public:
	void addTriangle(Triangle triangle);
	void addMesh(Mesh mesh);
	const std::vector<Triangle>& triangles() const;

private:
	std::vector<Triangle> triangles_;
};

} // namespace renderer
