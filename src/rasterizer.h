#pragma once
#include "picture.h"
#include <glm/glm.hpp>

struct ScreenVertex {
	float x;
	float y;
	float depth;
	glm::vec3 color;
};

class Rasterizer {
public:
	static void RasterizeTriangle(const ScreenVertex& a, const ScreenVertex& b,
	                              const ScreenVertex& c, Picture& picture);
};
