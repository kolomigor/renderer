#include "clip.h"

namespace renderer {
namespace {

constexpr Plane kLeftClipPlane{{1.0f, 0.0f, 0.0f, 1.0f}};
constexpr Plane kRightClipPlane{{-1.0f, 0.0f, 0.0f, 1.0f}};
constexpr Plane kBottomClipPlane{{0.0f, 1.0f, 0.0f, 1.0f}};
constexpr Plane kTopClipPlane{{0.0f, -1.0f, 0.0f, 1.0f}};
constexpr Plane kNearClipPlane{{0.0f, 0.0f, 1.0f, 1.0f}};
constexpr Plane kFarClipPlane{{0.0f, 0.0f, -1.0f, 1.0f}};

float signedDistance(const Plane& plane, const Vertex& vertex) {
	return dot(plane.equation, vertex.position);
}

bool isInside(const Plane& plane, const Vertex& vertex) {
	return signedDistance(plane, vertex) >= 0.0f;
}

Vertex interpolate(const Vertex& a, const Vertex& b, float t) {
	Vertex out;
	out.position = a.position + t * (b.position - a.position);
	out.color = a.color + t * (b.color - a.color);
	return out;
}

Vertex intersectSegmentBy(const Plane& plane, const Vertex& a, const Vertex& b) {
	const float distance_a = signedDistance(plane, a);
	const float distance_b = signedDistance(plane, b);
	const float t = distance_a / (distance_a - distance_b);
	return interpolate(a, b, t);
}

std::vector<Vertex> clipPolygonBy(const Plane& plane, const std::vector<Vertex>& vertices) {
	std::vector<Vertex> clipped;
	for (std::size_t current_index = 0; current_index < vertices.size(); ++current_index) {
		const Vertex& current = vertices[current_index];
		const Vertex& next = vertices[(current_index + 1) % vertices.size()];
		const bool current_inside = isInside(plane, current);
		const bool next_inside = isInside(plane, next);

		if (current_inside && next_inside) {
			clipped.push_back(next);
		} else if (current_inside && !next_inside) {
			clipped.push_back(intersectSegmentBy(plane, current, next));
		} else if (!current_inside && next_inside) {
			clipped.push_back(intersectSegmentBy(plane, current, next));
			clipped.push_back(next);
		}
	}
	return clipped;
}

std::vector<Triangle> triangulate(const std::vector<Vertex>& vertices, Material material) {
	std::vector<Triangle> triangles;
	if (vertices.size() < 3) {
		return triangles;
	}
	for (std::size_t index = 1; index + 1 < vertices.size(); ++index) {
		triangles.push_back({vertices[0], vertices[index], vertices[index + 1], material});
	}
	return triangles;
}

} // namespace

std::array<Plane, 6> frustumClipPlanesInClipSpace() {
	return {kLeftClipPlane, kRightClipPlane, kBottomClipPlane,
	        kTopClipPlane,  kNearClipPlane,  kFarClipPlane};
}

std::vector<Triangle> clipTriangleBy(const Plane& plane, const Triangle& triangle) {
	return triangulate(clipPolygonBy(plane, {triangle.v0, triangle.v1, triangle.v2}),
	                   triangle.material);
}

std::vector<Triangle> clipTrianglesBy(const Plane& plane, const std::vector<Triangle>& triangles) {
	std::vector<Triangle> clipped;
	for (const Triangle& triangle : triangles) {
		std::vector<Triangle> parts = clipTriangleBy(plane, triangle);
		clipped.insert(clipped.end(), parts.begin(), parts.end());
	}
	return clipped;
}

std::vector<Triangle> clipTriangleByFrustum(const Triangle& triangle) {
	std::vector<Triangle> clipped{triangle};
	for (const Plane& plane : frustumClipPlanesInClipSpace()) {
		clipped = clipTrianglesBy(plane, clipped);
		if (clipped.empty()) {
			break;
		}
	}
	return clipped;
}

} // namespace renderer
