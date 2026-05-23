#include "renderer.h"

#include "clip.h"
#include "rasterizer.h"

#include <cassert>

namespace renderer {
namespace {

float toScreenX(float normalized_x, int width) {
	return (normalized_x * 0.5f + 0.5f) * static_cast<float>(width);
}

float toScreenY(float normalized_y, int height) {
	return (1.0f - (normalized_y * 0.5f + 0.5f)) * static_cast<float>(height);
}

Vertex transformVertex(const mat4& transform, const Vertex& vertex) {
	return {transform * vertex.position, vertex.color};
}

Triangle transformTriangle(const mat4& transform, const Triangle& triangle) {
	return {transformVertex(transform, triangle.v0), transformVertex(transform, triangle.v1),
	        transformVertex(transform, triangle.v2)};
}

bool isBackFaceInViewSpace(const Triangle& triangle) {
	const vec3 p0 = xyz(triangle.v0.position);
	const vec3 p1 = xyz(triangle.v1.position);
	const vec3 p2 = xyz(triangle.v2.position);
	const vec3 normal = cross(p1 - p0, p2 - p0);
	const vec3 direction_to_camera = -p0;
	return dot(normal, direction_to_camera) <= 0.0f;
}

Vertex projectVertexToScreen(const Vertex& vertex, int width, int height) {
	assert(vertex.position.w > 0.0f);
	assert(vertex.position.z >= -vertex.position.w);

	const vec3 normalized = xyz(vertex.position) / vertex.position.w;
	return {{toScreenX(normalized.x, width), toScreenY(normalized.y, height), normalized.z,
	         1.0f / vertex.position.w},
	        vertex.color};
}

Triangle projectTriangleToScreen(const Triangle& triangle, int width, int height) {
	return {projectVertexToScreen(triangle.v0, width, height),
	        projectVertexToScreen(triangle.v1, width, height),
	        projectVertexToScreen(triangle.v2, width, height)};
}

} // namespace

Renderer::Renderer(Width width, Height height) : width_(width), height_(height) {
	assert(width_ > 0);
	assert(height_ > 0);
}

Picture Renderer::render(const World& world, const Camera& camera) const {
	Picture picture(Width{width_}, Height{height_});
	picture.clear(kBlack);

	const mat4 view = camera.viewMatrix();
	const mat4 view_projection = camera.viewProjectionMatrix();
	const Plane near_plane = nearClipPlaneInClipSpace();
	for (const Triangle& triangle : world.triangles()) {
		const Triangle view_triangle = transformTriangle(view, triangle);
		if (isBackFaceInViewSpace(view_triangle)) {
			continue;
		}

		const Triangle clip_triangle = transformTriangle(view_projection, triangle);
		for (const Triangle& clipped_triangle : clipTriangleBy(near_plane, clip_triangle)) {
			const Triangle screen_triangle =
			    projectTriangleToScreen(clipped_triangle, width_, height_);
			Rasterizer::rasterizeTriangle(screen_triangle, &picture);
		}
	}
	return picture;
}

} // namespace renderer
