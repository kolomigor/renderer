#include "world.h"

void World::AddTriangle(const Triangle& t) {
	triangles_.push_back(t);
}

const std::vector<Triangle>& World::GetTriangles() const {
	return triangles_;
}