#include "rasterizer.h"
#include <algorithm>
#include <cmath>

static float Edge(const glm::vec2& a, const glm::vec2& b, const glm::vec2& p) {
	return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

void Rasterizer::RasterizeTriangle(const ScreenVertex& a, const ScreenVertex& b,
                                   const ScreenVertex& c, Picture& picture) {
	const int min_x = static_cast<int>(std::floor(std::min({a.x, b.x, c.x})));
	const int max_x = static_cast<int>(std::ceil(std::max({a.x, b.x, c.x})));
	const int min_y = static_cast<int>(std::floor(std::min({a.y, b.y, c.y})));
	const int max_y = static_cast<int>(std::ceil(std::max({a.y, b.y, c.y})));
	const glm::vec2 p0{a.x, a.y};
	const glm::vec2 p1{b.x, b.y};
	const glm::vec2 p2{c.x, c.y};
	const float area = Edge(p0, p1, p2);
	if (area == 0.0f) {
		return;
	}
	for (int y = min_y; y <= max_y; ++y) {
		for (int x = min_x; x <= max_x; ++x) {
			const glm::vec2 p{static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f};
			const float w0 = Edge(p1, p2, p);
			const float w1 = Edge(p2, p0, p);
			const float w2 = Edge(p0, p1, p);
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				const float alpha = w0 / area;
				const float beta = w1 / area;
				const float gamma = w2 / area;
				const float depth = alpha * a.depth + beta * b.depth + gamma * c.depth;
				glm::vec3 color = alpha * a.color + beta * b.color + gamma * c.color;
				picture.SetPixel(x, y, color, depth);
			}
		}
	}
}
