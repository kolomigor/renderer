#include "rasterizer.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace renderer {
namespace {

struct PixelBounds {
	int min_x;
	int max_x;
	int min_y;
	int max_y;
};

float computeEdgeFunction(const vec2& a, const vec2& b, const vec2& p) {
	return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

int floorToPixel(float value) {
	return static_cast<int>(std::floor(value));
}

int ceilToPixel(float value) {
	return static_cast<int>(std::ceil(value));
}

float pixelCenter(int value) {
	return static_cast<float>(value) + 0.5f;
}

PixelBounds makePixelBounds(const Triangle& triangle, const Picture& picture) {
	const vec2 p0 = xy(triangle.v0.position);
	const vec2 p1 = xy(triangle.v1.position);
	const vec2 p2 = xy(triangle.v2.position);
	const int min_x = floorToPixel(std::min({p0.x, p1.x, p2.x}));
	const int max_x = ceilToPixel(std::max({p0.x, p1.x, p2.x}));
	const int min_y = floorToPixel(std::min({p0.y, p1.y, p2.y}));
	const int max_y = ceilToPixel(std::max({p0.y, p1.y, p2.y}));

	return {std::clamp(min_x, 0, picture.width() - 1), std::clamp(max_x, 0, picture.width() - 1),
	        std::clamp(min_y, 0, picture.height() - 1), std::clamp(max_y, 0, picture.height() - 1)};
}

bool hasInsideSign(float edge, float area) {
	return area > 0.0f ? edge >= 0.0f : edge <= 0.0f;
}

vec3 interpolateColor(const Triangle& triangle, float alpha, float beta, float gamma) {
	const float reciprocal_w = alpha * triangle.v0.position.w + beta * triangle.v1.position.w +
	                           gamma * triangle.v2.position.w;
	const vec3 color_over_w = alpha * triangle.v0.color * triangle.v0.position.w +
	                          beta * triangle.v1.color * triangle.v1.position.w +
	                          gamma * triangle.v2.color * triangle.v2.position.w;
	return color_over_w / reciprocal_w;
}

vec2 interpolateTexcoord(const Triangle& triangle, float alpha, float beta, float gamma) {
	const float reciprocal_w = alpha * triangle.v0.position.w + beta * triangle.v1.position.w +
	                           gamma * triangle.v2.position.w;
	const vec2 texcoord_over_w = alpha * triangle.v0.texcoord * triangle.v0.position.w +
	                             beta * triangle.v1.texcoord * triangle.v1.position.w +
	                             gamma * triangle.v2.texcoord * triangle.v2.position.w;
	return texcoord_over_w / reciprocal_w;
}

vec3 interpolateWorldPosition(const Triangle& triangle, float alpha, float beta, float gamma) {
	const float reciprocal_w = alpha * triangle.v0.position.w + beta * triangle.v1.position.w +
	                           gamma * triangle.v2.position.w;
	const vec3 position_over_w = alpha * triangle.v0.world_position * triangle.v0.position.w +
	                             beta * triangle.v1.world_position * triangle.v1.position.w +
	                             gamma * triangle.v2.world_position * triangle.v2.position.w;
	return position_over_w / reciprocal_w;
}

vec3 interpolateNormal(const Triangle& triangle, float alpha, float beta, float gamma) {
	const float reciprocal_w = alpha * triangle.v0.position.w + beta * triangle.v1.position.w +
	                           gamma * triangle.v2.position.w;
	const vec3 normal_over_w = alpha * triangle.v0.normal * triangle.v0.position.w +
	                           beta * triangle.v1.normal * triangle.v1.position.w +
	                           gamma * triangle.v2.normal * triangle.v2.position.w;
	const vec3 normal = normal_over_w / reciprocal_w;
	if (length(normal) == 0.0f) {
		return {0.0f, 0.0f, 1.0f};
	}
	return normalize(normal);
}

float interpolateDepth(const Triangle& triangle, float alpha, float beta, float gamma) {
	return alpha * triangle.v0.position.z + beta * triangle.v1.position.z +
	       gamma * triangle.v2.position.z;
}

vec3 materialBaseColor(const Triangle& triangle, float alpha, float beta, float gamma) {
	const vec3 color = interpolateColor(triangle, alpha, beta, gamma);
	if (!triangle.material.texture) {
		return color;
	}
	return color * triangle.material.texture->sample(interpolateTexcoord(triangle, alpha, beta, gamma));
}

float pointLightAttenuation(const PointLight& light, float distance_to_light) {
	return 1.0f / (1.0f + light.linear_attenuation * distance_to_light +
	               light.quadratic_attenuation * distance_to_light * distance_to_light);
}

vec3 specularColor(const Material& material, const vec3& light_color, float light_intensity,
                   float attenuation, const vec3& normal, const vec3& direction_to_light,
                   const vec3& direction_to_camera, float light_power) {
	const vec3 reflected_light = normalize(reflect(-direction_to_light, normal));
	const float specular_power =
	    std::pow(std::max(dot(reflected_light, direction_to_camera), 0.0f), material.shininess);
	return light_color * light_intensity * attenuation * material.specular * specular_power *
	       light_power;
}

vec3 litPixelColor(const Triangle& triangle, const Camera& camera, const Lights& lights,
                   float alpha, float beta, float gamma) {
	const Material& material = triangle.material;
	const vec3 base_color = materialBaseColor(triangle, alpha, beta, gamma);
	const vec3 world_position = interpolateWorldPosition(triangle, alpha, beta, gamma);
	const vec3 normal = interpolateNormal(triangle, alpha, beta, gamma);
	const vec3 direction_to_camera = normalize(camera.position() - world_position);
	vec3 color =
	    material.emission + base_color * lights.ambientColor() * lights.ambientIntensity() *
	                            material.ambient;

	for (const DirectionalLight& light : lights.directionalLights()) {
		const vec3 direction_to_light = light.direction_to_light;
		const float light_power = std::max(dot(normal, direction_to_light), 0.0f);
		color += base_color * light.color * light.intensity * material.diffuse * light_power;
		color += specularColor(material, light.color, light.intensity, 1.0f, normal,
		                       direction_to_light, direction_to_camera, light_power);
	}

	for (const PointLight& light : lights.pointLights()) {
		const vec3 to_light = light.position - world_position;
		const float distance_to_light = length(to_light);
		if (distance_to_light == 0.0f) {
			continue;
		}
		const vec3 direction_to_light = to_light / distance_to_light;
		const float attenuation = pointLightAttenuation(light, distance_to_light);
		const float light_power = std::max(dot(normal, direction_to_light), 0.0f);
		color += base_color * light.color * light.intensity * attenuation * material.diffuse *
		         light_power;
		color += specularColor(material, light.color, light.intensity, attenuation, normal,
		                       direction_to_light, direction_to_camera, light_power);
	}

	return color;
}

} // namespace

void rasterizeTriangleWithShader(const Triangle& triangle, Picture *picture,
                                 const Camera *camera = nullptr, const Lights *lights = nullptr) {
	assert(picture != nullptr);

	const vec2 p0 = xy(triangle.v0.position);
	const vec2 p1 = xy(triangle.v1.position);
	const vec2 p2 = xy(triangle.v2.position);
	const float area = computeEdgeFunction(p0, p1, p2);
	if (area == 0.0f) {
		return;
	}

	const PixelBounds bounds = makePixelBounds(triangle, *picture);
	for (int y = bounds.min_y; y <= bounds.max_y; ++y) {
		for (int x = bounds.min_x; x <= bounds.max_x; ++x) {
			const vec2 point{pixelCenter(x), pixelCenter(y)};
			const float edge0 = computeEdgeFunction(p1, p2, point);
			const float edge1 = computeEdgeFunction(p2, p0, point);
			const float edge2 = computeEdgeFunction(p0, p1, point);
			if (hasInsideSign(edge0, area) && hasInsideSign(edge1, area) &&
			    hasInsideSign(edge2, area)) {
				const float alpha = edge0 / area;
				const float beta = edge1 / area;
				const float gamma = edge2 / area;
				const float depth = interpolateDepth(triangle, alpha, beta, gamma);
				const vec3 color =
				    (camera != nullptr && lights != nullptr)
				        ? litPixelColor(triangle, *camera, *lights, alpha, beta, gamma)
				        : materialBaseColor(triangle, alpha, beta, gamma);
				picture->setPixel(PixelX{x}, PixelY{y}, color, depth);
			}
		}
	}
}

void Rasterizer::rasterizeTriangle(const Triangle& triangle, Picture *picture) {
	rasterizeTriangleWithShader(triangle, picture);
}

void Rasterizer::rasterizeTriangle(const Triangle& triangle, const Camera& camera,
                                   const Lights& lights, Picture *picture) {
	rasterizeTriangleWithShader(triangle, picture, &camera, &lights);
}

} // namespace renderer
