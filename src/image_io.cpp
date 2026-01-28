#include "image_io.h"
#include "picture.h"

#include <algorithm>
#include <fstream>

bool SavePPM(const Picture& picture, const std::string& filename) {
	std::ofstream out(filename, std::ios::binary);
	if (!out) {
		return false;
	}
	const int width = picture.Width();
	const int height = picture.Height();
	out << "P6\n";
	out << width << " " << height << "\n";
	out << "255\n";
	const auto& buffer = picture.ColorBuffer();
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			const glm::vec3& c = buffer[y * width + x];
			unsigned char r = static_cast<unsigned char>(std::clamp(c.r, 0.0f, 1.0f) * 255.0f);
			unsigned char g = static_cast<unsigned char>(std::clamp(c.g, 0.0f, 1.0f) * 255.0f);
			unsigned char b = static_cast<unsigned char>(std::clamp(c.b, 0.0f, 1.0f) * 255.0f);
			out.write(reinterpret_cast<char *>(&r), 1);
			out.write(reinterpret_cast<char *>(&g), 1);
			out.write(reinterpret_cast<char *>(&b), 1);
		}
	}
	return true;
}
