#pragma once
#include "glm.h"

class Camera {
public:
	Camera(float fov, float aspect, float near, float far);
	glm::mat4 View() const;
	glm::mat4 Projection() const;
	void SetPosition(const glm::vec3& pos);
	void LookAt(const glm::vec3& target);

private:
	glm::vec3 pos_;
	glm::vec3 forward_;
	glm::vec3 up_;
	glm::vec3 right_;
	float fov_;
	float aspect_;
	float near_;
	float far_;
};