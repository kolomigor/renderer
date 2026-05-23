#include "world.h"

#include <utility>

namespace renderer {

void World::addTriangle(Triangle triangle) {
	triangles_.push_back(std::move(triangle));
}

void World::addMesh(Mesh mesh) {
	for (Triangle& triangle : mesh.triangles) {
		addTriangle(std::move(triangle));
	}
}

const std::vector<Triangle>& World::triangles() const {
	return triangles_;
}

} // namespace renderer
