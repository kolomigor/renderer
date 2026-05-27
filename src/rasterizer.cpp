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

} // namespace

void rasterizeTriangleWithShader(const Triangle& triangle, Picture *picture) {
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
				const vec3 color = materialBaseColor(triangle, alpha, beta, gamma);
				picture->setPixel(PixelX{x}, PixelY{y}, color, depth);
			}
		}
	}
}

void Rasterizer::rasterizeTriangle(const Triangle& triangle, Picture *picture) {
	rasterizeTriangleWithShader(triangle, picture);
}

} // namespace renderer
