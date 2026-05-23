#include "renderer.h"

#include "clip.h"
#include "rasterizer.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace renderer {
namespace {

constexpr float kClipSpaceEpsilon = 0.0001f;

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
	        transformVertex(transform, triangle.v2), triangle.material};
}

Triangle colorTriangle(const Triangle& triangle, const vec3& color) {
	return {{triangle.v0.position, color},
	        {triangle.v1.position, color},
	        {triangle.v2.position, color},
	        triangle.material};
}

bool isBackFaceInViewSpace(const Triangle& triangle) {
	const vec3 p0 = xyz(triangle.v0.position);
	const vec3 p1 = xyz(triangle.v1.position);
	const vec3 p2 = xyz(triangle.v2.position);
	const vec3 normal = cross(p1 - p0, p2 - p0);
	const vec3 direction_to_camera = -p0;
	return dot(normal, direction_to_camera) <= 0.0f;
}

vec3 triangleCenter(const Triangle& triangle) {
	return (xyz(triangle.v0.position) + xyz(triangle.v1.position) + xyz(triangle.v2.position)) /
	       3.0f;
}

vec3 triangleNormal(const Triangle& triangle) {
	const vec3 p0 = xyz(triangle.v0.position);
	const vec3 p1 = xyz(triangle.v1.position);
	const vec3 p2 = xyz(triangle.v2.position);
	return normalize(cross(p1 - p0, p2 - p0));
}

vec3 ambientColor(const Material& material, const Lights& lights) {
	return material.albedo * lights.ambientColor() * lights.ambientIntensity() * material.ambient;
}

vec3 diffuseColor(const Material& material, const DirectionalLight& light, float light_power) {
	return material.albedo * light.color * light.intensity * material.diffuse * light_power;
}

vec3 specularColor(const Material& material, const DirectionalLight& light, const vec3& normal,
                   const vec3& direction_to_light, const vec3& direction_to_camera,
                   float light_power) {
	const vec3 reflected_light = normalize(reflect(-direction_to_light, normal));
	const float specular_power =
	    std::pow(std::max(dot(reflected_light, direction_to_camera), 0.0f), material.shininess);
	return light.color * light.intensity * material.specular * specular_power * light_power;
}

vec3 shadeColor(const Triangle& triangle, const Camera& camera, const Lights& lights) {
	const Material& material = triangle.material;
	const vec3 normal = triangleNormal(triangle);
	const vec3 center = triangleCenter(triangle);
	const vec3 direction_to_camera = normalize(camera.position() - center);
	vec3 color = ambientColor(material, lights);

	for (const DirectionalLight& light : lights.directionalLights()) {
		const vec3 direction_to_light = light.direction_to_light;
		const float light_power = std::max(dot(normal, direction_to_light), 0.0f);
		color += diffuseColor(material, light, light_power);
		color += specularColor(material, light, normal, direction_to_light, direction_to_camera,
		                       light_power);
	}

	return color;
}

bool isInsideClipVolume(const vec4& position) {
	return position.w > 0.0f && position.x >= -position.w - kClipSpaceEpsilon &&
	       position.x <= position.w + kClipSpaceEpsilon &&
	       position.y >= -position.w - kClipSpaceEpsilon &&
	       position.y <= position.w + kClipSpaceEpsilon &&
	       position.z >= -position.w - kClipSpaceEpsilon &&
	       position.z <= position.w + kClipSpaceEpsilon;
}

Vertex projectVertexToScreen(const Vertex& vertex, int width, int height) {
	assert(isInsideClipVolume(vertex.position));

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

Picture Renderer::render(const World& world, const Camera& camera, const Lights& lights) const {
	Picture picture(Width{width_}, Height{height_});
	picture.clear(kBlack);

	const mat4 view = camera.viewMatrix();
	const mat4 view_projection = camera.viewProjectionMatrix();
	for (const Triangle& triangle : world.triangles()) {
		const Triangle view_triangle = transformTriangle(view, triangle);
		if (isBackFaceInViewSpace(view_triangle)) {
			continue;
		}

		const Triangle lit_triangle = colorTriangle(triangle, shadeColor(triangle, camera, lights));
		const Triangle clip_triangle = transformTriangle(view_projection, lit_triangle);
		for (const Triangle& clipped_triangle : clipTriangleByFrustum(clip_triangle)) {
			const Triangle screen_triangle =
			    projectTriangleToScreen(clipped_triangle, width_, height_);
			Rasterizer::rasterizeTriangle(screen_triangle, &picture);
		}
	}
	return picture;
}

} // namespace renderer
