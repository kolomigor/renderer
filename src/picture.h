#pragma once
#include "glm.h"
#include <vector>

class Picture {
public:
	Picture(int width, int height);
	void Clear(const glm::vec3& color);
	void ClearDepth();
	void SetPixel(int x, int y, const glm::vec3& color, float depth);
	int Width() const;
	int Height() const;
	const std::vector<glm::vec3>& ColorBuffer() const;
	const std::vector<float>& DepthBuffer() const;

private:
	int width_;
	int height_;
	std::vector<glm::vec3> color_;
	std::vector<float> depth_;
};