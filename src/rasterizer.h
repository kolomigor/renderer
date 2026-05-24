#pragma once

#include "camera.h"
#include "lights.h"
#include "picture.h"
#include "primitives.h"

namespace renderer {

class Rasterizer {
public:
	static void rasterizeTriangle(const Triangle& triangle, Picture *picture);
	static void rasterizeTriangle(const Triangle& triangle, const Camera& camera,
	                              const Lights& lights, Picture *picture);
};

} // namespace renderer
