#pragma once

#include "picture.h"
#include "primitives.h"

namespace renderer {

class Rasterizer {
public:
	static void rasterizeTriangle(const Triangle& triangle, Picture *picture);
};

} // namespace renderer
