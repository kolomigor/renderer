#pragma once

#include "linalg.h"
#include "strong_alias.h"

#include <vector>

namespace renderer {

class Picture {
public:
	Picture(Width width, Height height);

	void clear(const vec3& color);
	void setPixel(PixelX x, PixelY y, const vec3& color, float depth);

	int width() const;
	int height() const;
	const vec3& colorAt(PixelX x, PixelY y) const;

private:
	int pixelIndex(int x, int y) const;

	int width_;
	std::vector<vec3> color_;
	std::vector<float> depth_;
};

} // namespace renderer
