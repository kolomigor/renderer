#pragma once

#include "linalg.h"

#include <filesystem>
#include <vector>

namespace renderer {

class Texture {
public:
	Texture(int width, int height, std::vector<vec3> pixels);

	int width() const;
	int height() const;
	vec3 sample(const vec2& texcoord) const;

private:
	int pixelIndex(int x, int y) const;

	int width_;
	int height_;
	std::vector<vec3> pixels_;
};

Texture loadPPMTexture(const std::filesystem::path& filename);

} // namespace renderer
