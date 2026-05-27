#include "picture.h"

#include <algorithm>
#include <cassert>
#include <limits>

namespace renderer {
namespace {

int positiveWidth(Width width) {
	assert(width > 0);
	return width;
}

int pixelCount(Width width, Height height) {
	assert(width > 0);
	assert(height > 0);
	return width * height;
}

} // namespace

Picture::Picture(Width width, Height height)
    : width_(positiveWidth(width)), color_(pixelCount(width, height)),
      depth_(pixelCount(width, height)) {
	clear(kBlack);
}

void Picture::clear(const vec3& color) {
	std::fill(color_.begin(), color_.end(), color);
	std::fill(depth_.begin(), depth_.end(), std::numeric_limits<float>::infinity());
}

void Picture::setPixel(PixelX x, PixelY y, const vec3& color, float depth) {
	const int x_value = x;
	const int y_value = y;
	assert(x_value >= 0);
	assert(y_value >= 0);
	assert(x_value < width());
	assert(y_value < height());

	const int index = pixelIndex(x_value, y_value);
	if (depth < depth_[index]) {
		depth_[index] = depth;
		color_[index] = color;
	}
}

int Picture::width() const {
	return width_;
}

int Picture::height() const {
	return static_cast<int>(color_.size()) / width_;
}

const vec3& Picture::colorAt(PixelX x, PixelY y) const {
	const int x_value = x;
	const int y_value = y;
	assert(x_value >= 0);
	assert(y_value >= 0);
	assert(x_value < width());
	assert(y_value < height());
	return color_[pixelIndex(x_value, y_value)];
}

int Picture::pixelIndex(int x, int y) const {
	return y * width_ + x;
}

} // namespace renderer
