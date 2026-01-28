#pragma once
#include "primitives.h"

class World {
public:
	void AddTriangle(const Triangle&);
	const std::vector<Triangle>& GetTriangles() const;

private:
	std::vector<Triangle> triangles_;
};