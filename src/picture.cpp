#include "picture.h"
#include <algorithm>
#include <limits>

Picture::Picture(int w, int h) : width_(w), height_(h), color_(w * h), depth_(w * h) {
	Clear({0, 0, 0});
	ClearDepth();
}

void Picture::Clear(const glm::vec3& color) {
	std::fill(color_.begin(), color_.end(), color);
}

void Picture::ClearDepth() {
	std::fill(depth_.begin(), depth_.end(), std::numeric_limits<float>::infinity());
}

void Picture::SetPixel(int x, int y, const glm::vec3& color, float depth) {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) {
		return;
	}
	int index = y * width_ + x;
	if (depth < depth_[index]) {
		depth_[index] = depth;
		color_[index] = color;
	}
}

int Picture::Width() const {
	return width_;
}

int Picture::Height() const {
	return height_;
}

const std::vector<glm::vec3>& Picture::ColorBuffer() const {
	return color_;
}

const std::vector<float>& Picture::DepthBuffer() const {
	return depth_;
}