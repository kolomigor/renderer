#include "image_io.h"

#include <algorithm>
#include <cstdint>
#include <fstream>

namespace renderer {
namespace {

std::uint8_t toColorChannel(float color) {
	return static_cast<std::uint8_t>(std::clamp(color, 0.0f, 1.0f) * 255.0f);
}

void writeColorChannel(std::ostream& out, float color) {
	out.put(static_cast<char>(toColorChannel(color)));
}

} // namespace

bool savePPM(const Picture& picture, const std::filesystem::path& filename) {
	std::ofstream out(filename, std::ios::binary);
	if (!out) {
		return false;
	}
	const int width = picture.width();
	const int height = picture.height();
	out << "P6\n";
	out << width << " " << height << "\n";
	out << "255\n";
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			const vec3& color = picture.colorAt(PixelX{x}, PixelY{y});
			writeColorChannel(out, color.r);
			writeColorChannel(out, color.g);
			writeColorChannel(out, color.b);
		}
	}
	return true;
}

} // namespace renderer
