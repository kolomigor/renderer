#include "camera.h"

static constexpr glm::vec3 WORLD_UP = {0.f, 1.f, 0.f};

Camera::Camera(float fov, float aspect, float near, float far)
    : fov_(fov), aspect_(aspect), near_(near), far_(far) {
	pos_ = {0, 0, 5};
	forward_ = {0, 0, -1};
	up_ = WORLD_UP;
	right_ = glm::normalize(glm::cross(forward_, up_));
}

glm::mat4 Camera::View() const {
	return glm::lookAt(pos_, pos_ + forward_, up_);
}

glm::mat4 Camera::Projection() const {
	return glm::perspective(glm::radians(fov_), aspect_, near_, far_);
}

void Camera::SetPosition(const glm::vec3& pos) {
	pos_ = pos;
}

void Camera::LookAt(const glm::vec3& target) {
	forward_ = glm::normalize(target - pos_);
	right_ = glm::normalize(glm::cross(forward_, WORLD_UP));
	up_ = glm::normalize(glm::cross(right_, forward_));
}
