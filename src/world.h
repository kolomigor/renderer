#pragma once

#include "primitives.h"

#include <vector>

namespace renderer {

class World {
public:
	void addTriangle(Triangle triangle);
	const std::vector<Triangle>& triangles() const;

private:
	std::vector<Triangle> triangles_;
};

} // namespace renderer
