#include "texture.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace renderer {
namespace {

int positiveDimension(int value, const std::string& name) {
	if (value <= 0) {
		throw std::runtime_error("texture " + name + " must be positive");
	}
	return value;
}

std::string readToken(std::istream& input) {
	char ch = 0;
	while (input.get(ch)) {
		if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
			continue;
		}
		if (ch == '#') {
			input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			continue;
		}

		std::string token(1, ch);
		while (input.get(ch)) {
			if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
				break;
			}
			if (ch == '#') {
				input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				break;
			}
			token.push_back(ch);
		}
		return token;
	}
	throw std::runtime_error("unexpected end of PPM file");
}

int readIntToken(std::istream& input, const std::string& name) {
	try {
		return std::stoi(readToken(input));
	} catch (const std::exception&) {
		throw std::runtime_error("invalid PPM " + name);
	}
}

float normalizeColorChannel(int value, int max_value) {
	if (value < 0 || value > max_value) {
		throw std::runtime_error("PPM color channel is out of range");
	}
	return static_cast<float>(value) / static_cast<float>(max_value);
}

vec3 readP3Pixel(std::istream& input, int max_value) {
	return {normalizeColorChannel(readIntToken(input, "red channel"), max_value),
	        normalizeColorChannel(readIntToken(input, "green channel"), max_value),
	        normalizeColorChannel(readIntToken(input, "blue channel"), max_value)};
}

vec3 readP6Pixel(std::istream& input, int max_value) {
	unsigned char channels[3]{};
	if (!input.read(reinterpret_cast<char *>(channels), 3)) {
		throw std::runtime_error("unexpected end of binary PPM pixel data");
	}
	return {normalizeColorChannel(channels[0], max_value),
	        normalizeColorChannel(channels[1], max_value),
	        normalizeColorChannel(channels[2], max_value)};
}

float clamp01(float value) {
	return std::clamp(value, 0.0f, 1.0f);
}

float lerp(float a, float b, float t) {
	return a + (b - a) * t;
}

vec3 lerp(const vec3& a, const vec3& b, float t) {
	return {lerp(a.r, b.r, t), lerp(a.g, b.g, t), lerp(a.b, b.b, t)};
}

} // namespace

Texture::Texture(int width, int height, std::vector<vec3> pixels)
    : width_(positiveDimension(width, "width")),
      height_(positiveDimension(height, "height")),
      pixels_(std::move(pixels)) {
	if (static_cast<int>(pixels_.size()) != width_ * height_) {
		throw std::runtime_error("texture pixel count does not match dimensions");
	}
}

int Texture::width() const {
	return width_;
}

int Texture::height() const {
	return height_;
}

vec3 Texture::sample(const vec2& texcoord) const {
	assert(!pixels_.empty());

	const float u = clamp01(texcoord.x);
	const float v = clamp01(texcoord.y);
	const float x = u * static_cast<float>(width_ - 1);
	const float y = (1.0f - v) * static_cast<float>(height_ - 1);
	const int x0 = std::clamp(static_cast<int>(std::floor(x)), 0, width_ - 1);
	const int y0 = std::clamp(static_cast<int>(std::floor(y)), 0, height_ - 1);
	const int x1 = std::min(x0 + 1, width_ - 1);
	const int y1 = std::min(y0 + 1, height_ - 1);
	const float tx = x - static_cast<float>(x0);
	const float ty = y - static_cast<float>(y0);

	const vec3 top = lerp(pixels_[pixelIndex(x0, y0)], pixels_[pixelIndex(x1, y0)], tx);
	const vec3 bottom = lerp(pixels_[pixelIndex(x0, y1)], pixels_[pixelIndex(x1, y1)], tx);
	return lerp(top, bottom, ty);
}

int Texture::pixelIndex(int x, int y) const {
	return y * width_ + x;
}

Texture loadPPMTexture(const std::filesystem::path& filename) {
	std::ifstream input(filename, std::ios::binary);
	if (!input) {
		throw std::runtime_error("failed to open texture: " + filename.string());
	}

	const std::string magic = readToken(input);
	if (magic != "P3" && magic != "P6") {
		throw std::runtime_error("unsupported texture format in " + filename.string() +
		                         ": expected P3 or P6 PPM");
	}
	const int width = positiveDimension(readIntToken(input, "width"), "width");
	const int height = positiveDimension(readIntToken(input, "height"), "height");
	const int max_value = readIntToken(input, "max value");
	if (max_value <= 0 || max_value > 255) {
		throw std::runtime_error("unsupported PPM max value in " + filename.string());
	}

	std::vector<vec3> pixels;
	pixels.reserve(width * height);
	for (int index = 0; index < width * height; ++index) {
		pixels.push_back(magic == "P3" ? readP3Pixel(input, max_value)
		                               : readP6Pixel(input, max_value));
	}
	return Texture(width, height, std::move(pixels));
}

} // namespace renderer
